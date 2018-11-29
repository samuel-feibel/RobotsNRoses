#include <Wire.h>

const byte OV7670_I2C_ADDRESS = 0x21;

const byte COM3_ADDR      = 0x0C; // enable scaling
const byte COM7_ADDR      = 0x12; // reset all regs, enable color bar test
const byte COM9_ADDR      = 0x14; // AGC
const byte CLKRC_ADDR     = 0x11; // use external clock as internal clock 
const byte COM15_ADDR     = 0x40; // data format
const byte COM17_ADDR     = 0x42; // dsp color bar  
const byte MVFP_ADDR      = 0x1E ; // Vertical and mirror flip the output image 
const byte PSHFT_ADDR     = 0x1B; // set camera to output the resolution specified
const byte COM14_ADDR     = 0x3E; //const byte COM14_SET = 0x1E;
const byte SCALING_XSC    = 0x70; // test pattern

const byte RESET     = 0x80;
const byte COM3_SET  = 0x08; // scale enable (COM14 default set)
const byte COM7_SET  = 0x0C; //0x0C; // RGB output and QCIF res // 0x0E color bar
const byte COM9_SET  = 0x0B; // AGC = 2x, freeze AGC
//const byte COM9_SET  = 0x7B; // AGC = 2x, freeze AGC
const byte CLKRC_SET = 0xC0; // use external clock directly
const byte COM15_SET = 0xD0; // RGB565 0xD0; // RGB444
const byte COM17_SET = 0x08; // dsp color bar enable 
const byte MVFP_SET  = 0x30; // mirror and vflip
const byte COM14_SET = 0x08; // manual scaling enable

//RED WIRE
int color_pin = 2;
//BLUE WIRE
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

// 1 equals RED, 0 equals BLUE
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
  Wire.begin();
  OV7670_write_register(COM7_ADDR, RESET);
  read_key_registers();
  byte rgb444_val = read_register_value(0x8C);
  Serial.print("rgb444 before write ");
  Serial.println(rgb444_val, HEX);
  delay(100);
  write_key_registers();
  read_key_registers();
  set_color_matrix();
}

void detect_treasure(){
  
  //NO SHAPE
  if (shape0==0 && shape1==0 && color==0) {
    Serial.println("NO SHAPE");
    treasure_out=0;
    }
  
  //RED TRIANGLE
  if (color==1 && shape0==1 && shape1==0) {
    Serial.println("RED TRIANGLE");
     treasure_out=1;
    }
  
  //RED SQUARE
  if (color==1 && shape0==1 && shape1==1) {
    Serial.println("RED SQUARE");
     treasure_out=2;
    }
  
  //RED DIAMOND
  if (color==1 && shape0==0 && shape1==1) {
    Serial.println("RED DIAMOND");
     treasure_out=3;
    }

  //BLUE TRIANGLE
  if (color==0 && shape0==1 && shape1==0) {
    Serial.println("BLUE TRIANGLE");
     treasure_out=4;
    }
  
  //BLUE SQUARE
  if (color==0 && shape0==1 && shape1==1) {
    Serial.println("BLUE SQUARE");
     treasure_out=5;
    }
  
  //BLUE DIAMOND
  if (color==0 && shape0==0 && shape1==1) {
    Serial.println("BLUE DIAMOND");
     treasure_out=6;
   }
}

// the loop function runs over and over again forever
void loop() {
  color = digitalRead(color_pin); 
  shape0 = digitalRead(shape0_pin); 
  shape1 = digitalRead(shape1_pin); 
  //treasure_out is set depending on what kind of treasure is detected
  detect_treasure();
}
//
void write_key_registers(){
    OV7670_write_register(0x12, 0x80); 
    delay(100);  
    OV7670_write_register(0x11, 0xC0);
    OV7670_write_register(0x0C, 0x08);
    OV7670_write_register(0x12, 0x0c);
    OV7670_write_register(0x42, 0x00);
    OV7670_write_register(0x14, 0x50);  
}

void read_key_registers(){
  Serial.print("0x11: ");
  Serial.println(read_register_value(0x11), HEX);
  Serial.print("0x1e: ");
  Serial.println(read_register_value(0x1e), HEX);
  Serial.print("0x0c: ");
  Serial.println(read_register_value(0x0c), HEX);
  Serial.print("0x12: ");
  Serial.println(read_register_value(0x12), HEX);
  Serial.print("0x40: ");
  Serial.println(read_register_value(0x40), HEX);
  Serial.print("0x42: ");
  Serial.println(read_register_value(0x12), HEX);
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
    return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
