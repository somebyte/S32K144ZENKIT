/*
 * light.h
 *
 *  Created on: May 12, 2021
 *      Author: somebyte
 */

#ifndef LIGHTS_H_
#define LIGHTS_H_

/*!
   * Pins definitions
   * ===================================================
   *
   * Pin number        | Function
   * ----------------- |------------------
   * PTD0              | GPIO [BLUE  LED]
   * PTD15             | GPIO [RED   LED]
   * PTD16             | GPIO [GREEN LED]
   *
   */

typedef enum
{
  LIGHTS_OFF  = 0,
  RED_LIGHT   = 1,
  GREEN_LIGHT = 2,
  BLUE_LIGHT  = 3
} lights_t;


void lights_init();
void lights_off ();
void lights_set (lights_t light, int state );


#endif /* LIGHTS_H_ */
