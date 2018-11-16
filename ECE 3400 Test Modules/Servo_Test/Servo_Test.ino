#include <Servo.h>

 Servo left; 
 Servo right;
 const int LEFT_PIN   = 6;
 const int RIGHT_PIN  = 5;
 const int STOP       = 90;
 const int R_FORWARD  = 0;
 const int L_FORWARD  = 180;
 const int R_BACKWARD = 180;
 const int L_BACKWARD = 0;
 
void setup() {
  Serial.begin(9600); // read analog input
  left.attach(LEFT_PIN);
  right.attach(RIGHT_PIN);
}

void loop() {
 move_forward();
 delay(5000);
 turn_right();
 delay(5000);
 turn_left();
 delay(5000);
 stop_moving();
 delay(5000);
 

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
void stop_moving(){
  left.write(STOP);
  right.write(STOP);  
}
