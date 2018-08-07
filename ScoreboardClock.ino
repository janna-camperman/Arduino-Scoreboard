/* 
   File:   ScoreboardClock.ino
   Author: Janna Camperman
   
   Code Description: Displays 4 digit clock either as current time or a countdown timer. Time is displayed
            on the scoreboard using individually addressable LEDs and on the control panel with a serial
            7-segment display. Timer value is set from a 10 position rotary switch.
   
   Hardware: Arduino Uno, 4-digit serial 7-Segment Displays (Sparkfun), 10 Position rotary switch (Sparkfun),
             Rotary Switch potentiometer breakout board (Sparkfun), On-On SPDT Switch,
             2x pushbutton momentary switches
*/

#include <Adafruit_NeoPixel.h>
#include <RTClib.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define clkTimerSwPin 8   // switch to determine if clock or timer

// Buzzer pin to sound a buzzer when the timer ends
// I ended up not attaching the buzzer but left it in the code anyway
//   for possible future use.
#define buzzerPin 9       

#define ledPin 10
#define timerSwPin 0      // analog pin used for the timer rotary switch
#define startStopButtonPin 6
#define resetButtonPin 7

// Rx and Tx pins for the serial 7-segment display
#define rxPin 5
#define txPin 4

SoftwareSerial serDisp(rxPin, txPin);

//analog values from timer rotary switch
const int timer1 = 0;
const int timer2 = 113;
const int timer3 = 228;
const int timer4 = 341;
const int timer5 = 454;
const int timer6 = 569;
const int timer7 = 682;
const int timer8 = 796;
const int timer9 = 911;
const int timer10 = 1023;

RTC_DS1307 rtc; // real time clock
DateTime nowInfo; // stores data about current time/date
int nowHour;
int nowMinute;

// saves time currently displayed
int hrStored = 0;
int minStored = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, ledPin, NEO_RGB + NEO_KHZ800);

// LED layout on scoreboard
//      5  6  7
//  4            8
//  3            9
//  2            10
//  1            11
//     14 13 12
// 15            25
// 16            24
// 17            23
// 18            22
//     19 20 21

int ledLayout[10][28] = {
  {1,1,1,1, 1,1,1, 1,1,1,1, 0,0,0, 1,1,1,1, 1,1,1, 1,1,1,1}, // 0
  {0,0,0,0, 0,0,0, 1,1,1,1, 0,0,0, 0,0,0,0, 0,0,0, 1,1,1,1}, // 1
  {0,0,0,0, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 0,0,0,0}, // 2
  {0,0,0,0, 1,1,1, 1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1}, // 3
  {1,1,1,1, 0,0,0, 1,1,1,1, 1,1,1, 0,0,0,0, 0,0,0, 1,1,1,1}, // 4
  {1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1}, // 5
  {1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1}, // 6
  {0,0,0,0, 1,1,1, 1,1,1,1, 0,0,0, 0,0,0,0, 0,0,0, 1,1,1,1}, // 7
  {1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1}, // 8
  {1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1}  // 9
};

int clockFlag = 0;

void setup() {
  if (! rtc.begin()) {
    while (1);
  }
  
  // adjust rtc to computer's time if needed
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 

  pinMode(clkTimerSwPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, 0);

  strip.begin();
  strip.show();

  serDisp.begin(9600);
  serDisp.write(0x77);        // Decimal control command
  serDisp.write(0b00010000);  // Turns on colon
}

void loop() {
  if(digitalRead(clkTimerSwPin) == HIGH) { //Timer
    int timerMinute;
    int timerSecond;
    int timerMinTemp = 0;
    int timerSecTemp = 0;
    int startFlag = 0;
    int stopFlag = 0;

    while(digitalRead(startStopButtonPin) == LOW){ // stay in this loop until start button pressed
      int timerSwVal = analogRead(timerSwPin);

      // Timer switch value is determined by looking for a value in a +/- 5 
      // range from saved switch values to account for possible differences
      // in read values.
      if((timerSwVal > timer1-5) && (timerSwVal < timer1+5)){
        timerMinute = 0;
        timerSecond = 30;
      }
      else if((timerSwVal > timer2-5) && (timerSwVal < timer2+5)){
        timerMinute = 1;
        timerSecond = 0;
      }
      else if((timerSwVal > timer3-5) && (timerSwVal < timer3+5)){
        timerMinute = 2;
        timerSecond = 0;
      }
      else if((timerSwVal > timer4-5) && (timerSwVal < timer4+5)){
        timerMinute = 3;
        timerSecond = 0;
      }
      else if((timerSwVal > timer5-5) && (timerSwVal < timer5+5)){
        timerMinute = 5;
        timerSecond = 0;
      }
      else if((timerSwVal > timer6-5) && (timerSwVal < timer6+5)){
        timerMinute = 7;
        timerSecond = 0;
      }
      else if((timerSwVal > timer7-5) && (timerSwVal < timer7+5)){
        timerMinute = 10;
        timerSecond = 0;
      }
      else if((timerSwVal > timer8-5) && (timerSwVal < timer8+5)){
        timerMinute = 12;
        timerSecond = 0;
      }
      else if((timerSwVal > timer9-5) && (timerSwVal < timer9+5)){
        timerMinute = 15;
        timerSecond = 0;
      }
      else if((timerSwVal > timer10-5) && (timerSwVal < timer10+5)){
        timerMinute = 20;
        timerSecond = 0;
      }

      // Only change display if the timer value has changed
      // This keeps the display from flickering from high refresh rate
      if((timerMinTemp != timerMinute) || (timerSecTemp != timerSecond)){
        timerMinTemp = timerMinute;
        timerSecTemp = timerSecond;
        displayValues(timerMinute, timerSecond);
      }

      if(digitalRead(clkTimerSwPin) == LOW) clockFlag = 1;
      if(clockFlag == 1) break;
    }
    
    while((timerMinute > 0) || (timerSecond > 0)){
      
      // This loop was added to prevent the start/stop button from being read as a stop
      // when the timer is first started. The first second runs uninterrupted unlike
      // the rest of the timer -- see below.
      if(startFlag == 0){ 
        delay(1000);
        startFlag = 1; 
      }

      if(digitalRead(clkTimerSwPin) == LOW) clockFlag = 1;
      if(clockFlag == 1) break;
      
      if(timerSecond == 0) {
        timerMinute -= 1;
        timerSecond = 59;
      }
      else {
        timerSecond -= 1;
      }

      displayValues(timerMinute, timerSecond);

      if((timerMinute <= 0) && (timerSecond <= 0)){
        digitalWrite(buzzerPin, 1);
        delay(3000); //buzzer for 3 seconds
        digitalWrite(buzzerPin, 0);
        break;
      }

      // A start/stop button check is done multiple times a second to avoid missing
      //   a button press.
      // I realize this may add a bit of a delay - thus making the second a hair longer
      //   than an actual second but for this application I didn't need 100% accuracy
      //   so I just stuck with a more simple solution.
      delay (250);
      if(digitalRead(startStopButtonPin) == HIGH) stopFlag = 1;
      if(stopFlag == 0) delay(250);
      if(digitalRead(startStopButtonPin) == HIGH) stopFlag = 1;
      if(stopFlag == 0) delay(250);
      if(digitalRead(startStopButtonPin) == HIGH) stopFlag = 1;
      if(stopFlag == 0) delay(250);

      if(digitalRead(clkTimerSwPin) == LOW) clockFlag = 1;
      if(clockFlag == 1) break;
      
      if((digitalRead(startStopButtonPin) == HIGH) || (stopFlag == 1)) { // if button pressed - stop
        delay(750); // enough delay to make sure the button is not still pressed
        stopFlag = 0;
        while(digitalRead(startStopButtonPin) == LOW) { // while button not pressed
          if(digitalRead(clkTimerSwPin) == LOW) clockFlag = 1;
          if(clockFlag == 1) break;
      
          if(digitalRead(resetButtonPin) == HIGH){
            timerMinute = 0;
            timerSecond = 0;
            break;
          }
        }

        if(clockFlag == 1) break;
      }
    }
  }
  
  else { // Clock
    nowInfo = rtc.now();
    int hr = nowInfo.hour();
    int mn = nowInfo.minute();
    if(hr > 12) hr = hr - 12;

    // Only change display if the clock value has changed
    // This keeps the display from flickering from high refresh rate
    if((hr != hrStored) || (mn != minStored)){
      hrStored = hr;
      minStored = mn;
      displayValues(hrStored, minStored);
    }
    
    if(clockFlag == 1){
      
      // This display might seem redundant, however it is necessary if switched between
      // timer and clock before the time has changed from previous saved then the time
      // will not display until the minute advances. Thus, this forces the time to display
      // even if it hasn't changed from last saved.
      displayValues(hrStored, minStored);
      
      clockFlag = 0;
    }
  }
}

void displayValues(int num1, int num2){
  // Separate each digit of values to display
  int disp1 = num1/10;
  int disp2 = num1%10;
  int disp3 = num2/10;
  int disp4 = num2%10;

  // For each of the digits to display on the LEDs, the strip refers to the ledLayout array
  // for the corresponding digit, setting each LED to either 1 or 0.
  // Digits are set from left to right.
  for(int i=0; i<25; i++){
    if(ledLayout[disp1][i] == 1){
      strip.setPixelColor(i, strip.Color(255,0,0));
    }
    else if(ledLayout[disp1][i] == 0){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  for(int i=25; i<50; i++){
    if(ledLayout[disp2][i-25] == 1){
      strip.setPixelColor(i, strip.Color(255,0,0));
    }
    else if(ledLayout[disp2][i-25] == 0){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  for(int i=50; i<75; i++){
    if(ledLayout[disp3][i-50] == 1){
      strip.setPixelColor(i, strip.Color(255,0,0));
    }
    else if(ledLayout[disp3][i-50] == 0){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  for(int i=75; i<100; i++){
    if(ledLayout[disp4][i-75] == 1){
      strip.setPixelColor(i, strip.Color(80,0,0));
    }
    else if(ledLayout[disp4][i-75] == 0){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  strip.show();

  // Combine 4 digits to display into a char array to send to serial display
  char disp[4];
  sprintf(disp, "%d%d%d%d", disp1, disp2, disp3, disp4);

  serDisp.write(0x76);        // Clear display
  serDisp.write(0x77);        // Decimal control command
  serDisp.write(0b00010000);  // Turns on colon
  serDisp.write(disp);
}

