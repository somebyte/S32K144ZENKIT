/*
 * adc.c
 *
 * Created on: 2020.04.30
 *     Author: somebyte
 *
 */

#include "adc.h"

#define NULL (ADC_Type*)0

ADC_Type *ADCx = NULL;

int
adc_init (ADC_Type *ptr)
{
  ADCx = ptr;
  uint8_t INDEX = 0;

  if (ptr == ADC0)
    INDEX = PCC_ADC0_INDEX;
  else
  if (ptr == ADC1)
    INDEX = PCC_ADC1_INDEX;

  if (!INDEX)
    {
      return -1;
    }

  PCC->PCCn[INDEX] &= ~PCC_PCCn_CGC_MASK; /* Disable clock to change PCS */
  PCC->PCCn[INDEX] |=  PCC_PCCn_PCS(2);   /* PCS=2: Select SIRCDIV2      */
  PCC->PCCn[INDEX] |=  PCC_PCCn_CGC_MASK; /* Enable bus clock in ADC     */

  /* See p. 1222 of RM S32Kxx */
  ADCx->SC1[0] = ADC_SC1_ADCH_MASK;  /* Module is disabled */
  ADCx->CFG1   = ADC_CFG1_ADICLK(0)| /* ADICLK=0: Input clk=ALTCLK1=SOSCDIV2         */
                 ADC_CFG1_MODE(1)  | /* MODE=1:   12-bit conversion                  */
                 ADC_CFG1_ADIV(0);   /* ADIV=0:   Prescaler=1                        */

  ADCx->CFG2 = ADC_CFG2_SMPLTS(0xD);    /* SMPLTS=12(default): sample time is 13 ADC clks */

  ADCx->SC2 = 0x00000000;
  ADCx->SC3 = 0x00000000;

  return 0;
}

void
adc_disable()
{
  if (ADCx == ADC0)
    PCC->PCCn[PCC_ADC0_INDEX]&= ~PCC_PCCn_CGC_MASK;
  else
  if (ADCx == ADC1)
    PCC->PCCn[PCC_ADC1_INDEX]&= ~PCC_PCCn_CGC_MASK;

  ADCx = NULL;
}

int
adc_st_conversion_chx (uint16_t CHx)
{
  if (!ADCx)
    return -1;
  ADCx->SC1[0] &= ~ADC_SC1_ADCH_MASK; /* Clear prior ADCH bits */
  ADCx->SC1[0]  =  ADC_SC1_ADCH(CHx); /* Initiate Conversion   */
  return 0;
}

uint32_t
adc_read_result_mV (void)
{
  uint32_t R;
  R = (uint32_t) ADCx?ADCx->R[0]:0;
  return (uint32_t)((5000*R)/0x0FFF);
}

uint8_t
adc_ck_completion (void)
{
  return ADCx?((ADCx->SC1[0] & ADC_SC1_COCO_MASK)>>ADC_SC1_COCO_SHIFT):0;
}

void
adc_wt_completion (void)
{
  while(adc_ck_completion () == 0);
}

uint32_t adc_wt_conversion_chx (uint16_t CHx)
{
  if (adc_st_conversion_chx (CHx) < 0)
    return 0;
  adc_wt_completion ();
  return adc_read_result_mV();
}

