#include <Wire.h>

const byte OV7670_I2C_ADDRESS = 0x21;

const byte COM3_ADDR      = 0x0C; // enable scaling
const byte COM7_ADDR      = 0x12; // reset all regs, enable color bar test
const byte COM9_ADDR      = 0x14; // AGC
const byte CLKRC_ADDR     = 0x11; // use external clock as internal clock 
const byte COM15_ADDR     = 0x40; // scales PCLK disable internal PLL
const byte COM17_ADDR     = 0x42; // enable color bar test 
const byte MVFP_ADDR      = 0x1E ; // Vertical and mirror flip the output image 
const byte PSHFT_ADDR     = 0x1B; // set camera to output the resolution specified
const byte COM14_ADDR     = 0x3E; //const byte COM14_SET = 0x1E;
const byte SCALING_XSC    = 0x70; // test pattern
const byte RESET          = 0x80;

const byte COM3_SET  = 0x08; // scale enable (COM14 default set)
const byte COM7_SET  = 0x0E; //0x0C; // RGB output and QCIF res
const byte COM9_SET  = 0x0B; // AGC = 2x, freeze AGC
const byte CLKRC_SET = 0xC0; // use external clock directly
const byte COM15_SET = 0xD0; // RGB565
const byte COM17_SET = 0x08; // color bar enable  
const byte MVFP_SET  = 0x30; // mirror and vflip
const byte COM14_SET = 0x08; // manual scaling enable

void setup() {
  Wire.begin();
  Serial.begin(9600);
  OV7670_write_register(COM7_ADDR, RESET);
  read_key_registers();
  delay(100);
  write_key_registers();
  read_key_registers();
  set_color_matrix();
}
void loop(){
    
}
void write_key_registers(){
    OV7670_write_register(COM7_ADDR, RESET);
    
    OV7670_write_register(COM3_ADDR, COM3_SET);
    OV7670_write_register(COM7_ADDR, COM7_SET);
    OV7670_write_register(COM9_ADDR, COM9_SET);
    OV7670_write_register(CLKRC_ADDR, CLKRC_SET);
    OV7670_write_register(COM15_ADDR, COM15_SET);
    OV7670_write_register(COM17_ADDR, COM17_SET);
    OV7670_write_register(MVFP_ADDR, MVFP_SET);
    //OV7670_write_register(PSHFT_ADDR, PSHFT_SET);
    OV7670_write_register(SCALING_XSC, 0xCA);
    OV7670_write_register(COM14_ADDR, COM14_SET);
    
}

void read_key_registers(){
  byte com3_val = read_register_value(COM3_ADDR);
  Serial.print("COM3 ");
  Serial.println(com3_val, HEX);
  byte com7_val = read_register_value(COM7_ADDR);
  Serial.print("COM7 ");
  Serial.println(com7_val, HEX);
  byte com15_val = read_register_value(COM15_ADDR);
  Serial.print("COM15 ");
  Serial.println(com15_val, HEX);
  byte clkrc_val = read_register_value(CLKRC_ADDR);
  Serial.print("CLKRC ");
  Serial.println(clkrc_val, HEX);
  byte pshft_val = read_register_value(PSHFT_ADDR);
  Serial.print("PSHFT ");
  Serial.println(pshft_val, HEX);
  byte com17_val = read_register_value(COM17_ADDR);
  Serial.print("COM17 ");
  Serial.println(com17_val,HEX);
  byte mvfp_val = read_register_value(MVFP_ADDR);
  Serial.print("MVFP ");
  Serial.println(mvfp_val, HEX);
  byte com14_val = read_register_value(COM14_ADDR);
  Serial.print("COM14 ");
  Serial.println(com14_val, HEX);
  Serial.println("");
  byte com9_val = read_register_value(COM9_ADDR);
  Serial.print("COM9 ");
  Serial.println(com9_val, HEX);
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
