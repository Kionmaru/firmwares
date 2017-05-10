/*       
 *  RFIDreaderrr authors:
 *    John-Michael Denton
 *  
 *        
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <SoftwareSerial.h>
#include <string.h>
// Where RFID(txpin, rxpin)
// rxpin not required to be connected.
SoftwareSerial RFID(2, 3);
// Let's be fancy and use an LCD!
#include <LiquidCrystal.h>
// Since we're already using pin 2; we shift it over 1.
// Using an Hitachi HD44780 compatible LCD screen
// LCD K -> Vdd
// LCD A -> 220ohm -> Vss
// LCD D7 -> pin 3
// LCD D6 -> pin 4
// LCD D5 -> pin 5
// LCD D4 -> pin 6
// LCD D[3:0] disconnected
// LCD E -> pin 11
// LCD RS -> pin 12
// LCD RW -> ground
// LCD V0 -> 220ohm -> Vdd
// LCD Vdd -> Vdd
// LCD Vss -> Vss

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);
// Being able to idle would be nice
#include <avr/sleep.h>

const int taglength = 14;

struct keyid {
  int tag[taglength];
  char id[12];
};

// keyfob reference numbers not encoded in RFID datastream.
struct keyid mykeys[5] = {
  {{0x02, 0x30, 0x39, 0x30, 0x30, 0x36, 0x31, 0x36, 0x43, 0x39, 0x44, 0x39, 0x39, 0x03}, "0006384797"},
  {{0x02, 0x30, 0x45, 0x30, 0x30, 0x37, 0x41, 0x33, 0x44, 0x46, 0x35, 0x42, 0x43, 0x03}, "0008011253"},
  {{0x02, 0x30, 0x35, 0x30, 0x30, 0x38, 0x44, 0x46, 0x33, 0x41, 0x46, 0x44, 0x34, 0x03}, "0009302959"},
  {{0x02, 0x30, 0x35, 0x30, 0x30, 0x38, 0x45, 0x38, 0x32, 0x35, 0x41, 0x35, 0x33, 0x03}, "0009339482"},
  {{0x02, 0x30, 0x45, 0x30, 0x30, 0x37, 0x41, 0x34, 0x38, 0x43, 0x42, 0x46, 0x37, 0x03}, "0008014027"}
};

void setup() {
  // put your setup code here, to run once:
  RFID.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Reticulating");
  lcd.setCursor(0,1);
  lcd.print("splines...");
  delay(3000);
  lcd.clear();
  lcd.print("Spinning up");
  lcd.setCursor(0, 1);
  lcd.print("hamster...");
  delay(3000);
  lcd.clear();
  lcd.print("Boot sequence");
  lcd.setCursor(0, 1);
  lcd.print("complete.");
  delay(3000);
  lcd.clear();
  set_sleep_mode(SLEEP_MODE_IDLE);
}

void loop() {
  // put your main code here, to run repeatedly:
  static int tag[taglength] = {0};
  static int lasttag[taglength] = {0};
  static unsigned long cleartime;
  static uint8_t cleared = 0;
  /* The ID in the RFID block is not what's printed on the card.
  */
  while (RFID.available() < taglength)
  {
    if(millis() > cleartime && !cleared)
    {
      lcd.clear();
      cleared = 1;
    }
    sleep_enable();
    // It may look like there's nothing to trigger a wake up, but arduino millis() 
    // is an ISR vect run off a timer - so the unit will wake up at least once per millisecond.
    sleep_mode();
    sleep_disable();
  }

  for (int j = 0; j < taglength; j++)
  {
    tag[j] = RFID.read();
  }
  if (tag[0] == 0x02 && tag[13] == 0x03)
  { // First and last values as expected.
    for (int i = 0; i < 5; i++)
    {
      if (! memcmp(mykeys[i].tag, tag, taglength) && memcmp(lasttag, tag, taglength))
      {
        cleared = 0;
        Serial.print("Key found: ");
        Serial.print(mykeys[i].id);
        lcd.clear();
        lcd.print("Key: ");
        lcd.print(mykeys[i].id);
        lcd.setCursor(0, 1);
        Serial.print(", internal ID: ");
        lcd.print("ID: ");
        // Byte 1 is always 0x02, the last byte is always 0x03, and the two bytes
        // before 0x03 are XOR checksum bytes we're ignoring.
        for (int j = 1; j < 11; j++)
        {
          Serial.print((char) tag[j]);
          lcd.print((char) tag[j]);
        }
        Serial.print("\n");
        memcpy(lasttag, tag, 14);
        // Yes, this can overflow - but so does the value returned by millis(),
        // so we avoid failing to clear sometime on the 50th-ish day for power-on...
        cleartime = millis() + 10000;
      }
    }
  }
}
