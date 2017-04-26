#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Define the data pin for your neopixels:
#define PIN 9
// How many neopixels, including the one on the Flora, are there?
#define NEOPIXEL_COUNT 3
// Define Pi.
#define PI 3.141592653589793

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT - 1, PIN, NEO_GRB + NEO_KHZ800);

// This is the on-Flora NeoPixel, so it's left alone.
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);
// We have a global modified by interrupts
volatile uint16_t adc_result = 512;
volatile uint8_t adc_counter = 0;
// Classic scratch variable
uint8_t temp;
    
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip2.begin();
  strip2.show(); // Set the onboard neopixel to 'off' too.

  // We handle the Analog <-> digital conversions directly.
  // See the Atmel ATMega32U4 data sheet for more information.
  // We need to enable ADC9 - which means we set bits MUX5 and 
  // MUX0.
  ADMUX |= (1 << MUX0);
  ADCSRB |= (1 << MUX5);
  // Turn on the ADC.
  ADCSRA |= (1 << ADEN);
  // Enable ADC auto trigger enable.
  ADCSRA |= (1 << ADATE);
  // We want to generate an interrupt when a read is complete, 
  // we enable that.
  // How did I type this ACSR |= (1 << ACIE);
  ADCSRA |= (1 << ADIE);
  // Request conversion start.
  ADCSRA |= (1 << ADSC);
  // Enable global interrupts
  sei();
  }

//Handle ADC interrupt when conversion is complete.
ISR(ADC_vect)
{
  // The 10-bit results of the ADC are stored in two registers.
  // Grab the MSB here, and shift them 8 spaces left. Then grab
  // the other register.
  // Once ADCL is read, ADCH is locked and ADC readings can't complete.
  // This ensures we read them in the right order, instead of depending
  // on how the compiler interprets this...
  temp = ADCL;
  adc_result = ADCH << 8 | temp;
  // Start the next conversion.
  ADCSRA |= (1 << ADSC);
  adc_counter++;
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t adr;
  int counter = 0;
  uint32_t wheelcolor;
  static uint8_t position[NEOPIXEL_COUNT];
  for(counter = 0; counter < NEOPIXEL_COUNT; counter++)
  {
    position[counter] = counter * (255 / NEOPIXEL_COUNT);
  }

    adr = adc_result / 4;
    wheelcolor = Wheel(adr);
    
    
    strip2.setPixelColor(0, wheelcolor);
    for(counter = 0; counter < NEOPIXEL_COUNT - 1; counter++)
      strip.setPixelColor(counter, wheelcolor);
    //for(counter = 0; counter < NEOPIXEL_COUNT - 1; counter++)
    //  position[counter]++;
    if(adc_counter > 200)
    {
      position[0]++;
      adc_counter = 0;
    }
    strip.show();
    strip2.show();

    // Sleep until the ADC read is done.
    cli();
    interrupts();
    sleep_enable();
    // The first command after sei is always run.
    asm(
      "sei\n\t"
      "sleep\n\t"
      );
    sleep_disable();

}
