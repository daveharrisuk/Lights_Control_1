/*file: CONFIG.h
 *----------------------------------------------------------------------------
 * This is include file for Lights_Control.ino sketch
 * 
 * end user adjustable configuration data
 *
 * Layout lighting control module for 10 channels
 * Author: Dave Harris. Andover, UK.    © Dave Harris 2021
 * 
*/
#ifndef CONFIG_h_  /* include guard */
#define CONFIG_h_



const char sCONFIGNOTE[] = "DH development testing.."; /* on SerialMonitor  */

const uint16_t VERSION_CONFIG = 0002;           /* display on SerialMonitor */

const uint16_t SENSEAMP = 100 ;                 /* 2 Amp ADC threshold      */

const uint32_t BAUDRATE = 115200;               /* SerialMonitor BAUD rate  */

const bool     INPUTINVERT = false;             /* to invert input or not   */


/* channel configuration data
 * ---------------------------
 *
 * 4 configurations can be addressed via the address jumpers on PCB. 
 * adr = address of CONFIG table in use (0-3). Adr pins are read by setup().
 *
 * secTransit = fade up or fade down seconds  (0-255)
 *   ( 0 = quick fade = abs( dc0 - dc1 ) milli second                      )
 * secDelay[0] = switch delay seconds when switching to 'day' (0-255)
 * secDelay[1] = switch delay seconds when switching to 'night' (0-255)
 *   ( Max secTransit or secDelay is 255 = 4min 15sec = (4*60)+15 )
 * dc[0] = PWM duty cycle target when switch to 'day' (0-255)
 * dc[1] = PWM duty cycle target when switch to 'night'  (0-255)
 *   ( 1 = minimum duty cycle 1/255.  254 = max duty cycle, 254/255.       )
 *   ( 0 = steady off output, no PWM. 255 = steady on output, no PWM.      )
 *   ( Check the GAMMA8 lookup table that corrects non-linear LED response.)
 *   (   request 127 (50%) is corrected to duty cycle 36 to give true 50%. )
 *   ( PWM cycle time is 490 Hz                                            )
 * mode = DAYNIGHT, DUSK, DAWN, DUSKDAWN or NIGHTONOFF
 * 
 * 
 *###### Adjust the following table to suit end-use requirements ########
 *######       But be careful with the structures format!!!      ########
*/

const Config_t CONFIG[CONFIGSIZE][CHANSIZE] =    /* ReadOnly config array */
{  /* CONFIG adr 0 */
 { /*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode} */
/*0*/{  0,   {0, 0},  {1, 100}, DAYNIGHT }, /*ch0 note */
/*1*/{  1,   {0, 1},  {1, 254}, DAYNIGHT }, /*ch1 note */
/*2*/{  1,   {3, 3},  {0, 255}, DAYNIGHT }, /*ch2 note */
/*3*/{  1,   {4, 4},  {0, 254}, DAYNIGHT }, /*ch3 note */
/*4*/{  1,   {5, 5},  {50, 150}, DAYNIGHT }, /*ch4 note */
/*5*/{ 15,   {6, 6},  {60, 160}, DAYNIGHT }, /*ch5 note */
/*6*/{ 30,   {2, 1},  {70, 254}, DAYNIGHT }, /*ch6 note */
/*7*/{ (3*60), {1, 2}, {254, 1}, DAYNIGHT }, /*ch7 note */ 
/*8*/{ (4*60)+15, {2, 1}, {0, 254}, DAYNIGHT }, /*ch8 note */
/*9*/{  0,   {2, 1}, {0, 254}, DAYNIGHT } /*ch9 note */
 }
 , /* CONFIG adr 1 */
 { /*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode} */
/*0*/{  0,   {0, 0},  {1, 100}, DAYNIGHT }, /*ch0 note */
/*1*/{  1,   {1, 1},  {1, 254}, DAYNIGHT }, /*ch1 note */
/*2*/{  1,   {2, 2},  {0, 255}, DAYNIGHT }, /*ch2 note */
/*3*/{  1,   {3, 3},  {0, 254}, DAYNIGHT }, /*ch3 note */
/*4*/{  1,   {1, 1},  {50, 150}, DAYNIGHT }, /*ch4 note */
/*5*/{ 15,   {1, 1},  {60, 160}, DAYNIGHT }, /*ch5 note */
/*6*/{ 30,   {1, 1},  {70, 254}, DAYNIGHT }, /*ch6 note */
/*7*/{ (3*60), {1, 1}, {254, 1}, DAYNIGHT }, /*ch7 note */ 
/*8*/{ (4*60)+15, {1, 1}, {0, 254}, DAYNIGHT }, /*ch8 note */
/*9*/{  1,   {1, 1}, {0, 254}, DAYNIGHT } /*ch9 note */    
 }
 ,/* CONFIG adr 2 */
 { /*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode} */
/*0*/{  0,   {1, 1},  {1, 254}, DAYNIGHT }, /*ch0 note */
/*1*/{  1,   {1, 1},  {1, 254}, DAYNIGHT }, /*ch1 note */
/*2*/{  1,   {1, 1},  {0, 255}, DAYNIGHT }, /*ch2 note */
/*3*/{  1,   {1, 1},  {0, 254}, DAYNIGHT }, /*ch3 note */
/*4*/{  1,   {1, 1},  {50, 150}, DAYNIGHT }, /*ch4 note */
/*5*/{ 15,   {1, 1},  {60, 160}, DAYNIGHT }, /*ch5 note */
/*6*/{ 30,   {10, 10},  {10, 254}, DAYNIGHT }, /*ch6 note */
/*7*/{ (3*60), {1, 1}, {254, 1}, DAYNIGHT }, /*ch7 note */ 
/*8*/{ (4*60)+15, {2, 2}, {0, 254}, DAYNIGHT }, /*ch8 note */
/*9*/{  0,   {1, 1}, {0, 254}, DAYNIGHT } /* chan9 note */    
 }
 , /* CONFIG adr 3 - Test LED board */
 { /*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode} */
/*0*/{  1, {3, 2}, {200, 0}, NIGHTONOFF }, /* blue */
/*1*/{  5, {1, 1}, {0, 200}, DAYNIGHT }, /* blue */
/*2*/{  5, {1, 1}, {0, 200}, DAYNIGHT }, /* blue */
/*3*/{  5, {1, 2}, {0, 250}, DUSKDAWN}, /* red */
/*4*/{  2, {1, 2}, {0, 250}, DUSK }, /* red */
/*5*/{  3, {1, 1}, {0, 250}, DAWN }, /* red */
/*6*/{  0, {0, 0}, {127, 0}, DAYNIGHT }, /* white */
/*7*/{  5, {1, 1}, {127, 0}, DAYNIGHT }, /* white */ 
/*8*/{  1, {3, 6}, {127, 0}, NIGHTONOFF }, /* white */
/*9*/{  1, {5, 5}, {0, 200}, NIGHTONOFF }  /* white */    
 }
};


#endif /* CONFIG_h_ */
/*------------------------------------- EoF --------------------------------*/
