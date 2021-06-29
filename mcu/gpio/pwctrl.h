/*
 * pwctrl.h
 *
 *  Created on: June 9, 2021
 *      Author: somebyte
 */

#ifndef GPIO_PWCTRL_H_
#define GPIO_PWCTRL_H_

#include "gpio.h"

typedef enum
{
  IGNITION_KEY_LINE   = 0,
  MPU_POWER_LINE      = 1,
  MPU_SHUTDOWN_LINE   = 2,
  DISP_POWER_LINE     = 3,
  DISP_BACKLIGHT_LINE = 4,
  RESERV_LINE_0       = 5,
  RESERV_LINE_1       = 6,
  RESERV_LINE_2       = 7,
  LINES_NUMBER        = 8
} powercontrol_t;

struct IO
{
  portnum_t port;
  int       pin;
  int       off;
};

typedef struct IO IO_t;
typedef IO_t*  IO_ptr_t;

extern IO_t power_pins[LINES_NUMBER]; // array for configuring MPU power management pins
                                      // See indexes in powercontrol_t
extern uint32_t nGPIOs[nSIZE]; // array of GPIOs status
                               // See indexes in portnum_t

#define DEFINE_POWER_KEY(_line,_port,_pin,_value) \
power_pins[_line].port = _port;\
power_pins[_line].pin  = _pin;\
power_pins[_line].off  = _value;

typedef enum
{
  IGNKEY_OFF = 0,
  IGNKEY_ON  = 1
} ignkeyevent_t;

void init_power_pins ();
void ignition_key_handle (ignkeyevent_t);

void IGNITION_KEY_PORT_IRQHandler (void);
#define PORT_IRQHandler(PORTNAME) void PORT##PORTNAME##_IRQHandler (void) { IGNITION_KEY_PORT_IRQHandler (); }

extern int fstop;  // was event IGNKEY_OFF
extern int fhello; // Set to 1 when Linux is booted

int enter_STOP (void); // return 1 if mcu slept & waked up
                       // it may be only if fstop && fhello is true

#endif

