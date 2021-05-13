/*
 * adc.h
 *
 * Created on: 2020.04.30
 *     Author: somebyte
 *
 */

#ifndef __ADC_ADC_H__
#define __ADC_ADC_H__

#include <S32K144.h>

int      adc_init    (ADC_Type *ADC_PTR);
void     adc_disable ();
int      adc_st_conversion_chx (uint16_t CHx); /* Start conversion for CHx */
uint32_t adc_read_result_mV (void);            /* Result in mV             */
uint8_t  adc_ck_completion  (void);            /* Check of  completion     */
void     adc_wt_completion  (void);            /* Wait  for completion     */
uint32_t adc_wt_conversion_chx (uint16_t CHx); /* Start conversion for CHx */
                                               /* and wait for completion  */
                                               /* with return result       */
#endif

