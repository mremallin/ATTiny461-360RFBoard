ATTiny461-360RFBoard
====================

This is some code to allow you to controller the Xbox 360 RF Board with an ATTiny461.

Requirements:
 - avr-gcc

To Use:
 0. Change Makefile for your programmer!
 1. make install
 2. Watch the pretty LEDs on your RF board!

 Press the sync button to sync controllers.

Connections (RF Board Pin):
 - SYNC (1) to INT0
 - CLK  (2) to PA0
 - DATA (3) to PB0
