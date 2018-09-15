#include <Servo.h>

Servo left; 
Servo right;

// light sensor constants
const int GREY       = 750;
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
int l_light = 0;
int r_light = 0;
int figure_8 = 0;
int wait = 1000;
int turn_wait = 900;

void setup() {
  Serial.begin(9600); // read analog input
  left.attach(LEFT_PIN);
  right.attach(RIGHT_PIN);
}
// main function
void loop() {
  l_light = analogRead(L_LIGHT);
  r_light = analogRead(R_LIGHT);
  if (l_light < GREY && r_light < GREY){ // detect intersection
    figure_8 ++;
    if(figure_8 == 1){
      auto_right(); 
    } else if(figure_8 < 6){ //2 3 4 5
      auto_left();
    } else if(figure_8 < 8){ // 6 7 
      auto_right();
    } else { // figure_8 == 8
      stop_moving();
    } 
//LINE FOLLOWING
  } else if(l_light < GREY){ 
    turn_left();  
  } else if(r_light < GREY){
    turn_right();
  } else {
    move_forward();
  }
}

void auto_right(){
    turn_right();
    delay(1200);
    l_light = analogRead(L_LIGHT);
    
    while(l_light > GREY){ // is grey
      l_light = analogRead(L_LIGHT);
      turn_right();
    }
}

void auto_left(){
    turn_left();
    delay(1200);
    r_light = analogRead(R_LIGHT);
    
    while(r_light > GREY){ // is grey
      r_light = analogRead(R_LIGHT);
      turn_left();
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

