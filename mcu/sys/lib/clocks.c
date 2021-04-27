/*
 * clocks.c
 *
 *  Created on: Jul 30, 2020
 *      Author: somebyte
 */

#include <S32K144.h>
#include "../clocks.h"

void
disable_WDOG (void)
{
  WDOG->CNT   = 0xD928C520; /* Unlock watchdog       */
  WDOG->TOVAL = 0x0000FFFF; /* Maximum timeout value */
  WDOG->CS    = 0x00002100; /* Disable watchdog      */
}

void
init_LPO (void)
{
  SIM->LPOCLKS = SIM_LPOCLKS_LPO1KCLKEN(0) |  // [0]   LPO1KCLKEN=0:  Disable  1 kHz LPO_CLK output
                                              // [0]   LPO1KCLKEN=1:  Enable   1 kHz LPO_CLK output
                 SIM_LPOCLKS_LPO32KCLKEN(1)|  // [1]   LPO32KCLKEN=0: Disable 32 kHz LPO_CLK output
                                              // [1]   LPO32KCLKEN=1: Enable  32 kHz LPO_CLK output
                 SIM_LPOCLKS_LPOCLKSEL(2)  |  // [3-2] LPOCLKSEL=2:   32 kHz LPO_CLK which is derived from the 128 kHz LPO_CLK
                 SIM_LPOCLKS_RTCCLKSEL(1);    // [5-4] RTCCLKSEL=1:   32 kHz LPO_CLK
}

void
init_SOSC (void) // 8MHz
{
  // System OSC Divide Register
  SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(1) | // [2-0]  SOSCDIV1 =1: Divide by 1
                 SCG_SOSCDIV_SOSCDIV2(1);  // [10-8] SOSCDIV1 =1: Divide by 1
  // System Oscillator Configuration Register
  SCG->SOSCCFG = SCG_SOSCCFG_EREFS(1) |    // [2]   EREFS =1: Input is external XTAL
                 SCG_SOSCCFG_HGO(0)   |    // [3]   HGO   =0: Config xtal osc for low power
                 SCG_SOSCCFG_RANGE(2);     // [5-4] RANGE =2: Medium freq (SOSC between 1MHz-8MHz)
  // System OSC Control Status Register
  while(SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK); // Ensure SOSCCSR unlocked
                                             // LK=0: SOSCCSR can be written
  SCG->SOSCCSR = SCG_SOSCCSR_SOSCEN(1)  | // [0]  SOSCEN   =1: Enable oscillator
                 SCG_SOSCCSR_SOSCCM(0)  | // [16] SOSCCM   =0: OSC CLK monitor disabled
                 SCG_SOSCCSR_SOSCCMRE(0)| // [17] SOSCCMRE =0: Clock Monitor generates interrupt when error detected
                 SCG_SOSCCSR_SOSCSEL(0) | // [25] SOCSEL   =0: System OSC is not the system clock source
                 SCG_SOSCCSR_SOSCERR(0);  // [26] SOSCERR  =0: System OSC Clock Monitor is disabled or has not detected an error
  while(!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)); // Wait for valid sys OSC clk 
}

void
init_SIRC (void) //8MHz
{
  // Slow IRC Control Status Register
  while(SCG->SIRCCSR & SCG_SIRCCSR_LK_MASK); // Ensure SIRCCCSR unlocked
  SCG->SIRCCSR = SCG_SIRCCSR_SIRCEN(0)  | // [0]  SIRCEN   =0: SIRC is disabled
                 SCG_SIRCCSR_SIRCSTEN(0)| // [1]  SIRCSTEN =0: Slow IRC is disabled in supported Stop modes
                 SCG_SIRCCSR_SIRCLPEN(0)| // [2]  SIRCLPEN =0: Slow IRC is disabled in VLP modes
                 SCG_SIRCCSR_SIRCSEL(0);  // [25] SIRCSEL  =0: Slow IRC is not the system clock source

  // Slow IRC Configuration Register
  SCG->SIRCCFG = SCG_SIRCCFG_RANGE(1); // 0 - Slow IRC  low range clock (2 MHz)
                                       // 1 - Slow IRC high range clock (8 MHz)
  // Slow IRC Divide Register
  SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV1(1)| // [2-0]  SIRCDIV1 div. by 1
                 SCG_SIRCDIV_SIRCDIV2(1); // [10-8] SIRCDIV2 div. by 1

  SCG->SIRCCSR |= SCG_SIRCCSR_SIRCEN(1); // SIRC is enabled
  while(!(SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK)); // Wait for valid FIRC
}

void
init_FIRC (void) // 48MHz
{
  // Fast IRC Control Status Register
  while(SCG->FIRCCSR & SCG_FIRCCSR_LK_MASK); // Ensure FIRCCCSR unlocked
  SCG->FIRCCSR = SCG_FIRCCSR_FIRCEN(0)    |  // [0]  FIRCEN     =0: FIRC is disabled
                 SCG_FIRCCSR_FIRCREGOFF(0)|  // [3]  FIRCREGOFF =0: FIRC Regulator is enabled
                 SCG_FIRCCSR_FIRCSEL(0)   |  // [25] FIRCSEL    =0: Fast IRC is not the system clock source
                 SCG_FIRCCSR_FIRCERR(0);     // [26] FIRCERR    =0: Error not detected with the Fast IRC trimming

  // Fast IRC Configuration Register
  SCG->FIRCCFG = SCG_FIRCCFG_RANGE(0); // 0 is default: Fast IRC is trimmed to 48 MHz */

  // Fast IRC Divide Register
  SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV1(1)| // [2-0]  FIRCDIV1 div. by 1
                 SCG_FIRCDIV_FIRCDIV2(1); // [10-8] FIRCDIV2 div. by 1

  SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN(1);    // FIRC is enabled
  while(!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK)); // Wait for valid FIRC
}

void
init_SPLL (void) // 160 MHz
{
  // VCO_CLK = (SPLL_SOURCE/(PREDIV + 1)) X (MULT + 16) = (48/6)*(24+16) = 320 MHz
  // SPLL_CLK = (VCO_CLK)/2 = 320/2 = 160MHz

  unsigned int src = !((SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)>>SCG_SOSCCSR_SOSCVLD_SHIFT) &&
  ((SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK)>>SCG_FIRCCSR_FIRCVLD_SHIFT);

  while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK); // Ensure SPLLCSR unlocked
  SCG->SPLLCSR = SCG_SPLLCSR_SPLLEN(0)  | // SPLLEN  =0: SPLL is disabled (default)
                 SCG_SPLLCSR_SPLLCM(0)  | // SPLLCM  =0: SPLL Clock Monitor is disabled
                 SCG_SPLLCSR_SPLLCMRE(0)| // SPLLCMRE=0: Clock Monitor generates interrupt when error detected
                 SCG_SPLLCSR_SPLLSEL(0) | // SPLLSEL =0: SPLL is not the system clock source
                 SCG_SPLLCSR_SPLLERR(0);  // SPLLERR =0: SPLL Clock Monitor is disabled or has not detected an error

  SCG->SPLLDIV = SCG_SPLLDIV_SPLLDIV1(2)| // [2-0]  SPLLDIV1 div. by 2
                 SCG_SPLLDIV_SPLLDIV2(2); // [10-8] SPLLDIV2 div. by 2

  SCG->SPLLCFG = src| // SOURCE =0: System OSC (SOSC)
                      // SOURCE =1: Fast IRC (FIRC)
                 SCG_SPLLCFG_PREDIV(5) | // [10-8]  PREDIV =5:  Divide SOSC|FIRC_CLK by PREDIV+1 =6
                 SCG_SPLLCFG_MULT(0x18); // [20-16] MULT   =0x18: Multiply SPLL by 0x18 (24) + 0x10 (16) =
                                           // SPLL_CLK = 8MHz / 1 * 40 / 2 = 160 MHz
  while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK); // Ensure SPLLCSR unlocked
  SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN(1); // LK=0: SPLLCSR can be written
  while(!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK)); // Wait for SPLL valid
}

void
enter_mode_NRUN (void) // 80MHz /* Change to normal RUN mode with 8MHz FIRC, 80MHz PLL */
{
  SCG->RCCR = SCG_RCCR_DIVSLOW(2)| // [3-0]   DIVSLOW =2, div. by 3: FLASH_CLK = CORE_CLK or SYS_CLK / 3 = 26MHz
              SCG_RCCR_DIVBUS(1) | // [7-4]   DIVBUS  =1, div. by 2: BUS_CLK   = CORE_CLK or SYS_CLK / 2 = 40MHz
              SCG_RCCR_DIVCORE(1)| // [19-16] DIVCORE =1, div. by 2: CORE_CLK or SYS_CLK = SPLL/2 = 80MHz
              SCG_RCCR_SCS(6);     // [27-24] SCS     = 0b0110: System Clock Source is System PLL (SPLL_CLK)

  if (!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK))
    {
      init_SPLL();
    }
  // ClockStatusRegister_SystemClockSource
  while (((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT ) != 6) {} // Wait for sys clk src = SPLL
}

void
disable_LPO (void)
{
  SIM->LPOCLKS &= ~SIM_LPOCLKS_LPO32KCLKEN(1);
}

void
disable_SOSC (void)
{
  if (SCG->SOSCCSR & SCG_SOSCCSR_SOSCSEL(1))
    return;
  while(SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK);
  SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN(1);
  while(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK); // Wait for sys OSC clk to be disabled
}

void
disable_SIRC (void)
{
  if (SCG->SIRCCSR & SCG_SIRCCSR_SIRCSEL(1))
     return;
  while(SCG->SIRCCSR & SCG_SIRCCSR_LK_MASK);
  SCG->SIRCCSR &= ~SCG_SIRCCSR_SIRCEN(1);
  while(SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK); // Wait for SIRC to be disabled 
}

void
disable_FIRC (void)
{
  if (SCG->FIRCCSR & SCG_FIRCCSR_FIRCSEL(1))
    return;
  while (SCG->FIRCCSR & SCG_FIRCCSR_LK_MASK);
  SCG->FIRCCSR &= ~SCG_FIRCCSR_FIRCEN(1);
  while (SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK); // Wait for FIRC to be disabled
}

void
disable_SPLL (void)
{
  if (SCG->SPLLCSR & SCG_SPLLCSR_SPLLSEL(1))
      return;
  while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK);
  SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN(1);
  while(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK); // Wait for SPLL to be disabled
}

void
init_CLKs (void)
{
  disable_WDOG();
  init_LPO();
  init_SIRC();
  init_FIRC();
  init_SPLL();
  enter_mode_NRUN(); // Init clocks: 80 MHz sysclk & core,
                     //              40 MHz bus,
                     //              20 MHz flash
}

void
disable_CLKs (void)
{
  disable_SPLL();
  disable_FIRC();
  disable_SIRC();
  disable_SOSC();
  disable_LPO();
}

void
disable_CLKs_dbg (void)
{
  disable_SPLL();
//  disable_FIRC(); FIRC have to work during debug
  disable_SIRC();
  disable_SOSC();
  disable_LPO();
}

