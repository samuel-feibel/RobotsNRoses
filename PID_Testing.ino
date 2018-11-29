#include <Servo.h>

Servo left; 
Servo right;

// light sensor constants
const int GREY       = 600;
// pin constants
const int L_LIGHT    = 2;
const int R_LIGHT    = 3;
const int M_LIGHT    = 0;
const int LEFT_PIN   = 6;
const int RIGHT_PIN  = 5;
const int STOP = 90;
const int BASE_SPEED = 6;
const int K1 = 50;
const int K2 = 30;
int l_light = 0;
int r_light = 0;
int m_light = 0;
int wait = 1000;
int turn_wait = 900;

void setup() {
  Serial.begin(9600); // read analog input
  left.attach(LEFT_PIN);
  right.attach(RIGHT_PIN); 


}
// main function
void loop() { 
  l_light = digitalRead(L_LIGHT);
  r_light = digitalRead(R_LIGHT);
  m_light = digitalRead(M_LIGHT);

  if (l_light == 0 && r_light == 0) {
      full_right();

  }

  LINE_FOLLOWING(100);
  
}

void move_forward(){
  left.write(L_FORWARD(100));
  right.write(R_FORWARD(100)); 
}

void turn_right(){
  left.write(L_FORWARD(100));
  right.write(STOP);
}

void turn_left(){
  left.write(STOP);
  right.write(R_FORWARD(100));
}

void stop_moving(){
  left.write(STOP);
  right.write(STOP);  
}


void full_right(){
  move_forward();
  delay(300/BASE_SPEED);
  right.write(R_FORWARD(100));
  left.write(L_BACKWARD(100));
  delay(1200/BASE_SPEED);
  m_light = digitalRead(M_LIGHT);
    while(m_light == 1){ //  not on white
      if(m_light == 0) { 
        break;
      }
      m_light = digitalRead(M_LIGHT);
    }
}


int R_FORWARD(int K){
    return 90*(100-K*BASE_SPEED/100)/100;
}


int L_FORWARD(int K){
    return 90+90*(K/100)*BASE_SPEED/(100);
}

int L_BACKWARD(int K){
    return 90*(100-K*BASE_SPEED/100)/100;
}


int R_BACKWARD(int K){
   return 90+90*(K/100)*BASE_SPEED/100;
}

void LINE_FOLLOWING(int TEMP_SPEED){
  l_light = digitalRead(L_LIGHT);
  r_light = digitalRead(R_LIGHT);
  m_light = digitalRead(M_LIGHT);
  
  //Line Following
  if(m_light ==0 && l_light == 0){ 
    left.write(L_FORWARD(K1*TEMP_SPEED/100));
    right.write(R_FORWARD(100*TEMP_SPEED/100));  
  }else if(m_light ==0 && r_light == 0){ 
    right.write(R_FORWARD(K1*TEMP_SPEED/100));
    left.write(L_FORWARD(100*TEMP_SPEED/100));  
  } else if(l_light ==0){ 
    left.write(L_FORWARD(K2*TEMP_SPEED/100));
    right.write(R_FORWARD(100*TEMP_SPEED/100));  
  } else if(r_light ==0){
    right.write(R_FORWARD(K2*TEMP_SPEED/100));
    left.write(L_FORWARD(100*TEMP_SPEED/100));
  } else if(m_light ==0){
    right.write(R_FORWARD(100*TEMP_SPEED/100));
    left.write(L_FORWARD(100*TEMP_SPEED/100));
  }else {
    stop_moving();
  }
}

