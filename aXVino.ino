// Copyright (C) Anthony Roberts, 2012
// 6 April 2012
// Arduino Source for the aXVino iPhone App
// Designed to work with the Roving Networks RN-XV WiFly Module
// This version does NOT implement the Time functions. See the aXVino_with_Time if required

#include <SoftwareSerial.h>

/*
WiFly Pin  1 (POWER) to 3.3v on Arduino (or other 3.3v Power Supply)
WiFly Pin  2 (TX)    to Arduino Pin 2 (SoftSerial RX)
WiFly Pin  3 (RX)    to Arduino Pin 3 (SoftSerial TX)
WiFly Pin  8 (Adhoc) to 3.3v to put unit into Adhoc mode at power up
WiFly Pin 10 (GND)   to Ground

Useful (or maybe just interesting) Commands
scan                    View a list of visible WiFi networks
get ip                  Show IP details such as leased IP address
lites                   Flash the LEDs on the WiFly (use same command again to turn off)
ping 8.8.8.8 5          Ping 8.8.8.8 5 times
lookup www.apple.com    Use DNS to resolve www.apple.com
show net
*/

#define ARDUINO_RX 2    // Software Serial RX Pin (on Arduino)
#define ARDUINO_TX 3    // Software Serial TX Pin (on Arduino)
#define DIGITAL 0       // Used in the ledType array
#define ANALOG 1        // Used in the ledType array
#define TMP36 A0        // Analog input pin for TMP36 temperature sensor

#define ACCESS_ALLOW 5705    // Security Access Number. Are you old enough to remember this song?

// LED's are numbered 1 to 9, but array elements start at 0. To make things simple, I don't use element 0

// Put a Zero for unused pins (or any that are assigned to another function). Default is use all nine
int ledPins[10]  = {0,4,5,6,7,8,9,10,11,12};   

// Keep track of which LED's are On (HIGH) or Off (LOW). Default at the start is all off
int ledState[10] = {0, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// Make a note of which LED's are DIGITAL (On / Off only) and ANALOG (allow PWM for variable brightness)
int ledType[10]  = {0, DIGITAL, ANALOG, ANALOG, DIGITAL, DIGITAL, ANALOG, ANALOG, ANALOG, DIGITAL};

// Use the SoftwareSerial library so that Pins 0 and 1 don't need disconnecting to program Arduino
// The RN-XV is connected to these pins, so any traffic will be from the Wifly module
SoftwareSerial WiFly(ARDUINO_RX, ARDUINO_TX);

void setup()
{
// Initialise all the LED's that we're using so they're ready for Output
  for (int i = 1; i <= 9; i++) {
    if (ledPins[i] > 0) {                     // If zero, then we're not using that Pin
      pinMode(ledPins[i], OUTPUT);            // Set the Pin to Output (so we can write to it)
      digitalWrite(ledPins[i], ledState[i]);  // Set the default state (On or Off)
    }
  }
    
// Make sure all the LED's are working (only happens at power up). Confidence test
  for (int i = 1; i <= 9; i++) {
    if (ledPins[i] > 0) {
      digitalWrite(ledPins[i], HIGH);
    }
  }
// All the LED's (that we're using) should now be on. Wait for 1 second
  delay(1000);\
// Turn all the LED's (that we're using) Off
  for (int i = 1; i <= 9; i++) {
    if (ledPins[i] > 0) {
      digitalWrite(ledPins[i], LOW);
    }
  }
// Wait another second
  delay(1000);

// Set the LED's to their default state  
  for (int i = 1; i <= 9; i++) {
    if (ledPins[i] > 0) {
      digitalWrite(ledPins[i], ledState[i]);
    }
  }  
}



void loop()
{
  char instruc, param1, param2, param3, param4;
  int whichPin, brightness, accessCode;

// Any data from the RN-XV waiting to be processed?
  if (WiFly.available()) {
    
// The peek function has a look at the data in the buffer (but doesn't actually read it)
    if (WiFly.peek() == '!') {  // An instruction is on its way
      delay(100);               // Allow time for the rest of the instruction to arrive
      instruc = WiFly.read();   // eg. H for Set High on LED
      param1 = WiFly.read();    // First parameter - usually the LED number
      param2 = WiFly.read();    // Second parameter
      param3 = WiFly.read();    // Third parameter
      param4 = WiFly.read();    // Fourth parameter
      
// We've read the instruction, now let's see which one it was and act accordingly      
      switch (instruc) {
        case '?':                   // Get the current state of the LED's
          WiFly.print("STATE:");
          for (int i = 1; i <= 9; i++) {
            if (ledState[i] == LOW) WiFly.print("0"); else WiFly.print("1");
          }
          WiFly.println("");
          break;

        case 'H':                   // H (for High) is to turn on an LED
          whichPin = param1 - 48;   // 0 is Ascii 48, so subtract 48 to convert to an actual number
          if (ledPins[whichPin] > 0) {               // Make sure it's a valid LED
            digitalWrite(ledPins[whichPin], HIGH);   // Turn the LED on
            ledState[whichPin] = HIGH;               // Make a note of the state
          }
          break;
        
        case 'L':                   // L (for Low) is to turn off an LED
          whichPin = param1 - 48;   // 0 is Ascii 48, so subtract 48 to convert to an actual number
          if (ledPins[whichPin] > 0) {            // Make sure it's a valid LED
            digitalWrite(ledPins[whichPin], LOW); // Turn the LED off
            ledState[whichPin] = LOW;             // Make a note of the state
          }
          break;
        
        case 'A':                        // A for Analog. Set an LED brightness (0 to 255)
          whichPin = param1 - 48;        // 0 is Ascii 48, so subtract 48 to convert to an actual number
// The value is passed as three ascii characters. So subtract 48 to get number and multiple accordingly          
          brightness = ((param2 - 48) * 100) + ((param3 - 48) * 10) + (param4 - 48);
          if (ledPins[whichPin] > 0 && ledType[whichPin] == ANALOG) {   // Make sure we can do this
            analogWrite(ledPins[whichPin], brightness);                 // Set the LED brightness
            if (brightness == 0) ledState[whichPin] = LOW; else ledState[whichPin] = HIGH;
          }
          break;
        
        case 'S':    // The Security function checks the 4 digit number passed
          accessCode = ((param1 - 48) * 1000) + ((param2 - 48) * 100) + ((param3 - 48) * 10) + (param4 - 48);
          if (accessCode == ACCESS_ALLOW) {   // Correct number entered?
            WiFly.println("ACCESS:ALLOW");    // Send ACCESS:ALLOW back to the RN-XN
            // Would really need to do something here now that they've got the correct access code
          } else {
            WiFly.println("ACCESS:DENY");     // Bad access code - send ACCESS:DENY to RN-XV
          }
          break;

        case 'C':      // Get the Temperature (if the TMP36 is installed)
          getTemp();   // Call the get temperature function
          break;
        
        default:       // Didn't understand that command
          break;       // Do nothing if we don't recognise the instruction
      }
      return;
    }
  }
}

// If the TMP36 temperature sensor is installed, then this function will read the Analog value and
// convert the voltage to a temperaure (in celsius)
void getTemp() 
{
  float ap0 = analogRead(TMP36) * 0.004882814;
  float tdc = ((ap0 - 0.5) * 100) - 0.80;
  WiFly.print("TEMP:");
  WiFly.println(tdc);
}

