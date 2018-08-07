/* 
   File:   ScoreboardScores.ino
   Author: Janna Camperman
   
   Code Description: Displays two 3-digit scores from keypad input.
   
   Hardware: Arduino Uno, 12 Button Keypad, 2x 4-digit serial 7-Segment Displays (Sparkfun),
             On-On SPDT Switch
*/

#include <Key.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define scoreSwPin 13 // used to determine which score to write to -- on-on toggle switch used
#define rxPin1 15
#define txPin1 12
#define rxPin2 14
#define txPin2 11
#define ledPin1 9
#define ledPin2 10

SoftwareSerial serDisp1(rxPin1, txPin1); // used to display score1 on the control console
SoftwareSerial serDisp2(rxPin2, txPin2); // used to display score2 on the control console

// keypad initialization
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
// the pins were ordered so that they are in order from the physical keypad 8-2 (L to R)
byte rowPins[ROWS] = {7, 2, 3, 5};
byte colPins[COLS] = {6, 8, 4};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//LED initialization
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(100, ledPin1, NEO_RGB + NEO_KHZ800); // Score1 on scoreboard
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(100, ledPin2, NEO_RGB + NEO_KHZ800); // Score2 on scoreboard

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
  {1,1,1,1, 1,1,1, 1,1,1,1, 0,0,0, 1,1,1,1, 1,1,1, 1,1,1,1}, //0
  {0,0,0,0, 0,0,0, 1,1,1,1, 0,0,0, 0,0,0,0, 0,0,0, 1,1,1,1}, //1
  {0,0,0,0, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 0,0,0,0}, //2
  {0,0,0,0, 1,1,1, 1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1}, //3
  {1,1,1,1, 0,0,0, 1,1,1,1, 1,1,1, 0,0,0,0, 0,0,0, 1,1,1,1}, //4
  {1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1}, //5
  {1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1}, //6
  {0,0,0,0, 1,1,1, 1,1,1,1, 0,0,0, 0,0,0,0, 0,0,0, 1,1,1,1}, //7
  {1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 1,1,1,1}, //8
  {1,1,1,1, 1,1,1, 1,1,1,1, 1,1,1, 0,0,0,0, 1,1,1, 1,1,1,1}  //9
};

void setup(){
  serDisp1.begin(9600);
  serDisp1.write(0x76); // Clear
  serDisp2.begin(9600);
  serDisp2.write(0x76); // Clear

  strip1.begin();
  strip1.show();
  strip2.begin();
  strip2.show();
}

void loop(){

  // Program takes 3 digit scores, wait for each digit until it is received (in order)
  char keypadVal[3];
  keypadVal[0] = keypad.waitForKey();
  keypadVal[1] = keypad.waitForKey();
  keypadVal[2] = keypad.waitForKey();

  // Convert ASCII value input to numerical digit
  int val1 = keypadVal[0] - 48;
  int val2 = keypadVal[1] - 48;
  int val3 = keypadVal[2] - 48;

  if(digitalRead(scoreSwPin) == LOW){ // Score1
    for(int i=0; i<25; i++){ // Set LEDs for first digit of score1
      if(ledLayout[val1][i] == 1){
        strip1.setPixelColor(i, strip1.Color(255,0,0));
      }
      else if(ledLayout[val1][i] == 0){
        strip1.setPixelColor(i, strip1.Color(0,0,0));
      }
    }
    for(int i=25; i<50; i++){ // Set LEDs for second digit of score1
      if(ledLayout[val2][i-25] == 1){
        strip1.setPixelColor(i, strip1.Color(255,0,0));
      }
      else if(ledLayout[val2][i-25] == 0){
        strip1.setPixelColor(i, strip1.Color(0,0,0));
      }
    }
    for(int i=50; i<75; i++){ // Set LEDs for third digit of score1
      if(ledLayout[val3][i-50] == 1){
        strip1.setPixelColor(i, strip1.Color(255,0,0));
      }
      else if(ledLayout[val3][i-50] == 0){
        strip1.setPixelColor(i, strip1.Color(0,0,0));
      }
    }
    strip1.show(); // Display to LEDs

    char disp[3];
    sprintf(disp, "%d%d%d", val1, val2, val3); // Concatenate score digits into a char array
    serDisp1.write(0x76); // Clear
    serDisp1.write(0x79); // Send the Move Cursor Command
    serDisp1.write(0x01); // Send the data byte, with value 1
    serDisp1.write(disp); // Display score1 to serial display on control panel
  }

  else { //Score2
    for(int i=0; i<25; i++){
      if(ledLayout[val1][i] == 1){ // Set LEDs for first digit of score2
        strip2.setPixelColor(i, strip2.Color(255,0,0));
      }
      else if(ledLayout[val1][i] == 0){
        strip2.setPixelColor(i, strip2.Color(0,0,0));
      }
    }
    for(int i=25; i<50; i++){ // Set LEDs for second digit of score2
      if(ledLayout[val2][i-25] == 1){
        strip2.setPixelColor(i, strip2.Color(255,0,0));
      }
      else if(ledLayout[val2][i-25] == 0){
        strip2.setPixelColor(i, strip2.Color(0,0,0));
      }
    }
    for(int i=50; i<75; i++){ // Set LEDs for third digit of score2
      if(ledLayout[val3][i-50] == 1){
        strip2.setPixelColor(i, strip2.Color(255,0,0));
      }
      else if(ledLayout[val3][i-50] == 0){
        strip2.setPixelColor(i, strip2.Color(0,0,0));
      }
    }
    strip2.show(); //Display to LEDs

    char disp[3];
    sprintf(disp, "%d%d%d", val1, val2, val3); // Concatenate score digits into a char array
    serDisp2.write(0x76); // Clear
    serDisp2.write(0x79); // Send the Move Cursor Command
    serDisp2.write(0x01); // Send the data byte, with value 1
    serDisp2.write(disp); // Display score2 to serial display on control panel
  }
}
