#!/bin/bash

export LN="/bin/ln -s -T "

# need tty
$LN ../../S32K144ZENKIT/mcu/sys sys
$LN ../../S32K144ZENKIT/mcu/tty tty

