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

#define MP3_TRACKNAME  "TRACK01.MP3"

const boolean IS_METAL_BOX = true;

const boolean debug = true;

#define ledPin 13
#define buttonPin A0

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

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

boolean buttonPressed()
{
    return (!digitalRead( buttonPin));
}

//
void setup() 
{
  Serial.begin(9600);
  Serial.println("mp3player1_button");

  pinMode( buttonPin, INPUT_PULLUP );

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
  if (! musicPlayer.startPlayingFile( MP3_TRACKNAME )) {
    Serial.print("Could not open file");
    return;
  }
  Serial.println(F("Started playing"));

  while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)

    if( buttonPressed() ) { 
      Serial.println("pressed!");
      vol = volStart;
    }

    // decay volume (higher is quieter)
    if( IS_METAL_BOX ) {
      vol += 3;
    } else { // wooden box
      vol++;
    }
    //vol = (vol * 3) / 2;

    if( vol >= 200 ) {
      vol = 200;
    }

    musicPlayer.setVolume( vol, vol );
    delay(50);
  }
  Serial.println("Done playing music");
}


