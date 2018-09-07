#include <Servo.h>

Servo left; 
Servo right;

// light sensor constants
const int GREY       = 700;
const int WHITE      = 150;
// pin constants
const int L_LIGHT    = A1;
const int R_LIGHT    = A0;
const int LEFT_PIN   = 10;
const int RIGHT_PIN  = 9;
// speed constants 
const int STOP       = 90;
const int R_FORWARD  = 0;
const int L_FORWARD  = 180;
const int R_BACKWARD = 180;
const int L_BACKWARD = 0;

void setup() {
  //Serial.begin(9600); // read analog input
  left.attach(LEFT_PIN);
  right.attach(RIGHT_PIN);
}
// main function
void loop() {
  
  int l_light = analogRead(L_LIGHT);
  int r_light = analogRead(R_LIGHT);
  
  //if (l_light < GREY && r_light < GREY){
    // found intersection
  //}
  
  if(l_light < GREY){ // TODO change to else if
    turn_left();  
  } else if(r_light < GREY){
    turn_right();
  } else {
    move_forward();
  }
}

void move_forward(){
  left.write(L_FORWARD);
  right.write(R_FORWARD); 
}

void turn_right(){
  left.write(L_FORWARD);
  right.write(STOP);
}

void turn_left(){
  left.write(STOP);
  right.write(R_FORWARD);
}

void stop_moving(){
  left.write(STOP);
  right.write(STOP);  
}

