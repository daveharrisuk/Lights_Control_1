/*file: CONFIG.h     This is include file for Lights_Control_1.ino sketch
 *----------------------------------------------------------------------------
 *
 * End user adjustable configuration data for Lights_Control_1.ino
*/
const char sCONFIG_V[] = "v3 DH development config";  /* printed @ start */
/*
 * Layout lighting control module for 10 channels
 * Author: © Dave Harris 2021    Andover, UK.
 * 
 *
  * History:
 *  4-Jan-2021 Dave Harris - Project started.
 *  
 * 
*/
#ifndef CONFIG_h_  /* include guard */
#define CONFIG_h_


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
 *   ( PWM cycle time is Arduino default 490 Hz                            )
 * mode = DAYNIGHT, DUSK, DAWN, DUSKDAWN or NIGHTONOFF
 * 
 * 
 *###### Adjust the following table to suit end-use requirements ########
 *######       But be careful with the structure format!!!       ########
*/

const Config_t CONFIG[CONFIGQTY][CHANQTY] /* ReadOnly config array */
=
{/*adr0 = TBA  config note                       */
{/*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode}*/
/*0*/{  0,   {0, 0},  {1, 100}, DAYNIGHT }, // ch0 note
/*1*/{  1,   {0, 1},  {1, 254}, DAYNIGHT }, // ch1 note
/*2*/{  1,   {3, 3},  {0, 255}, DAYNIGHT }, // ch2 note
/*3*/{  1,   {4, 4},  {0, 254}, DAYNIGHT }, // ch3 note
/*4*/{  1,   {5, 5},  {50, 150}, DAYNIGHT }, // ch4 note
/*5*/{ 15,   {6, 6},  {60, 160}, DAYNIGHT }, // ch5 note
/*6*/{ 30,   {2, 1},  {70, 254}, DAYNIGHT }, // ch6 note
/*7*/{ (3*60), {1, 2}, {254, 1}, DAYNIGHT }, // ch7 note 
/*8*/{ (4*60)+15, {2, 1}, {0, 254}, DAYNIGHT }, // ch8 note
/*9*/{  0,   {2, 1}, {0, 254}, DAYNIGHT } // ch9 note
}
,/*adr1 = TBA  config note                       */
{/*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode}*/
/*0*/{  0,   {0, 0},  {1, 100}, DAYNIGHT }, // ch0 note
/*1*/{  1,   {1, 1},  {1, 254}, DAYNIGHT }, // ch1 note
/*2*/{  1,   {2, 2},  {0, 255}, DAYNIGHT }, // ch2 note
/*3*/{  1,   {3, 3},  {0, 254}, DAYNIGHT }, // ch3 note
/*4*/{  1,   {1, 1},  {50, 150}, DAYNIGHT }, // ch4 note
/*5*/{ 15,   {1, 1},  {60, 160}, DAYNIGHT }, // ch5 note
/*6*/{ 30,   {1, 1},  {70, 254}, DAYNIGHT }, // ch6 note
/*7*/{ (3*60), {1, 1}, {254, 1}, DAYNIGHT }, // ch7 note
/*8*/{ (4*60)+15, {1, 1}, {0, 254}, DAYNIGHT }, // ch8 note
/*9*/{  1,   {1, 1}, {0, 254}, DAYNIGHT } // ch9 note   
}
,/*adr2 = TBA  config note                  */
{/*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode}*/
/*0*/{  0,   {1, 1},  {1, 254}, DAYNIGHT }, // ch0 note
/*1*/{  1,   {1, 1},  {1, 254}, DAYNIGHT }, // ch1 note
/*2*/{  1,   {1, 1},  {0, 255}, DAYNIGHT }, // ch2 note
/*3*/{  1,   {1, 1},  {0, 254}, DAYNIGHT }, // ch3 note
/*4*/{  1,   {1, 1},  {50, 150}, DAYNIGHT }, // ch4 note
/*5*/{ 15,   {1, 1},  {60, 160}, DAYNIGHT }, // ch5 note
/*6*/{ 30,   {10, 10},  {10, 254}, DAYNIGHT }, // ch6 note
/*7*/{ (3*60), {1, 1}, {254, 1}, DAYNIGHT }, // ch7 note
/*8*/{ (4*60)+15, {2, 2}, {0, 254}, DAYNIGHT }, // ch8 note
/*9*/{  0,   {1, 1}, {0, 254}, DAYNIGHT } // chan9 note   
}
,/*adr3 =   LED Test board      */
{/*{secTransit,{secDelay[0],secDelay[1]},{dc[0],dc[1]},mode}*/
/*0*/{  1, {5, 5}, {30, 250}, NIGHTONOFF }, // blue
/*1*/{  5, {0, 0}, {30, 254}, DAYNIGHT }, // blue
/*2*/{  4, {1, 1}, {30, 254}, DAYNIGHT }, // blue
/*3*/{  5, {1, 2}, {30, 250}, DUSKDAWN}, // red
/*4*/{  2, {1, 2}, {30, 250}, DUSK }, // red
/*5*/{  3, {1, 1}, {30, 250}, DAWN }, // red
/*6*/{  0, {0, 0}, {255, 30}, DAYNIGHT }, // white
/*7*/{  5, {1, 1}, {255, 30}, DAYNIGHT }, // white
/*8*/{  0, {3, 6}, {30, 255}, NIGHTONOFF }, // white
/*9*/{  1, {5, 5}, {30, 255}, NIGHTONOFF }  // white   
}
};


#endif /* include guard CONFIG_h_ */
/*
 *------ CONFIG.h ------------------------- EoF -------------------------
*/
