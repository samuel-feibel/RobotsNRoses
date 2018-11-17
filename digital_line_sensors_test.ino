#include <Servo.h>
//servos
Servo left; 
Servo right;
// pins 
const int L_LIGHT    = 2;
const int R_LIGHT    = 3;
const int LEFT_PIN   = 6;
const int RIGHT_PIN  = 5;
// speed constants 
const int STOP       = 90;
const int R_FORWARD  = 0;
const int L_FORWARD  = 180;
const int R_BACKWARD = 180;
const int L_BACKWARD = 0;
//line following
int l_light = 0;
int r_light = 0;


void setup() {
    Serial.begin(9600); // use the serial port
    left.attach(LEFT_PIN);
    right.attach(RIGHT_PIN);
    Serial.println("setup");
}

void loop() {
    l_light = !digitalRead(L_LIGHT); // line detected if 0
    r_light = !digitalRead(R_LIGHT);  
    //stop_moving();
    if (l_light && r_light) {
        auto_180();
        //move_forward();
    } else if(l_light){ 
        nudge_left();  
    } else if(r_light){
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
  delay(50);
}

void auto_right(){ 
    r_light = !digitalRead(R_LIGHT);
    move_forward();
    delay(300);
    turn_right();
    delay(650);

    l_light = !digitalRead(L_LIGHT);
    while(l_light){   
        l_light = !digitalRead(L_LIGHT);
        turn_right();
    }
}

void auto_left(){
    l_light = !digitalRead(L_LIGHT);
    move_forward();
    delay(300);
    turn_left();
    delay(650);

    r_light = !digitalRead(R_LIGHT);
    while(l_light){   
        l_light = !digitalRead(L_LIGHT);
        turn_left();
    }
    stop_moving();
}

void auto_180(){
    r_light = !digitalRead(R_LIGHT);
    move_forward();
    delay(150);
    turn_right();
    delay(1500);

    l_light = !digitalRead(L_LIGHT);
    while(l_light){   
        l_light = !digitalRead(L_LIGHT);
        turn_right();
    }
}
