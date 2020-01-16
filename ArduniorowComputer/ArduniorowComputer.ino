//-------------------------------------------------------------------
// Steve Aitken - 2015
/* Arduino row Computer
 *  This is the code to integrate with an Arduino Uno
 * Uses an UNO + LCD keypad shield like this: http://www.lightinthebox.com/16-x-2-lcd-keypad-shield-for-arduino-uno-mega-duemilanove_p340888.html?currency=GBP&litb_from=paid_adwords_shopping
 * principles behind calculations are here : http://www.atm.ox.ac.uk/rowing/physics/ergometer.html#section7
 * 13% memory used for LCD / menu
 * 41% total
 */
#include <avr/sleep.h>

#include "mainEngine.h"   
#define debug  // uncomment this to get more verbose serial output

//-------------------------------------------------------------------
//               pins
const byte switchPin = 2;                    // switch is connected to pin 2
const byte analogPin = 1;                    // analog pin (Concept2)
//-------------------------------------------------------------------
//               reed (switch) handling
int val;                                    // variable for reading the pin status
int buttonState;                            // variable to hold the button state

void setup() 
{
  pinMode(switchPin, INPUT_PULLUP);        // Set the switch pin as input
  Serial.begin(115200);                    // Set up serial communication at 115200bps
  buttonState = digitalRead(switchPin);    // read the initial state
  detectMachine();
   // set up the LCD's number of columns and rows: 
  #ifdef UseLCD
    lcdSetup();
    //register graphics for up/down
    graphics();
    startMenu();
  #endif
  Serial.println(F("Stroke\tSPM\tSplit\tWatts\tDistance\tTime\tDragFactor"));
}

//quickly figure out if a rotation has happened.
void loop()
{
  mTime = millis();
  uTime = micros(); 
  processSerial();
  if(analogSwitch)
  {
    doAnalogRead();
  }
  else
  {
    val = digitalRead(switchPin);            // read input value and store it in val                       
  }
   if (val != buttonState && val == LOW && (uTime- lastStateChangeus) >5000)            // the button state has changed!
    { 
      registerClick();
         #ifdef UseLCD
            writeNextScreen();
         #endif
      lastStateChangeus=uTime;
    }
    if((millis()-mTime) >=18)
    {
      Serial.print(F("warning - loop took (ms):"));
      Serial.println(millis()-mTime);
    }
  buttonState = val;                       // save the new state in our variable
}

//write the details of a stroke to serial
void writeStrokeRow()
{
  char tabchar = '\t';
  Serial.print(totalStroke); Serial.print(tabchar);
  Serial.print(spm); Serial.print(tabchar);
  Serial.print(getSplitString()); Serial.print(tabchar);
  Serial.print(power); Serial.print(tabchar);
  Serial.print(distancem); Serial.print(tabchar);
  Serial.print(getTime()); Serial.print(tabchar);
  Serial.print(k*1000000); 
  Serial.println();
}


