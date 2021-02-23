/*file: PIN.h     This is include file for Lights_Control_1.ino sketch
 *------------------------------------------------------------------------------
 *
 * Lighting Control module for 10 channels
 * 
 * Author: Dave Harris. Andover, UK.    © Dave Harris 2021
 * 
 * IO pin definitions
 * 
 * Target MCU is Arduino compatable 'MEGA 2560 PRO (EMBED)' or Arduino 'MEGA'
 * 
 * Target PCB is Lights_Control_1   V1 Rev A
 * 
*/
#ifndef PIN_h_  /* include guard */
#define PIN_h_


/* Define the pins used
 * ---------------------
 *  
 *  
 * MEGA ATmega2560 has...
 *  External interrupt INT pins are 2, 3, 18, 19, 20 and 21. 
 *   2/3 used by timer comparators 3B and 3C. 2 assigned to SPI_INT.
 *   18/19 free, assigned to Encoder. 
 *   20/21 reserved for I2C. 
 *
 *  The choice of pins for PWM is governed by the usage of timers(0-5) and
 *   timer comparators (A, B & C) and their pins.
 * 
 *  Following PWM pins and timer usage...
 *  Pin Timer Usage by fuction/library
 *  --- ----  -------------------------
 *   2  3B    Free, but is an INT pin. Assigned to SPI_INT signal.
 *   3  3C    Free.  Assigned to PWM0.
 *   4  0B    millis, used in this code.
 *   5  3A    Free.  Assigned to PWM1.
 *   6  4A    Free.  Assigned to PWM2.
 *   7  4B    Free.  Assigned to PWM3.
 *   8  4C    Free.  Assigned to PWM4.
 *   9  2B    tone, lib not used.  Assigned to PWM5.
 *  10  2A    tone, lib not used.  Assigned to PWM6.
 *  11  1A    timer1/servo. TimerOne lib used in this code.
 *  12  1B    timer1/servo. TimerOne lib used in this code.
 *  13  0A    millis, used in this code.
 *  44  5C    Free.  Assigned to PWM7.
 *  45  5B    Free.  Assigned to PWM8.
 *  46  5A    Free.  Assigned to PWM9.
 *  
*  
 * Array of PWM pins
*/
const uint8_t PWMPIN[CHANQTY] = { 3, 5, 6, 7, 8, 9, 10, 44, 45, 46};


/* Alarms and status                                                */

const uint8_t PINAWDSIG = 37; /* AudioWarningDevice signal          */

const uint8_t PINLEDRED = 36; /* Red LED.    Error                  */

const uint8_t PINLEDGRN = 35; /* Green LED.  Status green           */

const uint8_t PINLEDYEL = 34; /* Yellow LED. Status yellow          */


/* Module address input. Allows 4 configs in sketch, in CONFIG.h    */

const uint8_t PINADR0 = 33;   /* module config address 2^0 = 0 or 1 */
const uint8_t PINADR1 = 32;   /* module config address 2^1 = 0 or 2 */


/* inputs                                                           */

const uint8_t PINSENSE = A0;   /* All MOSFET via 0R05 for 3.0 A max */

const uint8_t PININPUT = A6;   /* module input via OptoIsolator     */

const uint8_t PINTACTSW = A12; /* Tactile push button switch        */

const uint8_t PINBLUE = A14;   /* PolyFuse sense, Volts on blue LED */


/* SPI bus   ### for future upgrade ###                             */

const uint8_t PINSPIMISO = 50; /* Master In, Slave Out  (HW SPI 50) */
const uint8_t PINSPIMOSI = 51; /* Master Out, Slave In  (HW SPI 51) */
const uint8_t PINSPISCK  = 52; /* clock                 (HW SPI 52) */
const uint8_t PINSPISS   = 53; /* Slave Select                      */
const uint8_t PINSPIINT  = 2;  /* Interrupt (must be an INT pin)    */


/* Rotary Encoder    ### for future upgrade ###                     */

const uint8_t PINENCPHA = 18;  /* encoder phase A (must be INT pin) */
const uint8_t PINENCPHB = 17;  /* encoder phase B                   */
const uint8_t PINENCSW  = 19;  /* encoder SW (must be INT pin)      */


/* I2C bus    ### for future upgrade ###                            */

const uint8_t PINI2CSCL = 20;  /* I2C clock     (HW I2C pin is 20 ) */
const uint8_t PINI2CSDA = 21;  /* I2C data      (HW I2C pin is 21 ) */


const uint8_t PINTP_D30 = 30;  /* MEGA MCU pin D30 maps CPU pin PC7 */
                               /* macros to fast digitalWrite       */
#define SetPINTP_D30   PORTC = PORTC | B10000000
#define ClrPINTP_D30   PORTC = PORTC & B01111111


const uint8_t PINTP_D31 = 31;  /* MEGA MCU pin D31 maps CPU pin PC6 */
                               /* macros to fast digitalWrite       */
#define SetPINTP_D31   PORTC = PORTC | B01000000
#define ClrPINTP_D31   PORTC = PORTC & B10111111


#endif /* PIN_h_  */
/* -------------------------------EoF------------------------------ */
