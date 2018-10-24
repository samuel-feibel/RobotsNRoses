#include <Servo.h>

Servo left; 
Servo right;

// light sensor constants
const int GREY       = 600;
// pin constants
const int L_LIGHT    = A2;
const int R_LIGHT    = A3;
const int LEFT_PIN   = 6;
const int RIGHT_PIN  = 5;
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
  Serial.println(l_light);
  Serial.println(r_light);
  Serial.println();
  Serial.println();
  if (l_light < GREY && r_light < GREY) {
      //auto_right();
      while(1){stop_moving();}
  }
  else if(l_light < GREY){ 
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
void auto_right(){ 
    r_light = analogRead(R_LIGHT);
    while(r_light < GREY){ // is white
      r_light = analogRead(R_LIGHT);
      if(r_light > GREY) { 
        Serial.println("R_LIGHT IS GREY");
        break;
      }
      turn_right();
      Serial.println("RLIGHT IS WHITE");
    }
    l_light = analogRead(L_LIGHT);
    while(l_light < GREY){ // is white
      l_light = analogRead(L_LIGHT);
      if(l_light > GREY) { 
        Serial.println("L_LIGHT IS GREY");
        stop_moving();
        break;
      }
      turn_right();
      Serial.println("L_LIGHT IS WHITE");
    }
}
