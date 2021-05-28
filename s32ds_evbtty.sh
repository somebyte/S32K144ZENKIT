#!/bin/bash

export LN="/bin/ln -s -T "

$LN ../../S32K144ZENKIT/mcu/sys sys
$LN ../../S32K144ZENKIT/mcu/adc adc
$LN ../../S32K144ZENKIT/mcu/tty tty
$LN ../../S32K144ZENKIT/mcu/evbtty evbtty

