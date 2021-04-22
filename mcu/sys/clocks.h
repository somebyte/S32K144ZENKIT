/*
 * clocks.h
 *
 *  Created on: Jul 30, 2020
 *      Author: somebyte
 */

#ifndef CLOCKS_CLOCKS_H_
#define CLOCKS_CLOCKS_H_

void disable_WDOG   (void);

// See p. 558 Figure 27-1 of S32Kxx Reference Manual for more information about clocking
void init_LPO        (void);
void init_SOSC       (void); // 8MHz
void init_SIRC       (void); // 8MHz
void init_FIRC       (void); // 48MHz
void init_SPLL       (void); // 160MHz
void enter_mode_NRUN (void); // 80MHz - CORE & SYS; 40MHz - BUS; 20MHz - FLASH

void disable_LPO  (void);
void disable_SOSC (void);
void disable_SIRC (void);
void disable_FIRC (void);
void disable_SPLL (void);

void init_CLKs        (void);
void disable_CLKs     (void);
void disable_CLKs_dbg (void);

#endif /* CLOCKS_CLOCKS_H_ */
