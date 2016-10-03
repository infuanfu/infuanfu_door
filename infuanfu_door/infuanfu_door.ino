#include <SPI.h>
#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"

String const myUID = "70 4D 7B 28"; // replace this UID with your NFC tag's UID
int const greenLedPin = 3; // green led used for correct key notification
int const redLedPin = 4; // red led used for incorrect key notification

PN532_SPI interface(SPI, 10); // create a SPI interface for the shield with the SPI CS terminal at digital pin 10
NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object
 
void setup(void) {
    Serial.begin(115200); // start serial comm
    Serial.println("NDEF Reader");
    nfc.begin(); // begin NFC comm
    
    // make LED pins outputs
    pinMode(greenLedPin,OUTPUT);
    pinMode(redLedPin,OUTPUT);
    
    // turn off the LEDs
    digitalWrite(greenLedPin,LOW);
    digitalWrite(redLedPin,LOW);
}
 
void loop(void) {
  
    Serial.println("Scanning...");
    if (nfc.tagPresent()) // check if an NFC tag is present on the antenna area
    {
        NfcTag tag = nfc.read(); // read the NFC tag
        String scannedUID = tag.getUidString(); // get the NFC tag's UID

        Serial.print("ID: ");
        Serial.println(scannedUID);
        
        if( myUID.compareTo(scannedUID) == 0) // compare the NFC tag's UID with the correct tag's UID (a match exists when compareTo returns 0)
        {
          // The correct NFC tag was used
          Serial.println("Correct Key");
          // Blink the green LED and make sure the RED led is off
          digitalWrite(greenLedPin,HIGH);
          digitalWrite(redLedPin,LOW);
          
          delay(1500);
          digitalWrite(greenLedPin,LOW);
          // put your here to trigger the unlocking mechanism (e.g. motor, transducer)
        }else{
          // an incorrect NFC tag was used
          Serial.println("Incorrect key");
          // blink the red LED and make sure the green LED is off
          digitalWrite(greenLedPin,LOW);
          digitalWrite(redLedPin,HIGH);
          
          delay(500);
          digitalWrite(redLedPin,LOW);
          delay(500);
          digitalWrite(redLedPin,HIGH);
          delay(500);
          digitalWrite(redLedPin,LOW);
          // DO NOT UNLOCK! an incorrect NFC tag was used. 
          // put your code here to trigger an alarm (e.g. buzzard, speaker) or do something else
        }
    }
    delay(2000);
}
