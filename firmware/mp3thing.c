/**
 * mp3thing --
 *
 * 
 * Some info on the 2GB Sylvania MP3 player
 * - Charges via USB, powered USB hub will charge
 * - Running current: -16mA normal, -20mA peak when changing tracks
 * - Charging current: up to +110mA ? 
 * - 110mA NiMH battery
 * - Power switch directly connected to battery
 * - On power on
 * -- Takes ~5 sec to start playing
 * -- Starts playing first track from start
 * -- Starts at not max-volume (8 steps to max volume?)
 * - Approx 32 steps between min/max volume
 * - Plays tracks in the "Trash"
 * - 
 * 
 * Anonymous MicroSD Clip player
 * - Starts up within 2 seconds
 * - Resumes playing where it left off
 * - Can't seem to get it to act as MicroSD card reader
 * - Running current: -36-40mA (use at least 40 for calcs)
 * - Charge current: +120mA
 * - Runs on 3 x AAA NiMH batteries (1.2V, Rayovac hybrid)
 * -- run time: approx 20 hours continuous
 * -- effective mAh = 20 * 40 = 800 mAh
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>      
#include <stdint.h>

/*
// this is for normal BlinkM, using software PWM
#define PIN_RED  PB3
#define PIN_GRN  PB4
#define PIN_BLU  PB1

#define PIN_SDA  PB0
#define PIN_SCL  PB2

#define PIN_LED  PB1
*/

#define SWA PB3  // SW1
#define SWB PB4  // SW2
#define SWC PB1  // MISO
#define SWD PB0  // MOSI

#define SW_MASK (_BV(SWA) |_BV(SWB) |_BV(SWC) |_BV(SWD))

#define NC 0  // normally-closed (i.e. connected), connects when input is LOW
#define NO 1  // normally-open (i.e. disconnected), connects when input is HIGH

// swithch config for MAX4616 chip and "mp3thing_b1" board
#define SWA_SENSE NO
#define SWB_SENSE NC
#define SWC_SENSE NC  // Note: swapped with D
#define SWD_SENSE NO  // Note: swapped with C

uint32_t tick;

// compatibility layer
#if defined(__AVR_ATtiny13__)
#define WDIE WDTIE
#endif

// while sleeping, draws 7uA
void sleep_sec(uint8_t x) 
{
  while (x--) {
     // set the WDT to wake us up!
    WDTCR |= (1 << WDCE) | (1 << WDE); // enable watchdog & enable changing it
    WDTCR = (1<< WDE) | (1 <<WDP2) | (1 << WDP1);
    WDTCR |= (1<< WDIE);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    sleep_disable();
  }
}

//
SIGNAL(WDT_vect) 
{
  WDTCR |= (1 << WDCE) | (1 << WDE);
  WDTCR = 0;
}

// MAX46xx class chips can have normally-closed or normally-open switches.
// This function abstracts that away so you can set a switchid with a hi/lo val
void set_switch( uint8_t switchid, uint8_t val )
{
    uint8_t sense = NC; // 
    switch( switchid ) { 
    case SWA: sense = SWA_SENSE; break;
    case SWB: sense = SWB_SENSE; break;
    case SWC: sense = SWC_SENSE; break;
    case SWD: sense = SWD_SENSE; break;
    }
    if( sense == NC )  val = !val;  // invert if normally-closed type
    if( val ) 
        PORTB |=  _BV( switchid );  // now actually twiddle the pin
    else 
        PORTB &=~ _BV( switchid );
}

void set_switch_all( uint8_t val )
{
    set_switch( SWA, val );
    set_switch( SWB, val );
    set_switch( SWC, val );
    set_switch( SWD, val );
}

void do_polaroid_clip(void)
{

    DDRB |= SW_MASK;
    set_switch_all( 0 );

    sei();                      // enable interrupts

    //sleep_sec(2);
    _delay_ms(1000);
    for( uint8_t i=0; i<8; i++ ) { 
        set_switch( SWB, 1 );
        set_switch( SWD, 1 );
        _delay_ms(100);
        set_switch( SWB, 0 );
        set_switch( SWD, 0 );
        _delay_ms(100);
    }

    _delay_ms(100);

    set_switch( SWC, 1 );
    set_switch( SWD, 1 );
    _delay_ms(500);
    set_switch( SWD, 0 );
    
    for(;;) {
        set_switch( SWD, 1 );
        sleep_sec(1);
        set_switch( SWD, 0 );
        sleep_sec(1);
    }

}


/*
 *
 */
int main( void )
{
    WDTCR |= (1 << WDCE) | (1 << WDE);
    WDTCR = 0;

    do_polaroid_clip();
    
} 



// 
// Called on overflow (256 counts) of Timer0.
// (but I think not used here
//
ISR(SIG_OVERFLOW0)
{
    tick++;
}


/*
    // run forever
    for(;;) {
        //PORTB |=  PIN_SW_MASK;
        set_switch_all( 1 ); 
        sleep_sec(1);
        //_delay_ms(500);
        //PORTB &=~ PIN_SW_MASK;
        set_switch_all( 0 ); 
        sleep_sec(1);
        //_delay_ms(500);
    }
    */

