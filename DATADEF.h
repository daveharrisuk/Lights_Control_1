/*file: DATADEF.h
 *----------------------------------------------------------------------------
 * This is include file for Lights_Control.ino sketch
 * 
 * data structure definitions
 *
 * Layout lighting control module for 10 channels
 * Author: Dave Harris. Andover, UK.    © Dave Harris 2021
 * 
*/
#ifndef DATADEF_h_  /* include guard */
#define DATADEF_h_

/*------------------------- data definitions -------------------------------*/

/* table sizes */

const uint8_t CHANSIZE = 10;  /* qty of PWM channels used for light strings */

const uint8_t CONFIGSIZE = 4; /* qty of CONFIGs in sketch. Selected by Adr  */



enum Mode_t : uint8_t 
{
  DAYNIGHT = 0, DUSK = 1, DAWN = 2, DUSKDAWN = 3, NIGHTONOFF = 4
};
/* Mode
 *  
 *  DAYNIGHT 
 *  --------
 *  Input change to 1,
 *   after secDelay[1], dc transitions from current dc to dc[1].
 *   Stays steady until...
 *  Input change to 0,
 *   after secDelay[0], dc transitions from current dc to dc[0].
 *  
 *  DUSKDAWN
 *  -----------
 *  Input change to 1,
 *   after secDelay[1], dc transitions from current dc to dc[1],
 *   after secDelay[0], dc transitions from current dc to dc[0].
 *  Stays steady until...
 *  Input change to 0... as per input change to 1.
 *   
 *  NIGHTONOFF
 *  ---------
 *   Input change to 1,
 *    after secDelay[1], transitions from current dc to dc[1],
 *    after secDelay[0], transitions from dc[1] to dc[0],
 *    after secDelay[1], transitions from dc[0] to dc[1],
 *    ... repeats until input change to 0.
 *   Input change to 0,
 *    after secDelay[0], transitions from current dc to dc[0].  
 *    
 *  [0] is day, [1] is night
 *  dc[0] and dc[1] can be configured as any value you want.
*/
const char sMode[5][10] = { "DayNight","Dusk","Dawn","DuskDawn","NightOnOf" };


struct Config_t        /* channel CONFIG array structure. Use in CONFIG[][] */
{
  uint8_t secTransit;  /* seconds to Transit between dc[0] and dc[1] levels */
                       /*                                           0 - 255 */
 
  uint8_t secDelay[2]; /* seconds Delay after trigger switch.               */
                       /*                      Delay[0] & Delay[1]  0 - 255 */

  uint8_t dc[2];       /* duty cycle target day and night values.           */
                       /*                           dc[0] & dc[1]   0 - 255 */
 
  Mode_t  mode;        /* channel operating mode.  See enum Mode_t    0 - 4 */
};



enum State_t : uint8_t  /* channel state codes. Used for var[]              */
{
  STEADY = 0, TRANSIT = 1, DELAY = 2 
};
const char sState[3][80] = { "Steady", "Transit", "Delay" }; /* state codes */



struct var_t     /* channel variables data structure */
{
  uint8_t  dc;        /* current duty cycle value                   0 - 255 */
  uint16_t msPerStep; /* ms between duty cycle increment/decrement 0 - 1000 */
  uint32_t msCount;   /* millis count to next step              0 - 255,000 */
  uint8_t  secCount;  /* seconds count                              0 - 255 */
  State_t  state;     /* See enum State_t                             0 - 3 */
  bool     phase;     /* phase 0 or 1                                 0 - 1 */
};



#endif /* DATADEF_h_  */
/*--------------------------------------- EoF ------------------------------*/
