/*file: Lights_Control_1.ino
 *--------------------------------------------------------------------------
 *
 * Layout lighting control module for 10 channels of LED strings
 * 
 * v1 Logic input as trigger
*/
const char sTITLE[] = "\n Lights_Control_1 © Dave Harris 2021 v1.00";
/* 
 * Author: Dave Harris. Andover, UK.  MERG #2740.
 * 
 * TargetPCB : Lights_Control V1.0 revA (see KiCad files)
 * MCU Board : 'MEGA 2560 PRO (EMBED)' (code should be OK on 'Arduino MEGA')
 * Processor : ATmega2560
 * Framework : Arduino 1.8.13
 * FlashMem  : 8.3k of 254k
 * GlobalVar : 1.4k of 8k
 * 
 *
 *----------------------------- History -------------------------------------
 * 
 *  4-Jan-2021 Dave Harris (DH) Project started
 *  9-Jan-2021 DH, v0.01  breadboard first test
 * 23-Jan-2021 DH, v0.02  add channel Modes
 *  2-Feb-2021 DH, v0.03  add TimerOne lib and 1 ms ISR
 *  4-Feb-2021 DH, v1.00  release
 *---------------------------- include files --------------------------------
*/

#include "DATADEF.h"   /* data structure definitions                        */

#include "CONFIG.h"    /* user editable configuration tables                */

#include "PIN.h"       /* module pin definitions                            */

#include "GAMMA8.h"    /* Gamma correction table                            */

#include <TimerOne.h>  /* use timer1 for 1 ms Interrupt Service Routine     */



/*------------------------------ globals -----------------------------------*/


uint8_t  adr;                 /* address of CONFIG used 0-3  Set in setup() */

bool     input = 0;           /* input state 0 or 1                         */


volatile var_t var[CHANSIZE]; /* channel variable array.  Used by ISR       */


uint16_t amps = 0;            /* ADC reading of PINSENSE     0-1023         */


extern const Config_t CONFIG[CONFIGSIZE][CHANSIZE]; /* in CONFIG.h          */




/*------------------------------- soundAlarm() -------------------------------
 * sound AWD piezo buzzer
*/

void soundAlarm( uint8_t seconds )
{
  for( uint16_t i = 0; i < ( 5000 * seconds ) ; i++ )
  {
    digitalWrite( PINAWDSIG, HIGH ); /* about 5 kHz */
    delayMicroseconds( 100 );
    digitalWrite( PINAWDSIG, LOW );
    delayMicroseconds( 100 );
  }  
}



/*-------------------------------printConfig()---------------------------------
 * print config to Serial
*/

void printConfig( uint8_t chan )
{
  char str[70];
  static const char fmt[] = 
   "ch%u Trn=%03us Dly0=%03us Dly1=%03us dc0=%03u dc1=%03u step=%03ums %s";
  
  sprintf( str, fmt
     , chan
     , CONFIG[adr][chan].secTransit
     , CONFIG[adr][chan].secDelay[0]
     , CONFIG[adr][chan].secDelay[1]
     , CONFIG[adr][chan].dc[0]
     , CONFIG[adr][chan].dc[1]
     , var[chan].msPerStep     // derived config value
     , sMode[CONFIG[adr][chan].mode]
    );
  Serial.println( str );
}



/*--------------------------------printVar()---------------------------------
 * debug message
*/

void printVar( uint8_t chan )
{
  char str[60];
  static const char fmt[] = "ch%u dc=%03u phase:%u state:%s";

  sprintf( str, fmt,
    chan, var[chan].dc, var[chan].phase, sState[var[chan].state] );
  Serial.println( str );
}



/*-------------------------------- printAmps() -------------------------------
 * debug message
*/

void printAmps()
{
  char str[60];
  static const char fmt[] = "%u Amp";
  sprintf( str, fmt, amps );
  
  Serial.println( str );
}



/*------------------------------ readConfigAdr() -----------------------------
 * 
 * read the config address from the address jumpers into global var adr   0-3
*/

void readConfigAdr()
{
  adr = digitalRead( PINADR0 ) + ( digitalRead( PINADR1 ) << 1 );

  Serial.print("Config Adr=");
  Serial.println( adr );
}



/*------------------------------startNewPhase() ------------------------------
 * 
 * The input has changed, so setup new phase for each channel
*/

void startNewPhase()
{
  noInterrupts();              /* all variables here are manipulated by ISR */
  
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )          /* loop all channels  */
  {
    var[ch].secCount = 0;
    var[ch].msCount = 0;

    var[ch].state = TRANSIT;          /* default values - updated by switch */
    var[ch].phase = 0;                /* allow dc to transit back to dc0    */

    switch( CONFIG[adr][ch].mode )
    {
      case DAYNIGHT :
        var[ch].state = DELAY;
        var[ch].phase = input;
        break;
        
      case DUSK :
        if( input == 1 )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
        
      case DAWN :
        if( input == 0 )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
        
      case DUSKDAWN :
        var[ch].state = DELAY;
        var[ch].phase = 1;
        break;
        
      case NIGHTONOFF :
        if( input == 1 )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
    }
  } /* end of loop */
  
  interrupts();
}



/*-------------------------------- processChannels() -------------------------
 * 
 * timer1 Interrupt Service Routine every 1 milli second
 * 
 * 
 * cycle though the channels and process them
 * 
 * target DutyCycle, dc, is CONFIG[ch].dc[phase]      0-255
 * current dc is in var[ch].dc                        0-255
 * secs to delay is in CONFIG[ch].secDelay[phase]     0-255
 * Delay seconds counter is in var[ch].secCount       0-255
 * ms between inc/dec steps is in var[ch].msPerStep   0-1000
 *
*/

void processChannels()          /*! This is an ISR which runs every 1 ms  !*/
{
  PORTC = PORTC | B01000000;    /* PIN ISRTIME = D31 aka PC6 = high.       */
                                /* pulse width high = ISR run time.        */
                                /* 16 MHz ATmega2560 min 21 us, peak 87 us */

  uint8_t countTransits = 0;    /* sets LED_builtin if Transits active     */

                                               /* loop all channels        */
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )
  {
    if( var[ch].state != STEADY )          /* is state TRANSIT or DELAY?   */
    {
      var[ch].msCount++;
      
      if( var[ch].state == DELAY )         /* is state DELAY?              */
      {

        if( var[ch].msCount > 1000 )
        {
          var[ch].msCount = 0;
          var[ch].secCount++;
          
          if( var[ch].secCount > CONFIG[adr][ch].secDelay[var[ch].phase] )
          {
            //printVar( ch ); /* debug */
            
            var[ch].state = TRANSIT;               /* end DELAY            */
            var[ch].secCount = 0;
          }
        }
      }
      else                                         /* so, in TRANSIT       */
      {
        countTransits++;                           /* drives LED_builtin   */
        
        if( var[ch].msCount > var[ch].msPerStep )
        {
          var[ch].msCount = 0;
          if( var[ch].dc == CONFIG[adr][ch].dc[var[ch].phase] )
          { 
            var[ch].state = STEADY;                /* dc transit complete  */
            
            switch( CONFIG[adr][ch].mode )         /* trigger new phase    */
            {
              case NIGHTONOFF :
                if( input == 1 )
                {
                  var[ch].phase = ! var[ch].phase;
                  var[ch].state = DELAY;
                }
                break;

              case DUSK :
                if( var[ch].phase == 1 )
                {
                  var[ch].phase = 0;
                  var[ch].state = DELAY;                  
                }
                break;
                
              case DAWN :
                if( var[ch].phase == 1  )
                {
                  var[ch].phase = 0;
                  var[ch].state = DELAY;                  
                }
                break;
                
              case DUSKDAWN :
                if( var[ch].phase == 1 )
                {
                  var[ch].phase = 0;
                  var[ch].state = DELAY;
                }
                break;
                
              case DAYNIGHT : 
                break;
            }
          }
          else              /* in Transit, current dc is GT or LT target dc */
          {
            if( var[ch].dc > CONFIG[adr][ch].dc[var[ch].phase] )
            {
              var[ch].dc--;                              /* GT so decrement */
            }
            else
            {
              var[ch].dc++;                              /* LT so increment */
            }
            //printVar( ch ); /* debug */

            analogWrite( PWMPIN[ch], GAMMA8[var[ch].dc] );
            
          }  /* end GT or LT                 */
        }  /* end msCount > msPerStep        */
      }  /* end if Transit                   */
    }  /* end if state NOT steady            */
  }  /* next ch                              */

  digitalWrite( LED_BUILTIN, countTransits );     /* 0 LED off, >0 LED on */

  PORTC = PORTC & B10111111;           /* PIN ISRTIME = D31 aka PC6 = low */
}



/*---------------------- setupChannels() ------------------------------------
 * 
 * preset channel variables derived from channel constants and start the PWM
 *  
 * called from setup(), before timerOne is started
*/

void setupChannels()
{
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )     /* loop through channels   */
  {
    if( CONFIG[adr][ch].secTransit == 0 )        /* zero transit is special */
    { 
                                 /* dc transit rapidly to target duty cycle */
      var[ch].msPerStep = 1;
    }
    else
    {             /*  non-zero transit calculate ms per step  */
    
      uint16_t diff = abs( CONFIG[adr][ch].dc[0] - CONFIG[adr][ch].dc[1] );
      
      uint32_t msSt = ( CONFIG[adr][ch].secTransit * 1000UL ) / diff;
      
      if( msSt > 999 )       /* issue: long Transit & small diff = overflow */
      {
        msSt = 999;          /* 1 second step is a realistic limit          */
      }
      var[ch].msPerStep = msSt;
    }
    
    var[ch].dc = 36; //CONFIG[adr][ch].dc[input];

    analogWrite( PWMPIN[ch], var[ch].dc );        /* start PWM channel     */
    
    printConfig( ch );
  } 
  delay( 500 );
  
  startNewPhase();
}



/*----------------------------- isInputChanged() ----------------------------
 * 
 * read output of opto, debounce and set global var input
 * 
 * return true if input changed and false if unchanged
*/

bool isInputChanged()
{
  static uint32_t msStamp  = 0;             /* timestamp last input change */

  char str[20];
  static const char fmt[] = "input %u";
  
  bool rawInput = digitalRead( PININPUT );

  if( INPUTINVERT == true )
  {
    rawInput = ! rawInput;
  }
  
  if( ( rawInput != input ) && ( millis() > msStamp + 15 ) )
  {
    input = ! input;  /* invert */

    msStamp = millis();

    digitalWrite( PINLEDGRN, input );

    sprintf( str, fmt, input );
    Serial.println( str );

    return true;
  }
  return false;
}



/*------------------------------ isLEDsupplyOK() -----------------------------
 * 
 * if blue LED is not lit, the 12 V line fail or PolyFuse tripped
*/
bool isLEDsupplyOK()
{ 
  if( digitalRead( PINBLUE ) == 0 )
  {
    Serial.println("PolyFuse/12V fail");
    
    digitalWrite( PINLEDRED, HIGH );
    return false;
  }
  digitalWrite( PINLEDRED, LOW );
  return true;  
}




/*------------------------------- setDCmin() ----------------------------
 * In case of over Amps. set all channel duty cycle to min
 * If setdc > 0 set to that value. If setdc = 0 then restore running dc
*/
void setDCmin( uint8_t setdc )
{
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )
  {
    if( setdc > 0 )
    {
      if( setdc < var[ch].dc )
      {
        analogWrite( PWMPIN[ch], setdc );
      }
    }
    else
    {
      analogWrite( PWMPIN[ch], var[ch].dc );
    }
  }
}



/*------------------------------ isOverAmp() ------------------------------
 * 
 * read sense resistor to measure Amps 
 * returns true if over Amp
*/

bool isOverAmp()
{

  amps = analogRead( PINSENSE );
  
  if( amps > SENSEAMP )
  {
    digitalWrite( PINLEDRED, HIGH );
    return true;
  }
  digitalWrite( PINLEDRED, LOW );
  return false;
}



/*----------------------------------- testPower() ----------------------------
 * 
 * If over Amp then alarm and reduce all duty cycles
*/
void testPower()
{
  uint8_t setdc = 10;
  
  if( isOverAmp() == true )
  {
    Serial.print( amps );
    Serial.println(" OverAmp");
    
    soundAlarm( 1 );
    
    noInterrupts();                  /* stop ISR processing LED channels    */
    
    while( isOverAmp() == true )
    {
      if( setdc > 1 )
      {
        setDCmin( setdc-- );
      }
      soundAlarm( 1 );
    }
    setDCmin( 0 ); /* 0 to reset to running levels */
    
    interrupts();                   /* restart ISR processing LED channels  */
  } 
  
  while( isLEDsupplyOK() == false)
  {
    soundAlarm( 2 );
  }
}



/*------------------------------------------- setup() ------------------------
 * Arduino calls this on Reset or PowerUp
*/
void setup() 
{
  pinMode( PININPUT, INPUT_PULLUP ); /* Day or Night via opto isolate       */
  pinMode( PINADR0,  INPUT_PULLUP ); /* Config Address bit 2^0 jumper link  */
  pinMode( PINADR1,  INPUT_PULLUP ); /* Config Address bit 2^1 jumper link  */
  pinMode( PINBLUE,   INPUT );       /* PolyFuse trip sense from blue LED   */
  pinMode( PINENCPHA, INPUT );       /* Rotary encoder phase A via low-pass */
  pinMode( PINENCPHB, INPUT );       /* Rotary encoder phase B via low-pass */
  pinMode( PINENCSW,  INPUT );       /* Rotary encoder switch via low-pass  */
  pinMode( PINSENSE,  INPUT );       /* not strictly needed, ADC input      */
  
  pinMode( PINAWDSIG,   OUTPUT );    /* AudioWarningDevice: Piezo buzzer    */
  pinMode( PINLEDRED,   OUTPUT );    /* Red LED: alarm                      */
  pinMode( PINLEDYEL,   OUTPUT );    /* Yellow LED: warning                 */
  pinMode( PINLEDGRN,   OUTPUT );    /* Green LED: input state LED          */
  pinMode( LED_BUILTIN, OUTPUT );    /* Red LED_Builtin: PWM Transit active */
  pinMode( PINISRTIME,  OUTPUT );    /* Test Point - measure ISR duration   */
  
  digitalWrite( PINLEDRED, LOW );
  digitalWrite( PINLEDYEL, LOW );
  digitalWrite( PINLEDGRN, LOW );
  digitalWrite( LED_BUILTIN, LOW );
  digitalWrite( PINISRTIME, LOW );
 
  Serial.begin( BAUDRATE );
  while( ! Serial );
  delay( 200 );
  Serial.println( sTITLE );
  Serial.print("CONFIG.h v");
  Serial.print( VERSION_CONFIG );
  Serial.print(" ");
  Serial.println( sCONFIGNOTE );

  readConfigAdr();

  setupChannels();

  soundAlarm( 1 );

  analogReference( INTERNAL1V1 );
  
  Timer1.initialize( 1000 );                    /* set timer to 1 ms        */
  Timer1.attachInterrupt( processChannels );

  Serial.println("send '.' for Amp, '0' for ch0 vars, ,, '9' for ch9 vars.");
}



/*----------------------------------------------- loop() ---------------------
 * 
 * Arduino calls this after setup() has run
*/

void loop()
{   
  testPower();
  
  if( isInputChanged() == true )
  {
    startNewPhase();
  }

  if (Serial.available() > 0) 
  {
    char rx = Serial.read();
    
    if( rx == '.' ) printAmps();
    
    if( rx >= '0' && rx <= '9' ) printVar( rx - '0' );
    
  }
  
  delay( 1 );

} /* restart loop() */


/* Lights_Control_1.ino -------------------- EoF ---------------------------*/
