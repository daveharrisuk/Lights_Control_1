# Lights_Control_1
Arduino Lighting control module for 10 LED strings.

10 PWM channels drive 10 LED strings. 

Input (Day/Night) triggers channel modes...
 DAYNIGHT : Duty cycle (DC) transition on trigger
 DAWN : DC transitions on input changing to day
 DUSK : DC transitions on input changing to night
 DUSKDAWN : DC transitions on input change
 NIGHTONOFF : During night time, LEDs transition at durations

Transition config:
 - Delay seconds before transitions.
 - Transition duration in seconds.

Duty Cycle Day and Night config.


Configurations are defined in seperate CONFIG.h file.
This sketch has 4 configs builtin. Address jumpers select 1 of 4 configs.

Over current protection. Audio alarm sounder.
Serial Monitor provides simple diagnostics. 

This version uses logic signal input (opto isolated) as day/night trigger.

Possible future options...
  SPI bus on PCB to have input trigger via CBUS.
  LCD via I2C bus & rotary encoder for local configuration method.
