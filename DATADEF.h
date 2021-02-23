/*file: DATADEF.h     This is include file for Lights_Control_1.ino sketch
 *----------------------------------------------------------------------------
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

const uint8_t CHANQTY = 10;  /* qty of PWM channels used for light strings */

const uint8_t CONFIGQTY = 4; /* qty of CONFIGs in sketch. Selected by Adr  */

const uint8_t MAXDC = 255;



enum Input_t : uint8_t  /* module input codes. for input global var */ 
{ 
  DAY = 0, 
  NIGHT = 1,
  UNDEF = 2  /* UNDEF is removed by setup() */ 
};

const char sInput[3][8] =  /* input code string */
{
  "0:Day",
  "1:Night",
  "?input?"
};



enum State_t : uint8_t     /* channel state codes. For var[] */
{
  STEADY,
  TRANSIT,
  DELAY
};

const char sState[3][5] =  /* state code string */
{
  "Stdy",
  "Tran",
  "Dly"
};



enum Mode_t : uint8_t      /* channel mode codes. For CONFIG[] */
{
  DAYNIGHT,
  DUSK,
  DAWN,
  DUSKDAWN,
  NIGHTONOFF
};

const char sMode[5][11] =   /* Mode code string */
{
  "DayNight",
  "Dusk",
  "Dawn",
  "DuskDawn",
  "NightOnOff" 
};

/* Mode...
 *  
 *  DAYNIGHT 
 *  --------
 *  Input change to 1,
 *   after secDelay[1], dc transitions from current dc to dc[1].
 *   Stay steady until...
 *  Input change to 0,
 *   after secDelay[0], dc transitions from current dc to dc[0].
 *  
 *  DUSKDAWN
 *  -----------
 *  Input change to 1,
 *   after secDelay[1], dc transitions from current dc to dc[1],
 *   after secDelay[0], dc transitions from current dc to dc[0].
 *  Stay steady until...
 *  Input change to 0... as per input change to 1.
 *   
 *  NIGHTONOFF
 *  ---------
 *   Input change to NIGHT,
 *    after secDelay[1], transitions from current dc to dc[1],
 *    after secDelay[0], transitions from dc[1] to dc[0],
 *    after secDelay[1], transitions from dc[0] to dc[1],
 *    ... repeats until input change to 0.
 *   Input change to DAY,
 *    after secDelay[0], transitions from current dc to dc[0].  
 *    
 *  [0] is day, [1] is night
 *  dc[0] and dc[1] can be configured as any value you want.
*/



struct Config_t  /* channel CONFIG array structure. For CONFIG[][] */
{
  uint8_t secTransit;  /* seconds to Transit between dc[0] and dc[1] levels */
                       /*                                           0 - 255 */
 
  uint8_t secDelay[2]; /* seconds Delay after trigger switch.               */
                       /*                      Delay[0] & Delay[1]  0 - 255 */

  uint8_t dc[2];       /* duty cycle target day and night values.           */
                       /*                           dc[0] & dc[1]   0 - 255 */
 
  Mode_t  mode;        /* channel operating mode.  See enum Mode_t    0 - 4 */
};




struct var_t   /* channel data structure used for var[] */
{
  uint8_t  dc;        /* current duty cycle value                   0 - 255 */
  
  uint32_t msCount;   /* millis count for step or delay          0 - 255000 */
                      
  uint8_t  secCount;  /* seconds count                              0 - 255 */
  
  State_t  state;     /* See enum State_t                             0 - 3 */
  
  bool     phase;     /* phase 0 or 1                                 0 - 1 */
  
  uint16_t msPerStep; /* ms between duty cycle inc/dec              0 - 65k */
                      /* Derived from CONFIG secTransit during setup()      */  
};


const uint32_t MAXmsPERSTEP = 65534;  /* fix overflow if abs(DC0 - DC1) < 4 */



#endif /* DATADEF_h_  */
/*--------------------------------------- EoF ------------------------------*/
