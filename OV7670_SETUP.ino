#include <Wire.h>

const byte OV7670_I2C_ADDRESS = 0x21; /*TODO: write this in hex (eg. 0xAB) */
const byte COM7_ADDR      = 0x12; // reset all regs enable color bar test 
const byte COM14_ADDR     = 0x3E;
const byte CLKRC_ADDR     = 0x11; // 
const byte PSHIFT_ADDR    = 0x1B; //
const byte COM17_ADDR     = 0x42; //
const byte MVFP_ADDR      = 0x1E ; //
const byte COM9_ADDR      = 0x14; //

const byte RESET = 0b10000000;
//const byte COM14_SET

byte com7_val = 0;
byte com14_val = 0;
byte clkrc_val = 0;
byte pshift_val = 0;
byte com17_val = 0;
byte mvfp_val = 0;
byte com9_val = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  // TODO: READ KEY REGISTERS
  read_key_registers();
  delay(100);
  // TODO: WRITE KEY REGISTERS
  OV7670_write_register(COM14_ADDR, 0xFF); //COM14_SET
//  OV7670_write_register(CLKRC_ADDR, CLKRC_SET);
//  OV7670_write_register(PSHIFT_ADDR, PSHIFT_SET);
//  OV7670_write_register(COM17_ADDR, COM17_SET);
//  OV7670_write_register(MVFP_ADDR, MVFP_SET);
//  OV7670_write_register(COM9_ADDR, COM9_SET);
  
  read_key_registers();
  OV7670_write_register(COM7_ADDR, RESET);
  read_key_registers();
}

void loop(){
    //read_key_registers();
}

void read_key_registers(){
  byte com7_val;
  com7_val = read_register_value(COM7_ADDR);
  Serial.print("COM7 ");
  Serial.println(com7_val);
  com14_val = read_register_value(COM14_ADDR);
  Serial.print("COM14 ");
  Serial.println(com14_val);
  clkrc_val = read_register_value(CLKRC_ADDR);
  Serial.print("CLKRC ");
  Serial.println(clkrc_val);
  pshift_val = read_register_value(PSHIFT_ADDR);
  Serial.print("PSHIFT ");
  Serial.println(pshift_val);
  com17_val = read_register_value(COM17_ADDR);
  Serial.print("COM17 ");
  Serial.println(com17_val);
  mvfp_val = read_register_value(MVFP_ADDR);
  Serial.print("MVFP ");
  Serial.println(mvfp_val);
  com9_val = read_register_value(COM9_ADDR);
  Serial.print("COM9 ");
  Serial.println(com9_val);
  Serial.println("");
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
