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
}

void loop() {
  // put your main code here, to run repeatedly:
  static int tag[taglength] = {0};
  static int lasttag[taglength] = {0};
  /* The ID in the RFID block is not what's printed on the card.
  */
  while (RFID.available() < taglength);

  for (int j = 0; j < taglength; j++)
  {
    tag[j] = RFID.read();
  }
  //RFID.flush();
  if (tag[0] == 0x02 && tag[13] == 0x03)
  { // First and last values as expected.
    for (int i = 0; i < 5; i++)
    {
      if (! memcmp(mykeys[i].tag, tag, taglength) && memcmp(lasttag, tag, taglength))
      {
        Serial.print("Key found: ");
        Serial.print(mykeys[i].id);
        Serial.print(", internal ID: ");
        // Byte 1 is always 0x02, the last byte is always 0x03, and the two bytes
        // before 0x03 are XOR checksum bytes we're ignoring.
        for (int j = 1; j < 11; j++)
        {
          Serial.print((char) tag[j]);
        }
        Serial.print("\n");
        memcpy(lasttag, tag, 14);
      }
    }
  }
}
