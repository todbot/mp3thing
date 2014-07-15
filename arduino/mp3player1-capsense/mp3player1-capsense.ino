/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

#include <CapacitiveSensor.h>

#define ledPin 13

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

const boolean debug = true;

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  //Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

const int capsenseThreshold1 = 30;
CapacitiveSensor   cs_4_2 = CapacitiveSensor(A0,A2);        // 10M resistor between pins 4 & 2, pi

int volStart = 0;
int vol;

//
void blink( int speed, int times ) 
{ 
  for( int i=0; i< times; i++ ) {
    digitalWrite( ledPin, HIGH );
    delay( speed/2 );
    digitalWrite( ledPin, LOW );
    delay( speed/2 );
  }
}

//
void blinkError( int speed )
{
  while( 1 ) { 
    blink( speed, 1 );
  }
}

uint32_t lastPressTime;
uint32_t lastReleaseTime;
boolean pressed;
boolean doublePress;
boolean newpress;

//
void touchMeasure()
{
  long total1 =  cs_4_2.capacitiveSensor(30);
  newpress = ( total1 > capsenseThreshold1 );
  doublePress = false;

  if( pressed && newpress ) {   // maybe double-click
    if( (millis() - lastPressTime ) <  500 ) { // double click
      Serial.println("!");
      doublePress = true;
    }
    lastPressTime = millis();
  }
  else if( pressed && !newpress ) { // just released
    lastReleaseTime = millis();
    pressed = false;
  }
  else if( !pressed && newpress ) { // just pressed
    lastPressTime = millis();
    lastReleaseTime = lastPressTime;
    pressed = true;
  }

  if(debug) {
    char dstr[80];
    int secs            = millis()/100;
    int secsLastPress   = lastPressTime/100;
    int secsLastRelease = lastReleaseTime/100;
    sprintf(dstr, "%d:\t%d %d %d", total1, newpress, pressed, doublePress);
    Serial.print(dstr);
    sprintf(dstr, "- %d %d %d\n", secs, secsLastPress, secsLastRelease);
    Serial.print(dstr);
  }
}
//
boolean touchPressed()
{
  return newpress;
}

//
boolean touchDoublePressed()
{
  //if( lastTouchTime  ) { 
  // }
}

//
void setup() 
{
  Serial.begin(9600);
  Serial.println("Capsense MP3 Test");

  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     blinkError( 50 );
  }
  Serial.println(F("VS1053 found"));

  //musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
 
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    blinkError( 300 );
  }
  Serial.println("SD OK!");
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume( volStart, volStart );

  /***** Two interrupt options! *******/ 
  // This option uses timer0, this means timer1 & t2 are not required
  // (so you can use 'em for Servos, etc) BUT millis() can lose time
  // since we're hitchhiking on top of the millis() tracker
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
  
  // This option uses a pin interrupt. No timers required! But DREQ
  // must be on an interrupt pin. For Uno/Duemilanove/Diecimilla
  // that's Digital #2 or #3
  // See http://arduino.cc/en/Reference/attachInterrupt for other pins
  // *** This method is preferred
  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial.println(F("DREQ pin is not an interrupt pin"));
}

//
void loop()
{
  // Alternately, we can just play an entire file at once
  // This doesn't happen in the background, instead, the entire
  // file is played and the program will continue when it's done!
  //  musicPlayer.playFullFile("track001.ogg");
  //musicPlayer.playFullFile("jaxx-rom.mp3");

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("track002.mp3")) {
    Serial.print("Could not open file");
    return;
  }
  Serial.println(F("Started playing"));

  while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    touchMeasure();
    if( touchPressed() ) { 
      vol = volStart;
    }
    vol++;
    //vol = (vol * 3) / 2;
    if( vol >= 200 ) vol = 200;

    musicPlayer.setVolume( vol, vol );
    delay(50);
  }
  Serial.println("Done playing music");
}


