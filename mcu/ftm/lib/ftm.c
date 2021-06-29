/*
 * ftm.c
 *
 *  Created on: Jun 22, 2021
 *      Author: somebyte
 */

#include <stddef.h>
#include "../ftm.h"
#include "../../sys/nvic.h"

struct ftmdev;
typedef struct ftmdev ftmdev_t;
typedef ftmdev_t * ftmdev_ptr_t;

struct ftmdev
{
  FTM_Type*  FTM;
  uint32_t   CHn;
  PORT_Type* PORT;
  GPIO_Type* GPIO;
  uint32_t   GPIOn;
  uint16_t currentCapture;
  uint16_t priorCapture;
  uint16_t deltaCapture;
  uint16_t flagOverflow;
};

static ftmdev_t ftms[FTM_NS] = { {.FTM = NULL, .deltaCapture = 0},
                                 {.FTM = NULL, .deltaCapture = 0},
				 {.FTM = NULL, .deltaCapture = 0},
				 {.FTM = NULL, .deltaCapture = 0} };

uint32_t pcc_ftm_index_of (ftmnum_t FTM_N);
void     ftm_chX_chY_irqhandler    (ftmnum_t  FTMn);
void     ftm_ovf_reload_irqhandler (ftmnum_t  FTMn);

#define CHOOSE_INTERRUPT(FTMN, CHN) \
        switch (CHN)\
        {\
        case 0:\
        case 1:\
          NVIC_IRQn_init (FTM##FTMN##_Ch0_Ch1_IRQn, 0x0B);\
        break;\
        case 2:\
        case 3:\
          NVIC_IRQn_init (FTM##FTMN##_Ch2_Ch3_IRQn, 0x0B);\
        break;\
        case 4:\
        case 5:\
          NVIC_IRQn_init (FTM##FTMN##_Ch4_Ch5_IRQn, 0x0B);\
        break;\
        case 6:\
        case 7:\
          NVIC_IRQn_init (FTM##FTMN##_Ch6_Ch7_IRQn, 0x0B);\
        break;\
        default: return -1;\
        }

int
ftm_init (ftmnum_t     FTMn,
          uint32_t     CHn,
	  portnum_t    PORTn,
	  uint32_t     GPIOn,
	  uint32_t     ALT)
{
  ftms[FTMn].FTM = NULL;

  switch (FTMn)
    {
    case FTM_N0:
      ftms[FTMn].FTM = FTM0;
      NVIC_IRQn_init (FTM0_Ovf_Reload_IRQn, 0x0A);
      break;
    case FTM_N1:
      ftms[FTMn].FTM = FTM1;
      NVIC_IRQn_init (FTM1_Ovf_Reload_IRQn, 0x0A);
      break;
    case FTM_N2:
      ftms[FTMn].FTM = FTM2;
      NVIC_IRQn_init (FTM2_Ovf_Reload_IRQn, 0x0A);
      break;
    case FTM_N3:
      ftms[FTMn].FTM = FTM3;
      NVIC_IRQn_init (FTM3_Ovf_Reload_IRQn, 0x0A);
      break;
    default: return -1;
    }

  ftms[FTMn].CHn   = (CHn <= 7) ? CHn : 0;
  ftms[FTMn].PORT  = port_type_of (PORTn);
  ftms[FTMn].GPIO  = gpio_type_of (PORTn);
  ftms[FTMn].GPIOn = (GPIOn <= 31) ? GPIOn : 0;

  PCC->PCCn [pcc_port_index_of (PORTn)]  |= PCC_PCCn_CGC_MASK;
  ftms[FTMn].PORT->PCR[ftms[FTMn].GPIOn] |= PORT_PCR_MUX (ALT);

  PCC->PCCn[ pcc_ftm_index_of (FTMn) ] &= ~PCC_PCCn_CGC_MASK; /* Ensure clk disabled for config  */
  PCC->PCCn[ pcc_ftm_index_of (FTMn) ] |= PCC_PCCn_PCS(0b010) /* Clock Src=1, 8 MHz SIRCDIV1_CLK */
                                       |  PCC_PCCn_CGC_MASK;  /* Enable clock for FTM regs       */

  ftms[FTMn].FTM->MODE |= FTM_MODE_WPDIS_MASK; /* Write protect to registers disabled (default) */
  ftms[FTMn].FTM->SC    = 0x00000107;          /* Enable PWM channel 0 output*/
                                     /* Enable PWM channel 1 output*/
                                     /* TOIE (Timer Overflow Interrupt Enable) = 0 (default)      */
                                     /* CPWMS (Center aligned PWM Select) = 0 (default, up count) */
                                     /* CLKS (Clock source) = 0 (default, no clock; FTM disabled) */
                                     /* PS (Prescaler factor) = 7. Prescaler = 128                */
  ftms[FTMn].FTM->COMBINE = 0x00000000; /* FTM mode settings used: DECAPENx, MCOMBINEx, COMBINEx=0   */
  ftms[FTMn].FTM->POL     = 0x00000000; /* Polarity for all channels is active high (default)        */
  ftms[FTMn].FTM->MOD     = 62500-1 ;   /* FTM1 counter final value (used for PWM mode)              */
                              /* FTM1 Period = MOD-CNTIN+0x0001 ~= 62500 ctr clks          */
                              /* 8MHz/128 = 62.5kHz -> ticks -> 1Hz                        */

  ftms[FTMn].currentCapture = 0;
  ftms[FTMn].deltaCapture   = 0;
  ftms[FTMn].flagOverflow   = 0;
  ftms[FTMn].priorCapture   = 0;

  if (FTMn == FTM_N0)
    {
      CHOOSE_INTERRUPT(0, ftms[FTMn].CHn)
    }
  else
  if (FTMn == FTM_N1)
    {
      CHOOSE_INTERRUPT(1, ftms[FTMn].CHn)
    }
  else
  if (FTMn == FTM_N2)
    {
      CHOOSE_INTERRUPT(2, ftms[FTMn].CHn)
    }
  else
  if (FTMn == FTM_N3)
    {
      CHOOSE_INTERRUPT(3, ftms[FTMn].CHn)
    }
  else
    return -1;

  ftms[FTMn].FTM->CONTROLS[ftms[FTMn].CHn].CnSC = 0x00000048;

  return 0;
}

uint32_t
pcc_ftm_index_of (ftmnum_t FTMn)
{
  switch (FTMn)
    {
    case FTM_N0: return PCC_FTM0_INDEX;
    case FTM_N1: return PCC_FTM1_INDEX;
    case FTM_N2: return PCC_FTM2_INDEX;
    case FTM_N3: return PCC_FTM3_INDEX;
    default: return 0;
    }
}

void
ftm_start (ftmnum_t FTMn)
{
  if (ftms[FTMn].FTM)
    ftms[FTMn].FTM->SC |= FTM_SC_CLKS(3);
}

uint16_t
ftm_ticks (ftmnum_t FTMn)
{
  return ftms[FTMn].deltaCapture;
}

void
ftm_chX_chY_irqhandler (ftmnum_t FTMn)
{
  if (! ftms[FTMn].FTM)
    return;

  /* If channel flag is set */
  if (1 == ((ftms[FTMn].FTM->CONTROLS[ftms[FTMn].CHn].CnSC & FTM_CnSC_CHF_MASK) >> FTM_CnSC_CHF_SHIFT))
    {
      ftms[FTMn].FTM->CONTROLS[ftms[FTMn].CHn].CnSC &= 0xFFFFFFBF;
      ftms[FTMn].currentCapture = ftms[FTMn].FTM->CONTROLS[ftms[FTMn].CHn].CnV;    /* Record value of current capture     */
      if( ftms[FTMn].flagOverflow )
        {
          uint32_t LocalcurrentCapture = ftms[FTMn].flagOverflow*62500 + ftms[FTMn].currentCapture;
          ftms[FTMn].deltaCapture = LocalcurrentCapture - ftms[FTMn].priorCapture;
          ftms[FTMn].flagOverflow = 0;
        }
      else
        {
          ftms[FTMn].deltaCapture   = ftms[FTMn].currentCapture - ftms[FTMn].priorCapture;
        }
      ftms[FTMn].priorCapture = ftms[FTMn].currentCapture;                 /* Record value of prior capture       */
      ftms[FTMn].FTM->CONTROLS[ftms[FTMn].CHn].CnSC &= ~FTM_CnSC_CHF_MASK; /* Clear flag: read reg then set CHF=0 */
      ftms[FTMn].FTM->CONTROLS[ftms[FTMn].CHn].CnSC |= 0x00000040;
    }
}

void
ftm_ovf_reload_irqhandler (ftmnum_t FTMn)
{
  ftms[FTMn].FTM->SC &= ~0x00000300;
  ftms[FTMn].flagOverflow++;
  if( ftms[FTMn].flagOverflow > 3 )
    {
      ftms[FTMn].currentCapture = 0;
      ftms[FTMn].priorCapture   = 0;
      ftms[FTMn].deltaCapture   = 0;
      ftms[FTMn].flagOverflow   = 0;
    }
  ftms[FTMn].FTM->SC |=  0x00000100;
}

void
FTM0_Ovf_Reload_IRQHandler (void)
{
  ftm_ovf_reload_irqhandler (FTM_N0);
}

void
FTM1_Ovf_Reload_IRQHandler (void)
{
  ftm_ovf_reload_irqhandler (FTM_N1);
}

void
FTM2_Ovf_Reload_IRQHandler (void)
{
  ftm_ovf_reload_irqhandler (FTM_N2);
}

void
FTM3_Ovf_Reload_IRQHandler (void)
{
  ftm_ovf_reload_irqhandler (FTM_N3);
}

void
FTM0_Ch0_Ch1_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N0);
}

void
FTM0_Ch2_Ch3_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N0);
}

void
FTM0_Ch4_Ch5_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N0);
}

void
FTM0_Ch6_Ch7_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N0);
}

void
FTM1_Ch0_Ch1_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N1);
}

void
FTM1_Ch2_Ch3_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N1);
}

void
FTM1_Ch4_Ch5_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N1);
}

void
FTM1_Ch6_Ch7_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N1);
}

void
FTM2_Ch0_Ch1_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N2);
}

void
FTM2_Ch2_Ch3_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N2);
}

void
FTM2_Ch4_Ch5_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N2);
}

void
FTM2_Ch6_Ch7_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N2);
}

void
FTM3_Ch0_Ch1_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N3);
}

void
FTM3_Ch2_Ch3_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N3);
}

void
FTM3_Ch4_Ch5_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N3);
}

void
FTM3_Ch6_Ch7_IRQHandler (void)
{
  ftm_chX_chY_irqhandler (FTM_N3);
}
