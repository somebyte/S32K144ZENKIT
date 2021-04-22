#!/bin/bash

export LN="/bin/ln -s"

# base pkg
$LN ../../S32K144ZENKIT/mcu/sys sys
$LN ../../S32K144ZENKIT/mcu/tty tty
# for upload firmware & start
$LN ../../S32K144ZENKIT/mcu/fw fw
$LN ../../S32K144ZENKIT/mcu/drv drv
$LN ../../S32K144ZENKIT/mcu/bootload bootload

