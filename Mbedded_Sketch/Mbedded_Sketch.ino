#include <Time.h>
#include <EEPROM.h>

int modePin = 0;          // number of analog pin for Mode select
int dumpPin = 2;          // number of analog pin for Dump EEPROM / Reset
int padPin = 5;           // number of analog pin for sleeping pad
int lightPin = 3;        // number of digital output pin for lamp
int feedbackLight1 = 4;   // number of digital output pin for feedback light 1;   This light on = mode 1.
int feedbackLight2 = 2;   // number of digital output pin for feedback light 2;   This light on = mode 2.  Both lights = mode 3.
int brightness = 255;     // initial value of brightness for lamp when dimming
int sensorValue = 0;
int sensorMin = 1023;
int sensorMax = 0;

int padVal = 0;           // value of pad pin           
int modeVal = 0;          // value of mode pin
int dumpVal = 0;          // value of DUMP pin
int sheetMode = 1;        // mode of sheet.  Default is mode 1
int sleeping = 0;         // boolean to indicate we are in sleep mode
int dumped = 0;           // boolean to indicate if we have dumped
int reset = 0;            // boolean to indicate if we have reset the sheet
int modeOver = 0;         // boolean to indicate that the mode has finished being applied.
const int TIMETOSLEEP = 0;   // seconds that must pass before sleep events are noted.  This is here to let people fall asleep before calculating.


//Time variables
// last minute that was recorded.  We will only capture time based on two-minute 
//    intervals.  This is due to 1) the fact that we have very little storage space
//    and 2) we will only be interested in waking up after 2 minutes of inactivity
int lastMinute = 0;          // last minute that was recorded. 
int elapsedMinutes = 0;      // number of minutes that have elapsed - 2-minute chunks
long startTime = 0;          // placeholder for initial starting time.
long elapsedTime = 0;        // number of seconds that have passed since starting
long dumpStartTime = 0;      // placeholder for initial starting time for reset button
long dumpHoldTime = 0;       // placeholder for how long the dump button was held down
long modeStartTime = 0;      // placeholder for initial starting time for mode start button
long modeHoldTime = 0;       // placeholder for how long the mode button was held for


int caught = 0;           // boolean indicating we have hit an 'event'
int dump = 0;             // boolean indicating we have dumped EEPROM


int LOWVALUE = 100;       // low value for a button
int MIDVALUE = 400;       // 
int HIGHVALUE = 900;      // high value for a button
int DEBUG = 0;            // DEBUG boolean.  Used when using serial ouput


// button constants
const int DUMPBUTTONVAL = 400;    // value of reset button sensor (pressed)
const int MODEBUTTONVAL = 500;     // value of modebutton sensor (pressed)
int PADBUTTONVAL = 550;       // value of padbutton sesnor (pressed)
const int DUMPTIME = 3;            // number of seconds to hold down RESETBUTTON to dump data
const int RESETTIME = 6;           // number of seconds to hold down RESETBUTTON to reset EEPROM/timers
const int MODE2TIME = 3;           // number of seconds to hold down MODEBUTTON to set mode 2
const int MODE3TIME = 6;           // number of seconds to hold down MODEBUTTON to set mode 3
const int MODE2TIMER = 5;         // number of seconds until light is shut off
const int MAXBRIGHTNESS = 255;     // highest value for analog LED light
//const int MODE3TIME = 9;           // number of seconds to hold down MODEBUTTON to set mode 3


// EEPROM variables
int pointer = 0;  //pointer that points to last register written to




/*Smoothing of sensor data used in indication of an EVENT taken from:
Created 22 April 2007
  By David A. Mellis  <dam@mellis.org>
  modified 9 Apr 2012
  by Tom Igoe
  http://www.arduino.cc/en/Tutorial/Smoothing
*/
//Smoothing variables
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A0;




 // ######################################################################
 // SETUP
 void setup() {
    Serial.begin(9600); 
    pinMode(lightPin, OUTPUT);
    analogWrite(lightPin, MAXBRIGHTNESS);
    //analogWrite(7, MAXBRIGHTNESS);

 
    // Smoothing initialize all the readings to 0: 
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
       readings[thisReading] = 0;      
 
 /*   Calibration code taken from....
      created 29 Oct 2008
      By David A Mellis
      modified 30 Aug 2011
      By Tom Igoe
 
 http://arduino.cc/en/Tutorial/Calibration
 
    This example code is in the public domain.
    */
    
      // calibrate during the first five seconds 
  while (millis() < 10000) {
    sensorValue = analogRead(padPin);

    // record the maximum sensor value
    if (sensorValue > sensorMax) {
      sensorMax = sensorValue;
    }

    // record the minimum sensor value
    if (sensorValue < sensorMin) {
      sensorMin = sensorValue;
    }
  }   
  int diff = sensorMax - sensorMin;
  int delta;
  delta = floor((diff)/2);
  PADBUTTONVAL = sensorMax - delta;
  
  if(DEBUG){
    Serial.println("calibrated");

    Serial.println(PADBUTTONVAL);
    Serial.println(sensorMin);
    Serial.println(sensorMax);
  }

  
  
  while (millis() < 12000) {
    
  }
      //analogWrite(lightPin, MAXBRIGHTNESS);

 }


// ######################################################################
// Main Loop
void loop() {
  modeVal = analogRead(modePin);
  dumpVal = analogRead(dumpPin);
  padVal = analogRead(padPin);

 
 // ######################################################################
 //Smothing calculations - used to ID an EVENT
  // subtract the last reading:
  total= total - readings[index];         
  // read from the sensor:  
  readings[index] = analogRead(padPin); 
  // add the reading to the total:
  total= total + readings[index];       
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings)              
    // ...wrap around to the beginning: 
    index = 0;                           

  // calculate the average:
  average = total / numReadings;          
 
 // ######################################################################
 // We've picked up an 'event' on he big pad.  So go 
 //    ahead and calculate how many minutes
 //    have elapsed and store in EEPROM if this 
 //    is a new 2-minute period
 //   
 //    We do not start calculating until TIMETOSLEEP seconds have passed
 if((abs(padVal - average) > 30) && (elapsedTime > TIMETOSLEEP) && sleeping){
 
   
    //calculate number of 2-minute periods that have passed.  
    elapsedMinutes = floor(elapsedTime / 5);
    
    // we must 'roll' the # of 2-minute periods if it is over 255 due to space concerns...
    if(elapsedMinutes > 255){
       elapsedMinutes = elapsedMinutes % 256;
    }
   
    if(elapsedMinutes != lastMinute){
       if(DEBUG){
         Serial.print(padVal);
         Serial.print("."); 
         Serial.print(elapsedTime);
         Serial.print(".");
         Serial.print(elapsedMinutes);
         Serial.print(".");
         Serial.println(lastMinute);
       }
       caught = 1;
       dump = 0;
       
       // set lastMinute to elapsedMinutes as we now have a new time mark.
       lastMinute = elapsedMinutes;

       // and update EEPROM with the timestamp of this event
       updateEEPROM(elapsedMinutes);
    }   
 } //end of ID'ing an event
 
 
   // ######################################################################
   // Set sleeping if we have pressure on the big pad
   if(padVal < PADBUTTONVAL && !sleeping)
   {
      //Serial.println(padVal);
      if(DEBUG) Serial.println("Sleeping");
      sleeping = 1; 
      startTime = millis();
   }
 
   if(sleeping)
   {
      // number of seconds that have passed
      elapsedTime = (millis() - startTime) / 1000L;
   }
 
   // ######################################################################
   // Now handle seperate actions when sleeping based on mode
   
   // Mode 1 shuts the light off as someone lays down
   if (sleeping && (sheetMode == 1) && !modeOver && (elapsedTime > MODE2TIMER))
   {
      if(DEBUG) Serial.println("Mode 1 in action"); 
      modeOver = 1;
      analogWrite(lightPin, 0);

   }
   
   // Mode 2 Shuts the light off after a period of time
   //if (sleeping && (sheetMode == 2) && !modeOver && (elapsedTime > MODE2TIMER))
   //{



     // if(DEBUG) Serial.println("Mode 2 in action");
     // modeOver = 1;
     // analogWrite(lightPin, 0);
  // }
   
   // Mode 3 dims light over time
   if (sleeping && (sheetMode == 3) && !modeOver)
   {
      /* Code to diminish brightness of LED
         created 2006
         by David A. Mellis
         modified 30 Aug 2011
         by Tom Igoe and Scott Fitzgerald
      */
      delay(10);
      brightness--;
      analogWrite(lightPin, brightness);
      
      if(brightness == 0)
      {
         modeOver = 1; 
      }
     
      if(DEBUG) Serial.println("Mode 3 in action"); 
   }
 
 
   // ######################################################################
   // Handle pushing the reset button
   if (dumpVal > DUMPBUTTONVAL)
   {
      dumpStartTime = millis();
      dumped = 0;
      reset = 0;
   }
   
   // Amount of time dumpButtong was held
   //  0 or near 0 = not being held
   dumpHoldTime = (millis() - dumpStartTime) / 1000L;
   
   // Handle being held for first pass
   if ((dumpVal < DUMPBUTTONVAL) && (dumpHoldTime > 3) && !dumped){
     
      //Serial.println("Dump");
      dumpEEPROM();
      dumped = 1;
   }
   
   // handle being held for second pass
   if ((dumpVal < DUMPBUTTONVAL) && (dumpHoldTime > 6) && !reset)
   {
       if(DEBUG) Serial.print("Resetting...");
       resetSheet();
       reset = 1;
   }
   
   
   // ######################################################################
   // Handle pushing the mode button
   if (modeVal > MODEBUTTONVAL)
   {
      modeStartTime = millis(); 
   }
   
   // Amount of time dumpButtong was held
   //  0 or near 0 = not being held
   modeHoldTime = (millis() - modeStartTime) / 1000L;
   
   // Test how long the button was held and adjust mode accordingly
   if ((modeVal < MODEBUTTONVAL) && (modeHoldTime > 5))
   {
           analogWrite(lightPin, 0);
      
      analogWrite(lightPin, 0);
      delay(100);
      analogWrite(lightPin, MAXBRIGHTNESS);

      sheetMode = 3;
      if(DEBUG) Serial.println(sheetMode);  
   }
   //else if((modeVal < MODEBUTTONVAL) && (modeHoldTime > 3))
   //{
   //   sheetMode = 2;
   //   if(DEBUG) Serial.println(sheetMode); 
   //   analogWrite(lightPin, 0);
   //   delay(100);
   //   analogWrite(lightPin, MAXBRIGHTNESS);
   //   delay(100);
   //   analogWrite(lightPin, 0);
   //   delay(100);
   //   analogWrite(lightPin, MAXBRIGHTNESS);
   //}
   
}






// ######################################################################
//Function that resets EEPROM and sheet
//
// function resets all EEPROM values, resets the
//    mode the sheet is in and assumes you are no
//    longer sleeping on the sheet
void resetSheet()
{
    // Reset EERPOM by simply saying there are no records present
    EEPROM.write(0,0);
    
    //undo Mode settings
    sheetMode = 1;
    
    //reset timer
    startTime = millis();
  
    // reset booleans
    dumped = 0;
    sleeping = 0;
    modeOver = 0;

    // reset lamp
    analogWrite(lightPin, MAXBRIGHTNESS);

}


// ######################################################################
// Function that updates EEPROM.  
//
//  function reads slot 0, which contains the number of records
//     stored in EERPOM
//  Input:  value   - value to place in next slot
//
void updateEEPROM(int value){
    int myPointer = EEPROM.read(0);
    
    myPointer++;
    EEPROM.write(myPointer, value);
    EEPROM.write(0, myPointer);
}



// ######################################################################
// Function dumps EEPROM to the serial port
//
// Take no input.  Reads the value in record 0 which indicates
//    how many items are stored in EEPROM.
//
// Debug allows for more printing to the serial port.
// 
// Format is as follows:  
//    a "." is a delimeter between values
//    a "#" is the signal that all values have been dumped to the serial port
//
void dumpEEPROM(){
 
   // get number of records
   int numRecords = EEPROM.read(0);
  
  
   if(DEBUG){
  
 
      Serial.println(numRecords);
      Serial.println("Dumping EEPROM"); 
   }
 
   for (int i = 1; i <= numRecords; i++){
       Serial.print(EEPROM.read(i)); 
     
       // delimiter text.  If we are at the end, print '#'.  Else print a "."
       if(i == numRecords){
         Serial.print("#");
       }
       else
       {
          Serial.print(".");
       }   
   }  //end of for
 
   if(DEBUG){
      Serial.println("Finished dumping EEPROM");  
   }
 
 
}
