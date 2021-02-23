/*file: Lights_Control_1.ino
 *-------------------------------------------------------------------------
*/
const char sTITLE[] = "\nLights_Control_1 © Dave Harris 23-Feb-2021 v1.03";
/*
 *
 * Layout lighting control module for multiple LED strings.
 *
 * Author: Dave Harris. Andover, UK.  MERG #2740.
 * 
 * - 10x 12 Volt PWM channels driving LED strings. 
 *
 * - Input (Day/Night) triggers channel modes...
 *    DAYNIGHT :   Duty cycle (DC) transition on input change
 *    DAWN :       DC transitions on input changing to day
 *    DUSK :       DC transitions on input changing to night
 *    DUSKDAWN :   DC transitions on input change
 *    NIGHTONOFF : During night time, LEDs transition at durations
 *
 * - Transition config...
 *    Delay seconds before transitions.
 *    Duty cycle Transition duration in seconds.
 *
 * - Duty Cycle for Day and Night in config.
 *
 * - Over current protection. Audio alarm sounder.
 * - simple diagnostics provided on Serial Monitor. 
 * 
 * - logic signal input (opto isolated) as day/night trigger.
 * 
 * - Configurations are defined in seperate CONFIG.h file.
 *    This contains 4 configs and address jumpers select which is used.
 *    The configs are compiled and uploaded along with the sketch.
 *    Address jumpers are read at boot/reset time.
 * 
 * Possible future options... (: we will see! :)
 *  SPI bus on PCB to have input trigger via CBUS.
 *  LCD via I2C bus & rotary encoder for local configuration method.
 *  
 * 
 * TargetPCB : Lights_Control V1.1 revB (see KiCad files)
 * MCU Board : 'MEGA 2560 PRO (EMBED)' (this should be OK on 'Arduino MEGA')
 * Processor : ATmega2560
 * Framework : Arduino 1.8.13
 * FlashMem  : 8.6k of 254k (3%)
 * GlobalRAM : 1.2k of 8k   (13%)
 * 
 *
 *---------------------------- History ------------------------------------
 * 
 *  4-Jan-2021 Dave Harris (DH) Project started
 *  9-Jan-2021 DH, v0.01  breadboard first test
 * 23-Jan-2021 DH, v0.02  add channel Modes
 *  2-Feb-2021 DH, v0.03  add TimerOne lib and 1 ms ISR
 *  4-Feb-2021 DH, v1.00  release
 *  5-Feb-2021 DH, v1.01  correct over Amp code. Part change C3 0.1uF > 1.0uF
 * 11-Feb-2021 DH, v1.02  efficiency and tidy up changes
 * 23-Feb-2021 DH, v1.03  Bug fix, DC wrong after over Amp condition
 * 
 * 
 *
 *---------------------------- include files --------------------------------
*/
 

#include "DATADEF.h"      /* data structure definitions                     */

#include "CONFIG.h"       /* user editable configuration tables             */

#include "PIN.h"          /* module pin definitions                         */

#include "GAMMA8.h"       /* Gamma correction table                         */

#include <TimerOne.h>     /* timer1 library for 1 ms ISR foreground process */



/*
 *------------------------------ globals -----------------------------------
*/


uint8_t  adr;                 /* address of CONFIG used 0-3  Set by setup() */


Input_t  input = UNDEF;       /* input state 0 DAY or 1 NIGHT               */
  

volatile var_t var[CHANQTY];  /* channel variable array. Is modified by ISR */


uint16_t amps = 0;            /* ADC reading of PINSENSE     0-1023         */


extern const Config_t  CONFIG[CONFIGQTY][CHANQTY];       /*  See CONFIG.h   */


uint8_t stopISR = false;             /* disable 1 ms ISR process, if true   */


bool silenceAlarm = false;




/*
 *---------------------- Ammeter calibrate -------------------------------
 * 
 * Rsense = 0R050,   2.0 A = 0.100 V on PINSENSE, into ADC
 * ADC range 0-1023, Vref = 1.10 V.  ADC reads 0.001074 V per unit. 
 * Max reading is 0.10 / 0.001074 = 93
*/

const uint16_t MAXAMPADCREAD = 93;   /* 2.0 A  ADC threshold              */

const uint16_t AMPCALIBRATE = 22;    /* ADC multiplier to give true Amps  */



/*
 *--------------------------------- functions ----------------------------
*/


void soundAlarm();                  /* sound piezo buzzer for 0.25 second */

void printStartMsg();               /* ident & config data to Serial      */

void printChanConfig(uint8_t chan); /* print channel config               */

void printVar( uint8_t chan );      /* print channel vars                 */

void printAmps();                   /* print calibrated mA                */

uint8_t readConfigAdr();            /* read config address jumpers        */

bool isInputChanged();              /* get and debounce input             */

bool isUnderVolt();                 /* sense blue LED to see if 12V is ok */

bool isOverAmp();                   /* read Volts on sense resistor       */

void overrideDC(uint8_t setval);    /* set new DC on channels             */

void restoreDC();                   /* reset DCs on channels              */

void testPower();                   /* test over Amp & under Volts        */

void startNewPhase();               /* setup next phase for chans         */

void processChannels();             /* foreground process, every 1 ms ISR */

void setupChannels();               /* initial set up of channels         */

void powerOnTest();                 /* show indicators and sound alarm    */

void setPinModes();                 /* configure pins                     */

void checkSerialSend();             /* get and action any Serial commands */

void setup();                       /* power on / reset initialisation    */

void loop();                        /* background process                 */



/*
 *--------------------------- soundAlarm() -------------------------------
 * 
 * Sound Audio Warning Device (piezo buzzer ~4 kHz) for 0.25 second.
*/

void soundAlarm()
{
  checkSerialSend();
  
  if( silenceAlarm == false )
  {
    for( uint16_t i = 0; i < 1000; i++ )
    {
      digitalWrite( PINAWDSIG, HIGH );
      delayMicroseconds( 121 );
      digitalWrite( PINAWDSIG, LOW );
      delayMicroseconds( 121 );
    } 
  }
  else
  {
    delay( 250 ); 
  }
}



/*
 *-------------------------- printStartMsg() -------------------------------
 * 
 * setup() calls this
*/

void printStartMsg()
{
  Serial.print( sTITLE );
  Serial.print(F(" Build "));
  Serial.println( __DATE__ );
  Serial.print(F("CONFIG.h="));
  Serial.println( sCONFIG_V );
  Serial.print(F("INPUTINVERT="));
  Serial.println( INPUTINVERT );
  Serial.print(F("CONFIG adr="));
  Serial.println( adr );
}



/*
 *---------------------------------- printChanConfig() ---------------------
 * 
 * print one channel config to Serial
*/

void printChanConfig( uint8_t chan )
{
  char str[70];
  static const char fmt[] = 
   "ch%u Tran:%03us Dly0:%03us Dly1:%03us dc0:%03u dc1:%03u step:%05ums %s";
  
  sprintf( str, fmt
     , chan                                       /* chan number          */
     , CONFIG[adr][chan].secTransit               /* Transit seconds      */
     , CONFIG[adr][chan].secDelay[0]              /* Delay 0 secs         */
     , CONFIG[adr][chan].secDelay[1]              /* Delay 1 secs         */
     , CONFIG[adr][chan].dc[0]                    /* duty cycle DC0       */
     , CONFIG[adr][chan].dc[1]                    /* duty cycle DC1       */
     , var[chan].msPerStep                        /* derived config value */
     , sMode[CONFIG[adr][chan].mode]              /* string, mode of chan */
    );
  Serial.println( str );
}



/*
 *-------------------------------- printVar() ------------------------------
 * 
 * print channel vars to Serial
*/

void printVar( uint8_t chan )
{
  char str[60];
  static const char fmt[] = " ch%u dc:%03u phase:%u state:%s";

  sprintf( str, fmt,
    chan, var[chan].dc, var[chan].phase, sState[var[chan].state] );
  Serial.println( str );
}



/*
 *------------------------------------------ printAmps() -------------------
 * 
 * prints, approx, measured mA to Serial
 * displays as multiples of 22 mA   ## anything below that shows as zero ##
*/

void printAmps()
{
  char str[60];
  static const char fmt[] = " %umA";
  sprintf( str, fmt, ( amps * AMPCALIBRATE ) );  /* test against DVM Amps */
  
  Serial.println( str );
}



/*
 *----------------------------------- readConfigAdr() ---------------------
 * 
 * read the config address from the address jumpers
*/

uint8_t readConfigAdr()
{                       /*  2^0 + 2^1 x2  */
  return digitalRead( PINADR0 ) + ( digitalRead( PINADR1 ) << 1 );
}



/*
 *----------------------------- isInputChanged() --------------------------
 * 
 * read output of opto, debounce and set global var input
 * 
 * return true if input changed and false if unchanged
*/

bool isInputChanged()
{
  static uint32_t msStamp  = 0;             /* timestamp last input change */

  bool rawInput = digitalRead( PININPUT );

  if( INPUTINVERT == true )
  {
    rawInput = ! rawInput;
  }
  
  if( ( rawInput != input ) && ( millis() > msStamp + 15 ) )
  {
    switch( input )
    {
      case UNDEF:
        input = (Input_t) rawInput;
        break;
      case DAY:
        input = NIGHT;
        break;
      case NIGHT:
        input = DAY;
        break;
    }

    msStamp = millis();

    digitalWrite( PINLEDGRN, input );

    Serial.println( sInput[input] );

    return true;
  }
  else
  {
    return false;
  }
}



/*
 *------------------------------ isUnderVolt() ---------------------------
 * 
 * If blue LED is not lit, the 12 V line failed or PolyFuse tripped
 * 12 V feeds onto Blue LED via 10k, Vf ~ 2.8 V. 
 * ADC ref is 1.1 V. If blue < 1.075 V (1000) then underVolt condition.
 * 
 * return true if under Volt and false if 12 V is OK
*/

bool isUnderVolt()
{
  return( analogRead( PINBLUE ) < 1000 ); 
}



/*
 *------------------------------ isOverAmp() ----------------------------
 * 
 * read Volts on sense resistor to measure Amps.
 * 
 * V1.01 code change, simplified.
 * Schematic change: C3 sense RC filter. Change 0.1 uF to 1.0 uF. PCB OK.
 * 
 * return true if over Amp and false if Amps OK
*/

bool isOverAmp()
{
  amps = analogRead( PINSENSE );
  
  return ( amps > MAXAMPADCREAD );
}



/*
 *----------------------------- overrideDC() ---------------------------
 * 
 * In case of over Amps, set channel duty cycle to lower value
 * 
 * Simplified in v1.01 and v1.03 code change.
*/

void overrideDC( uint8_t shiftR )
{
  
  for( uint8_t ch = 0; ch < CHANQTY; ch++ )
  {
    uint8_t newDC = var[ch].dc >> shiftR;     /* divide DC down       */
    
    analogWrite( PWMPIN[ch], GAMMA8[newDC] );
    
    Serial.print(" ");
    Serial.print( ch );
    Serial.print("=");
    Serial.print( newDC );
  }
  Serial.print(" >>");
  Serial.println( shiftR );
}



/*
 *------------------------------- restoreDC() ----------------------------
 * 
 * restore running dc
 * 
 * New in V1.01 code change. v1.03 add GAMMA lookup.
*/

void restoreDC()
{
  for( uint8_t ch = 0; ch < CHANQTY; ch++ )
  {
    analogWrite( PWMPIN[ch], GAMMA8[var[ch].dc] );
  }
}



/*
 *----------------------------------- testPower() --------------------------
 * 
 * If over current then reduce duty cycles and alarm.
 * If under Volt (poly fuse?) then just alarm.
 * Rewrite in V1.01 and v1.03 code change.
*/
void testPower()
{
  uint8_t shiftR = 1;                /* shiftR  1 is divide by 2          */
  
  while( isOverAmp() == true )       /* are we over Amps?                 */
  {
    stopISR = true;                  /* stop ISR as this code overides DC */
    
    digitalWrite( PINLEDRED, HIGH );
    Serial.print(F("OverAmp"));
    printAmps();

    if( shiftR < 8 )
    {
      overrideDC( shiftR++ );           /* cut DC by half                 */          
    }
    soundAlarm();                       /* constant beep beep beep beep   */
  }

  if( shiftR > 1 )
  {
    Serial.println(F("recover"));
    restoreDC();
  }
  
  stopISR = false;                 /* restart ISR processing LED channels */


  while( isUnderVolt() == true )
  {
    digitalWrite( PINLEDRED, HIGH );
    Serial.println(F("12Vtrip"));

    soundAlarm();                   /* beep pause beep pause beep...     */
    delay( 1000 );
  }

  digitalWrite( PINLEDRED, LOW );
}



/*
 *------------------------------ startNewPhase() --------------------------
 * 
 * The day/night input has changed, so setup new phase for each channel 
*/

void startNewPhase()
{
  stopISR = true;         /* all variables here are also manipulated by ISR */
  
  for( uint8_t ch = 0; ch < CHANQTY; ch++ )          /*  loop all channels  */
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
        
      case DUSK :                 /* day to night starts dusk               */
        if( input == NIGHT )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
        
      case DAWN :                 /* night to day starts dawn               */
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
        
      case NIGHTONOFF :           /* day to night starts NightOnNOff        */
        if( input == NIGHT )
        {
          var[ch].state = DELAY;
          var[ch].phase = 1;
        }
        break;
    }
  }                              /* end of loop                             */
  stopISR = false;
}



/*
 *-------------------------- processChannels() -----------------------------
 * 
 * timer1 Interrupt Service Routine runs every 1 milli second
 * 
 * Cycle though the channels and process them
 * 
 * target DutyCycle, dc, is CONFIG[ch].dc[phase]      0-255
 * current dc is in var[ch].dc                        0-255
 * ms counter is in var[ch].msCount (step & Delay)    0-65k
 * Delay seconds counter is in var[ch].secCount       0-255
 * secs to delay is in CONFIG[ch].secDelay[phase]     0-255
 * ms between inc/dec steps is in var[ch].msPerStep   0-65k
*/

void processChannels()         /* ! This is an ISR which runs every 1 ms  ! */
{  
  SetPINTP_D31;                /* Scope TP on pin D31 ~= ISR run time       */
                               /* on ATmega2560 = min 21 us to peak 87 us   */

  bool transits = false;       /* lights LED_builtin if Transits are active */


  if( stopISR == false )                  /* allow this to change DC values */
  {
                                                   /* loop all channels     */
    for( uint8_t ch = 0; ch < CHANQTY; ch++ )
    {
  
      if( var[ch].state != STEADY )                /* is state not STEADY?  */
      {
        var[ch].msCount++;
        
        if( var[ch].state == DELAY )                 /* is state DELAY?     */
        {
  
          if( var[ch].msCount > 1000 )               /* count delay seconds */
          {
            var[ch].msCount = 0;
            var[ch].secCount++;
            
            if( var[ch].secCount > CONFIG[adr][ch].secDelay[var[ch].phase] )
            {
              var[ch].state = TRANSIT;               /* end state DELAY     */
              var[ch].secCount = 0;
            }
          }
        }
        else                                  /* we deduce state is TRANSIT */
        {
          transits = true;                           /* drives LED_builtin  */
          
          if( var[ch].msCount > var[ch].msPerStep )
          {
            var[ch].msCount = 0;
            if( var[ch].dc == CONFIG[adr][ch].dc[var[ch].phase] )
            { 
              var[ch].state = STEADY;                /* dc transit complete */
              
              switch( CONFIG[adr][ch].mode )         /* trigger a new phase */
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
            else            /* in Transit & current DC is GT or LT target DC */
            {
              if( var[ch].dc > CONFIG[adr][ch].dc[var[ch].phase] ) /* which? */
              {
                var[ch].dc--;                             /* GT so decrement */
              }
              else
              {
                var[ch].dc++;                             /* LT so increment */
              }
              analogWrite( PWMPIN[ch], GAMMA8[var[ch].dc] );   /* set new dc */
            }
          }
        }
      }  /* end if state not STEADY   */
    }  /* end for ch loop             */
  
    digitalWrite( LED_BUILTIN, transits );          /* show TRANSIT activity */
    
  } /* end not stopISR   */
  
  ClrPINTP_D31;   /* pin high, then low = ISR run time */
}



/*
 *---------------------- setupChannels() -----------------------------------
 * 
 * preset channel variables derived from channel constants and start the PWM
 *  
 * called from setup(), before timerOne is started
*/

void setupChannels()
{
  for( uint8_t ch = 0; ch < CHANQTY; ch++ )     /* loop through channels   */
  {
    if( CONFIG[adr][ch].secTransit == 0 )    /* zero sec transit is special */
    { 
      var[ch].msPerStep = 1;    /* dc transits rapidly to target duty cycle */
    }
    else                        /* non-zero transit secs, calc ms per step  */
    {
      uint16_t diff = abs( CONFIG[adr][ch].dc[0] - CONFIG[adr][ch].dc[1] );
      
      uint32_t msCalc = ( CONFIG[adr][ch].secTransit * 1000UL ) / diff;
      
      if( msCalc > MAXmsPERSTEP )    /* long Transit & tiny diff overflows  */
      {                              /* when abs(DC0 - DC1) is less than 4. */
        msCalc = MAXmsPERSTEP;       /* Limit step transit ms.              */
      }
      var[ch].msPerStep = (uint16_t) msCalc;
    }
    
    var[ch].dc = MAXDC / 2;             /* or... CONFIG[adr][ch].dc[input]; */

    analogWrite( PWMPIN[ch], var[ch].dc );        /* set DC of PWM channel  */
    
    printChanConfig( ch );
  } 
  delay( 500 );
  
  startNewPhase();
}



/*
 *------------------------------- powerOnTest() ----------------------------
 * 
 * setup() calls this
*/

void powerOnTest()
{
  for( uint8_t ch = 0; ch < CHANQTY; ch++ )
  {
    analogWrite( PWMPIN[ch], 5 );     /* start PWM channel at DC 5/255 = 2% */
  }
  
  pinMode( PINTP_D30, OUTPUT );       /* Test Point: TBA                    */
  SetPINTP_D30;                       /* macro fast pin method. See PIN.h   */
  ClrPINTP_D30;
  
  pinMode( PINTP_D31, OUTPUT );       /* Test Point: measure ISR duration   */
  SetPINTP_D31;
  ClrPINTP_D31;                       /* macro fast pin method. See PIN.h   */
  
  digitalWrite( PINLEDRED, 1 );       /* light LEDs & AWD sound test        */
  digitalWrite( PINLEDYEL, 1 );       /*                                    */
  digitalWrite( PINLEDGRN, 1 );       /*                                    */
  digitalWrite( LED_BUILTIN, 1 );     /* all LED flash ...                  */
  soundAlarm();                       /* test AWD, short beeep              */
  digitalWrite( PINLEDRED, 0 );       /* end power on tests, reset all LED  */
  digitalWrite( PINLEDYEL, 0);        /*                                    */ 
  digitalWrite( PINLEDGRN, 0 );       /*                                    */
  digitalWrite( LED_BUILTIN, 0 );     /*                                    */  
}



/*
 *--------------------------- setPinModes() --------------------------------
 * 
 * setup() calls this
*/

void setPinModes()
{
  pinMode( PININPUT,  INPUT_PULLUP ); /* Day or Night via opto isolate      */
  pinMode( PINADR0,   INPUT_PULLUP ); /* Config Address bit 2^0 jumper link */
  pinMode( PINADR1,   INPUT_PULLUP ); /* Config Address bit 2^1 jumper link */
  pinMode( PINTACTSW, INPUT_PULLUP ); /* Tactile push switch - TBA          */
  
  pinMode( PINBLUE,   INPUT );        /* 12 V sensed on blue LED into ADC   */
  pinMode( PINENCPHA, INPUT );        /* Rotary encoder phase A via lowpass */
  pinMode( PINENCPHB, INPUT );        /* Rotary encoder phase B via lowpass */
  pinMode( PINENCSW,  INPUT );        /* Rotary encoder switch via lowpass  */
  pinMode( PINSENSE,  INPUT );        /* input yes, but is ADC              */
  
  pinMode( PINAWDSIG, OUTPUT );       /* AudioWarningDevice: Piezo buzzer   */
  pinMode( PINLEDRED, OUTPUT );       /* Red LED: over Amp/under Volt alarm */
  pinMode( PINLEDYEL, OUTPUT );       /* Yellow LED:  TBA                   */
  pinMode( PINLEDGRN, OUTPUT );       /* Green LED: input state LED         */
  pinMode( LED_BUILTIN, OUTPUT );     /* Red LED on MEGA, DC Transit active */
}



/*
 *-------------------------------- setup() ---------------------------------
 * 
 * Arduino calls this on Reset or PowerUp
*/

void setup() 
{
  Serial.begin( BAUDRATE );
  while( ! Serial );

  setPinModes();
  
  powerOnTest();
  
  adr = readConfigAdr();

  printStartMsg();

  isInputChanged();
  
  setupChannels();

  analogReference( INTERNAL1V1 );   /* most sensitive on MEGA, 1.075 mV/unit */
  
  Timer1.initialize( 1000 );                   /* ISR to run @ 1000 us, 1 ms */
  Timer1.attachInterrupt( processChannels );   /* Foreground process         */

  Serial.println(F("Debug, send . =mA, 0..9 =ch0..9 var, +/- =AWD silence"));
}



/*
 *------------------------ checkSerialSend() --------------------------------
 * 
 * check and action serial input
 * Debug, send . =mA, 0..9 =ch0..9 var, +/- =AWD silence
*/

void checkSerialSend()
{
  if (Serial.available() > 0) 
  {
    char rx = Serial.read();
    
    if( rx == '.' ) printAmps();                /* print chan vars    */
    if( rx == '-' ) silenceAlarm = true;        /* silence            */
    if( rx == '+' ) silenceAlarm = false;       /* no silence         */
    
    if( rx >= '0' && rx <= '9' )                /* 0 to 9 ..          */ 
    {
      printVar( rx - '0' );                     /* print chan vars    */
    }
  }  
}



/*
 *----------------------------------- loop() -------------------------------
 * 
 * Background process loop.
*/

void loop()
{   
  testPower();
  
  if( isInputChanged() == true )
  {
    startNewPhase();
  }

  checkSerialSend();

} /* restart loop() */


/*
 *--------- Lights_Control_1.ino -------------------- EoF ------------------
*/
