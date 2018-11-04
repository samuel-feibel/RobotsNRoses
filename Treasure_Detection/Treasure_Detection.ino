/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/
//RED WIRE
int color_pin = 2;
//GREEN WIRE
int shape1_pin = 12;
//YELLOW WIRE
int shape0_pin = 8;

// [shape1, shape0] =
// [0,0]---> No Shape
// [0,1]---> Triangle
// [1,0]---> Square
// [1,1]---> Diamond
int shape0;
int shape1;

// 0 equals RED, 1 equals BLUE
int color;

// 0--> NO SHAPE
// 1--> RED TRIANGLE
// 2--> RED SQUARE
// 3--> RED DIAMOND
// 4--> BLUE TRIANGLE
// 5--> BLUE SQUARE
// 6--> BLUE DIAMOND
int treasure_out;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600); // read analog input
  pinMode(color_pin, INPUT);
  pinMode(shape1_pin, INPUT);
  pinMode(shape0_pin, INPUT);
}

void detect_treasure(){
  
  //NO SHAPE
  if (shape0==0 && shape1==0) {
    Serial.println("NO SHAPE");
    treasure_out=0;
    }
  
  //RED TRIANGLE
  if (color==0 && shape0==1 && shape1==0) {
    Serial.println("RED TRIANGLE");
     treasure_out=1;
    }
  
  //RED SQUARE
  if (color==0 && shape0==0 && shape1==1) {
    Serial.println("RED SQUARE");
     treasure_out=2;
    }
  
  //RED DIAMOND
  if (color==0 && shape0==1 && shape1==1) {
    Serial.println("RED DIAMOND");
     treasure_out=3;
    }

  //BLUE TRIANGLE
  if (color==1 && shape0==1 && shape1==0) {
    Serial.println("BLUE TRIANGLE");
     treasure_out=4;
    }
  
  //BLUE SQUARE
  if (color==1 && shape0==0 && shape1==1) {
    Serial.println("BLUE SQUARE");
     treasure_out=5;
    }
  
  //BLUE DIAMOND
  if (color==1 && shape0==1 && shape1==1) {
    Serial.println("BLUE DIAMOND");
     treasure_out=6;
    }
}

// the loop function runs over and over again forever
void loop() {
  shape0 = digitalRead(shape0_pin);
  shape1 = digitalRead(shape1_pin);
  color = digitalRead(color_pin);

  //treasure_out is set depending on what kind of treasure is detected
  detect_treasure();
  

}
