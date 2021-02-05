/*file: Lights_Control_1.ino
 *--------------------------------------------------------------------------
 *
 * Layout lighting control module for 10 LED strings.
 * 
 * 10 PWM channels drive 10 LED strings. 
 * 
 * Input (Day/Night) triggers channel modes...
 *  DAYNIGHT : Duty cycle (DC) transition on trigger
 *  DAWN : DC transitions on input changing to day
 *  DUSK : DC transitions on input changing to night
 *  DUSKDAWN : DC transitions on input change
 *  NIGHTONOFF : During night time, LEDs transition at durations
 * 
 * Transition config:
 * - Delay seconds before transitions.
 * - Transition duration in seconds.
 * 
 * Duty Cycle Day and Night config.
 * 
 * 
 * Configurations are defined in seperate CONFIG.h file.
 * This sketch has 4 configs builtin. Address jumpers select 1 of 4 configs.
 * 
 * Over current protection. Audio alarm sounder.
 * Serial Monitor provides simple diagnostics. 
 * 
 * This version uses logic signal input (opto isolated) as day/night trigger.
 * 
 * Possible future options...
 *  SPI bus on PCB to have input trigger via CBUS.
 *  LCD via I2C bus & rotary encoder for local configuration method.
*/

const char sTITLE[] = "\nLights_Control_1 © Dave Harris 2021 v1.01";
/*                      -------------------------------------------
 * Author: Dave Harris. Andover, UK.  MERG #2740.
 * 
 * 
 * TargetPCB : Lights_Control V1.0 revA (see KiCad files)
 * MCU Board : 'MEGA 2560 PRO (EMBED)' (code should be OK on 'Arduino MEGA')
 * Processor : ATmega2560
 * Framework : Arduino 1.8.13
 * FlashMem  : 8.4k of 254k
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
 *  5-Feb-2021 DH, v1.01  update overAmp. PCB change C3 0.1 uF to 1.0 uF
 *  
 *  
 *---------------------------- include files --------------------------------
*/

#include "DATADEF.h"   /* data structure definitions                        */

#include "CONFIG.h"    /* user editable configuration tables                */

#include "PIN.h"       /* module pin definitions                            */

#include "GAMMA8.h"    /* Gamma correction table                            */

#include <TimerOne.h>  /* use timer1 for 1 ms Interrupt Service Routine     */



/*------------------------------ globals -----------------------------------*/


uint8_t  adr;                 /* address of CONFIG used 0-3  Set in setup() */

Input_t  input = DAY;         /* input state 0 Day or 1 night               */
  

volatile var_t var[CHANSIZE]; /* channel variable array.  Used by ISR       */


uint16_t amps = 0;            /* ADC reading of PINSENSE     0-1023         */


extern const Config_t CONFIG[CONFIGSIZE][CHANSIZE]; /* in CONFIG.h          */




/*-------------------------------------- soundAlarm() ----------------------
 * sound AWD piezo buzzer for n seconds. About 4 kHz.
*/

void soundAlarm( uint8_t seconds )
{
  for( uint16_t i = 0; i < ( 4000 * seconds ) ; i++ )
  {
    digitalWrite( PINAWDSIG, HIGH );
    delayMicroseconds( 121 );
    digitalWrite( PINAWDSIG, LOW );
    delayMicroseconds( 121 );
  }  
}



/*---------------------------------- printConfig() -------------------------
 * print channel config to Serial
*/

void printConfig( uint8_t chan )
{
  char str[70];
  static const char fmt[] = 
   "ch%u Trn:%03us Dly0:%03us Dly1:%03us dc0:%03u dc1:%03u step:%03ums %s";
  
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



/*-------------------------------- printVar() -------------------------------
 * print channel var to Serial
*/

void printVar( uint8_t chan )
{
  char str[60];
  static const char fmt[] = " ch%u dc:%03u phase:%u state:%s";

  sprintf( str, fmt,
    chan, var[chan].dc, var[chan].phase, sState[var[chan].state] );
  Serial.println( str );
}



/*------------------------------------------ printAmps() --------------------
 * prints, roughly, measured mA to Serial
 * displays as multiples of 20 mA   ## anything below 20mA shows as zero ##
*/

void printAmps()
{
  char str[60];
  static const char fmt[] = " %u mA";
  sprintf( str, fmt, ( amps * 20 ) );
  
  Serial.println( str );
}



/*----------------------------------- readConfigAdr() -----------------------
 * 
 * read the config address from the address jumpers into global var adr  0-3
*/

void readConfigAdr()
{
  adr = digitalRead( PINADR0 ) + ( digitalRead( PINADR1 ) << 1 );

  Serial.print("Config Adr=");
  Serial.println( adr );
}



/*------------------------------------ startNewPhase() ----------------------
 * 
 * setup new phase for each channel, as the day/night input has changed
*/

void startNewPhase()
{
  noInterrupts();        /* all variables here are manipulated by interrupt */
  
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )          /* loop all channels  */
  {
    var[ch].secCount = 0;
    var[ch].msCount = 0;

    var[ch].state = TRANSIT;      /* default values - updated by switch     */
    var[ch].phase = 0;            /* this allows dc to transit back to dc0  */

    switch( CONFIG[adr][ch].mode )
    {
      case DAYNIGHT :             /* either input edge starts DAYNIGHT      */
        var[ch].state = DELAY;
        var[ch].phase = input;    /* phase 0 = 0 (day), phase 1 = 1 (night) */
        break;
        
      case DUSK :                 /* just day to night starts dawn          */
        if( input == NIGHT )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
        
      case DAWN :                 /* just night to day starts dawn          */
        if( input == DAY )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
        
      case DUSKDAWN :             /* either input edge starts dusk or dawn  */
        var[ch].state = DELAY;
        var[ch].phase = 1;
        break;
        
      case NIGHTONOFF :           /* just day to night starts NIGHTONOFF    */
        if( input == NIGHT )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
    }
  }                                                         /* end of loop  */
  interrupts();
}



/*-------------------------------- processChannels() -------------------------
 * 
 * timer1 Interrupt Service Routine runs every 1 milli second
 * 
 * cycle though the channels and process them
 * 
 * target DutyCycle, dc, is CONFIG[ch].dc[phase]      0-255
 * current dc is in var[ch].dc                        0-255
 * secs to delay is in CONFIG[ch].secDelay[phase]     0-255
 * Delay seconds counter is in var[ch].secCount       0-255
 * ms between inc/dec steps is in var[ch].msPerStep   0-1000
*/

void processChannels()          /*! This is an ISR which runs every 1 ms  !*/
{
  PORTC = PORTC | B01000000;    /* PIN ISRTIME = D31 aka PC6 = high.       */
                                /* pulse width high = ISR run time.        */
                                /* 16 MHz ATmega2560 min 21 us, peak 87 us */

  uint8_t countTransits = 0;    /* lights LED_builtin if Transits active   */

                                               /* loop all channels        */
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )
  {
    if( var[ch].state != STEADY )                  /* state not steady?    */
    {
      var[ch].msCount++;
      
      if( var[ch].state == DELAY )                 /* is state DELAY?      */
      {

        if( var[ch].msCount > 1000 )               /* count delay seconds  */
        {
          var[ch].msCount = 0;
          var[ch].secCount++;
          
          if( var[ch].secCount > CONFIG[adr][ch].secDelay[var[ch].phase] )
          {
            var[ch].state = TRANSIT;               /* end state DELAY      */
            var[ch].secCount = 0;
          }
        }
      }
      else                                         /* so, in state TRANSIT */
      {
        countTransits++;                           /* drives LED_builtin   */
        
        if( var[ch].msCount > var[ch].msPerStep )
        {
          var[ch].msCount = 0;
          if( var[ch].dc == CONFIG[adr][ch].dc[var[ch].phase] )
          { 
            var[ch].state = STEADY;                /* dc transit complete  */
            
            switch( CONFIG[adr][ch].mode )         /* trigger a new phase  */
            {
              case NIGHTONOFF :
                if( input == NIGHT )
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
            if( var[ch].dc > CONFIG[adr][ch].dc[var[ch].phase] )  /* which? */
            {
              var[ch].dc--;                              /* GT so decrement */
            }
            else
            {
              var[ch].dc++;                              /* LT so increment */
            }

            analogWrite( PWMPIN[ch], GAMMA8[var[ch].dc] );    /* set new dc */
            
          }  /* end GT or LT                 */
        }  /* end msCount > msPerStep        */
      }  /* end if Transit                   */
    }  /* end if state NOT steady            */
  }  /* next ch                              */

  digitalWrite( LED_BUILTIN, countTransits );     /* show TRANSIT activity */

  PORTC = PORTC & B10111111;           /* PIN ISRTIME = D31 aka PC6 = low  */
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

  static const char sInput[2][8] = {"0:Day", "1:Night"};
  
  bool rawInput = digitalRead( PININPUT );

  if( INPUTINVERT == true )
  {
    rawInput = ! rawInput;
  }
  
  if( ( rawInput != input ) && ( millis() > msStamp + 15 ) )
  {
    if( input == DAY )   /* invert */
      input = NIGHT; 
    else
      input = DAY;

    msStamp = millis();

    digitalWrite( PINLEDGRN, input );

    Serial.print("input ");
    Serial.println( sInput[input] );

    return true;
  }
  return false;
}



/*------------------------------ isLEDsupplyOK() -----------------------------
 * 
 * if blue LED is not lit, the 12 V line failed or PolyFuse tripped
*/

bool isLEDsupplyOK()
{ 
  if( digitalRead( PINBLUE ) == 0 )
  {
    Serial.println("PolyFuse/12V fail");
    
    return false;
  }
  return true;  
}



/*------------------------------- setDCmin() ----------------------------
 * In case of over Amps, set all channel duty cycle to emergency low
 * 
 * Simplified in V1.01 code change.
*/

void setDCmin( uint8_t setval )
{
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )
  {
    if( setval < var[ch].dc )
    {
      analogWrite( PWMPIN[ch], setval );
    }
  }
}



/*------------------------------- restoreDC() ----------------------------
 * restore running dc
 * 
 * New in V1.01 code change.
*/

void restoreDC()
{
  for( uint8_t ch = 0; ch < CHANSIZE; ch++ )
  {
    analogWrite( PWMPIN[ch], var[ch].dc );
  }
}



/*------------------------------ isOverAmp() ------------------------------
 * 
 * read Volts on sense resistor to measure Amps.  returns true if over Amp
 * 
 * V1.01 code change, simplified.
 * Schematic change: C3 sense RC filter. Change 0.1 uF to 1.0 uF. PCB OK.
*/

bool isOverAmp()
{
  const uint16_t SENSEAMP = 100 ;            /* 2 Amp ADC threshold      */

  amps = analogRead( PINSENSE );
  
  return ( amps > SENSEAMP );
}



/*----------------------------------- testPower() ----------------------------
 * 
 * If over Amp then alarm and reduce all duty cycles
 * 
 * Rewrite in V1.01 code change.
*/
void testPower()
{
  uint8_t emergencyDC = 8;
  
  if( isOverAmp() == true )
  {
    digitalWrite( PINLEDRED, 1 );
    
    Serial.print("OverAmp");
    printAmps();
    
    noInterrupts();                  /* stop ISR processing LED channels    */
    
    while( isOverAmp() == true )
    {
        soundAlarm( 1 );
        
        if( emergencyDC > 0 )
        {
          setDCmin( --emergencyDC );
        }
    }
    
    if( emergencyDC < 8 )
    {
      restoreDC();
    }
    
    interrupts();                    /* restart ISR processing LED channels */
  } 
  
  while( isLEDsupplyOK() == false)
  {
    noInterrupts();                  /* stop ISR processing LED channels    */
    
    digitalWrite( PINLEDRED, 1 );
    soundAlarm( 2 );
    
    interrupts();                    /* restart ISR processing LED channels */
  }
  
  digitalWrite( PINLEDRED, 0 );
}



/*------------------------------------------- setup() ------------------------
 * Arduino calls this on Reset or PowerUp
*/
void setup() 
{
  Serial.begin( BAUDRATE );
  while( ! Serial );
  
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
 
  delay( 200 );
  Serial.println( sTITLE );
  Serial.print("Build ");
  Serial.print(__DATE__);
  Serial.print(" CONFIG.h v");
  Serial.print( VERSION_CONFIG );
  Serial.print(" '");
  Serial.println( sCONFIGNOTE );

  readConfigAdr();

  setupChannels();

  soundAlarm( 1 );

  analogReference( INTERNAL1V1 );
  
  Timer1.initialize( 1000 );                    /* set timer to 1 ms        */
  Timer1.attachInterrupt( processChannels );

  Serial.println("send '.' for Amp, '0' for ch0 var .. '9' for ch9 var.");
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
