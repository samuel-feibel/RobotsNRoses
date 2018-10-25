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

//WALL CONSTANTS
const int R_WALL = A1;
const int L_WALL = A0;
const int F_WALL = A5;

int wall_threshold = 200;
int r_wall = 0;
int l_wall = 0;
int f_wall = 0;

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
      detect_walls();
      //auto_right();
      //while(1){stop_moving();}
  }
  else if(l_light < GREY){ 
    nudge_left();  
  } else if(r_light < GREY){
    nudge_right();
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
  right.write(R_BACKWARD);
}

void turn_left(){
  left.write(L_BACKWARD);
  right.write(R_FORWARD);
}

void nudge_right(){
  left.write(L_FORWARD);
  right.write(STOP);
}

void nudge_left(){
  left.write(STOP);
  right.write(R_FORWARD);
}
void stop_moving(){
  left.write(STOP);
  right.write(STOP);  
}

void auto_right(){ 
    r_light = analogRead(R_LIGHT);
    move_forward();
    delay(150);
    turn_right();
    delay(500);

    l_light = analogRead(L_LIGHT);
    while(l_light > GREY){   // is grey
    l_light = analogRead(L_LIGHT);
    turn_right();
    }
}

void auto_left(){
    l_light = analogRead(L_LIGHT);
    move_forward();
    delay(150);
    turn_left();
    delay(500);

    r_light = analogRead(R_LIGHT);
    while(l_light > GREY){   // is grey
    r_light = analogRead(R_LIGHT);
    turn_left();
    }
    stop_moving();
}

void auto_180(){
    r_light = analogRead(R_LIGHT);
    move_forward();
    delay(150);
    turn_right();
    delay(1250);

    l_light = analogRead(L_LIGHT);
    while(l_light > GREY){   // is grey
    l_light = analogRead(L_LIGHT);
    turn_right();
    }
  
}

//wall detection
void detect_walls(){
  l_light = analogRead(L_LIGHT);
  r_light = analogRead(R_LIGHT);
  r_wall = analogRead(R_WALL);
  l_wall = analogRead(L_WALL);
  f_wall = analogRead(F_WALL);
  if(r_wall < wall_threshold) {
    Serial.println("TURN RIGHT");
    auto_right(); //turn right if possible
   // delay(1000);
   
  } else if (l_wall < wall_threshold) {
    Serial.println("TURN LEFT");
    auto_left(); //if can't turn right, turn left if possible
    //delay(1000);
    
  } else if (f_wall < wall_threshold) {
    move_forward(); //if can't turn right or left, move forward if possible
    
  } else {
    auto_180(); //else it's a dead end, make a u-turn and head back
    //while(1) stop_moving();
  }
}

//    while(r_light < GREY){ // is white
//      r_light = analogRead(R_LIGHT);
//      if(r_light > GREY) { 
//        Serial.println("R_LIGHT IS GREY");
//        break;
//      }
//      turn_right();
//      Serial.println("RLIGHT IS WHITE");
//    }
//    l_light = analogRead(L_LIGHT);
//    while(l_light < GREY){ // is white
//      l_light = analogRead(L_LIGHT);
//      if(l_light > GREY) { 
//        Serial.println("L_LIGHT IS GREY");
//        stop_moving();
//        break;
//      }
//      turn_right();
//      Serial.println("L_LIGHT IS WHITE");
//    }
//}
