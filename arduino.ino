#include <Servo.h>   
#include <math.h>
#include <SoftwareSerial.h>
#define Rx 10 // DOUT to pin 10
#define Tx 11 // DIN to pin 11
SoftwareSerial Xbee (Rx, Tx); //setting up Xbee
const int pingPin = 2; //pin for ultrasonic sensor
boolean on=false; //determines if ultrasonic sensor is on
// Define the left and right servos
Servo servoLeft; 
Servo servoRight;
const int threshold = 27; // used for defining black v white
//pins designating the QTI sensors
int leftpin = 4;
int centerleftpin = 5;
int centerrightpin = 6;
int rightpin = 7;
int oL = 4;
int iL = 5;
int iR = 6;
int oR = 7;
boolean on_first = false; //true if on the first block
boolean past_first = false; //true if past the first block
//time first seen, time second seen, time it takes to travel down the line
unsigned long time_tot=0;
unsigned long time_first=0;
unsigned long time_second=0; 
boolean stop1 = false; //triggers loop to stop and display number
double spaces; //spaces seen between blocks
int number; //integer form of spaces between blocks
int tic; //number of tic marks needed to pass
int otherpeople = -1; //increments as receives signals from others to determine when to start moving
boolean startmove=false; //enters loop to move through white space
boolean firstmove = true; //triggers if statement that records time right when bot starts moving
unsigned long wait_for_people = 0; //time recorded when bot starts moving
boolean ticmove=false; //enters loop to start moving to tick marks
unsigned long firsttimecount=0; //when pass a legitimate tick mark, record time
unsigned long sectimecount=0; //when see all black, record to see if actually a tick mark
int count=0; //counts number of tic marks past
boolean trapatend=false; //literally trap at end


void setup()
{
  Xbee.begin(9600);
  servoLeft.attach(13);  // Attach (programmatically connect) servos to pins on Arduino
  servoRight.attach(12);
  on=true; //turning ultrasonic sensor on
  //initializing seven segment display pins
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(3, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(8, OUTPUT);
   delay(100); //bot pauses for 5 seconds before moving
}
   
void loop()
{
  //loop to display number and receive signals to decide when to move
  while (stop1){ 
    if(time_tot==0){ //calculate and display spaces
      time_tot=millis();
      spaces = calcSpaces((time_tot-100), time_first, time_second);
      if(spaces<1.5){
        digitalWrite(A1,HIGH);
        digitalWrite(A2,HIGH);
        number=1;
        tic=5;}
      if(spaces>=1.5&&spaces<2.5){
        digitalWrite(A0,HIGH);
        digitalWrite(A1,HIGH);
        digitalWrite(A3,HIGH);
        digitalWrite(A4,HIGH);
        digitalWrite(3,HIGH);
        number=2;
        tic=4;
        }
      if(spaces>=2.5&&spaces<3.5){
        digitalWrite(A0,HIGH);
        digitalWrite(A1,HIGH);
        digitalWrite(A2,HIGH);
        digitalWrite(A3,HIGH);
        digitalWrite(3,HIGH);
        number=3;
        tic=3;}
      if(spaces>=3.5){
        digitalWrite(A1,HIGH);
        digitalWrite(A2,HIGH);
        digitalWrite(A5,HIGH);
        digitalWrite(3,HIGH);
        number=4;
        tic=2;
        }   
    }
     if(Xbee.available()) { // Is data available from XBee?
      char incoming = Xbee.read(); // Read character
      if (incoming=='6'){
        otherpeople++;
        if (otherpeople==(number-1)){ 
          stop1=false;
          startmove=true;
        }
      }
    }
  }
  
  //loop to move through white space to black line
  while(startmove){
    if (firstmove){
      wait_for_people = millis(); //time recorded when bot starts to move
      firstmove=false;
    }
     unsigned long see_all_white = millis(); //record time when see all white
    //if see all white, move forward
    if (RCtime(centerleftpin) < threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)<threshold) {
    servoLeft.writeMicroseconds(1400);   
    servoRight.writeMicroseconds(1600);
    }
    else if (see_all_white>300){ //start turning when doesn't see all white
      servoLeft.writeMicroseconds(1500);         
      servoRight.writeMicroseconds(1600);
      delay(10);
      if(!(RCtime(centerleftpin) > threshold && RCtime(leftpin) > threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)>threshold)){
      startmove=false;
      ticmove=true;         
      Xbee.print('6'); // communicate number
      digitalWrite(8, LOW);   
      }
    }
    else{ //if doesn't see all white but nowhere near black line, keep going straight
         servoLeft.writeMicroseconds(1400);    
         servoRight.writeMicroseconds(1600);
    }
  }

  //loop to follow line and stop at tic mark
  while(ticmove){
    //right and center right black
  if (RCtime(centerleftpin) < threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)>threshold) {
     servoLeft.writeMicroseconds(1600);     
     servoRight.writeMicroseconds(1600);    
     delay(10);
  }
  //right black
  else if (RCtime(centerleftpin) < threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)>threshold) {
  servoLeft.writeMicroseconds(1600);     
  servoRight.writeMicroseconds(1600);    
  delay(10);
  }
  //left black
  else if (RCtime(centerleftpin) < threshold && RCtime(leftpin) > threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1400);     
  servoRight.writeMicroseconds(1400);    
  delay(10);
  }
  //left and center left black
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) > threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1400);     
  servoRight.writeMicroseconds(1400);    
  delay(10);
  }
  //center two black
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1400);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  }
  //center left and right
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)>threshold) {
  servoLeft.writeMicroseconds(1400);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  }
  //center left center right and right
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)>threshold) {
  servoLeft.writeMicroseconds(1400);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  }
  //all black
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) > threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)>threshold) {
  sectimecount=millis();
  if(((sectimecount-wait_for_people)>2000)&&((sectimecount-firsttimecount)>1000)){
    count++;
  firsttimecount=millis();
  }
    if (count == tic){
    servoLeft.detach();
    servoRight.detach();
    ticmove=false;
    trapatend=true;
    break; // will  break out of ticmove, get trapped in last loop
  }
  servoLeft.writeMicroseconds(1400);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  }
  //left center left center right
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) > threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1400);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  }
  //left center
  else if (RCtime(centerleftpin) > threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1400);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1400);
  delay(10);
  }
  //right center
  else if (RCtime(centerleftpin) < threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)> threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1600);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  }
  //none
  else if (RCtime(centerleftpin) < threshold && RCtime(leftpin) < threshold && RCtime(centerrightpin)< threshold && RCtime(rightpin)<threshold) {
  servoLeft.writeMicroseconds(1600);                      // Stop sending servo signals
  servoRight.writeMicroseconds(1600);
  delay(10);
  } 
  }

  //loop to trap at very end
  while(trapatend){ 
  }

  //normal line following
  if(!isBlack(oL) && !isBlack(iL) && !isBlack(iR) && !isBlack(oR)){
    stayStill();
    stop1=true;
  }
  //if WBBW go Straight
  if(!isBlack(oL) && isBlack(iL) && isBlack(iR) && !isBlack(oR)){
    goStraight();
  }
  //If the inner sensors get misaligned, make slight corrections
  //BW, adjust left
  if(isBlack(iL) && !isBlack(iR)){
    turnLeftP1();
  }
  //WB, adjust right
  if(!isBlack(iL) && isBlack(iR)){
    turnRightP1();
  }
  if(isBlack(oL) && isBlack(iL) && isBlack(iR) && !isBlack(oR)){
    turnLeftM();
  }
  if(!isBlack(oL) && !isBlack(iL) && !isBlack(iR) && isBlack(oR)){
    turnRightM();
  }

  //ultrasonic sensor code
  if(on){
  long duration, inches, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  inches = microsecondsToInches(duration);
  if(inches<9 && inches>7){
    if(time_first==0){
      unsigned long temp = millis();
      if (temp>1000){
        time_first = millis();
        on_first = true;
        Serial.println(time_first);
        digitalWrite(8, HIGH);
      }
    }
    unsigned long temp = millis();
    unsigned long diff = temp-time_first;
    if(time_second==0 && past_first==true && diff>1500){
      time_second=millis();
      Serial.println(time_second);
      digitalWrite(8, HIGH);
    }
  }
  else{
    if(on_first==true){
      on_first=false;
      past_first = true;
      digitalWrite(8, LOW);
    }
  } 
}
}

//BELOW ARE METHODS USED IN MAIN LOOP
//get time from ultrasonic sensor
long RCtime(int sensPin){
   long result = 0;
   pinMode(sensPin, OUTPUT);    // make pin OUTPUT
   digitalWrite(sensPin, HIGH); // make pin HIGH to discharge capacitor - study the schematic
   delay(1);                    // wait a  ms to make sure cap is discharged
   pinMode(sensPin, INPUT);     // turn pin into an input and time till pin goes low
   digitalWrite(sensPin, LOW);  // turn pullups off - or it won't work
   while(digitalRead(sensPin)){ // wait for pin to go low
    result++;
   }
   return result;                 // report results   
}

//convert time from ultrasonic sensor to inches
long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}

//calculate spaces between two blocks
double calcSpaces(unsigned long tottime, unsigned long firsttime, unsigned long sectime){
  double tot = (double)tottime;
  double first = (double)firsttime;
  double sec = (double) sectime;
  double x= (sec-first)*5.5/tot;
  return x;
}

//line following methods
boolean isBlack(int pin){
   long RCtime_1 = RCtime(pin);
   if(RCtime_1>threshold){ 
   Serial.println(RCtime_1);
   return true;
   }
   return false;
}
void turnLeftP1() {
  servoRight.writeMicroseconds(1560);                    
  servoLeft.writeMicroseconds(1350);
}
void turnRightP1() {
 servoRight.writeMicroseconds(1650);                      
 servoLeft.writeMicroseconds(1420);
}
void goStraight(){
 servoRight.writeMicroseconds(1700);                      
 servoLeft.writeMicroseconds(1300); 
}
void stayStill(){
  servoRight.writeMicroseconds(1500);
  servoLeft.writeMicroseconds(1500);
}
void turnLeftM() {
  servoRight.writeMicroseconds(1560);                    
  servoLeft.writeMicroseconds(1400);
  delay(50);
}
void turnRightM() {
 servoRight.writeMicroseconds(1600);                  
 servoLeft.writeMicroseconds(1440);
 delay(50);
}

