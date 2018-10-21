#include <Servo.h>
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

bool hat_detected = false;
bool start_detected = false;
int count660 = 0;
Servo left; 
Servo right;

// light sensor constants
const int GREY       = 710;
// pin constants
const int HAT_SENSOR    = A0;
const int WALL_SENSOR   = A1; // right wall sensor
const int L_WALL_SENSOR = A4; 
const int F_WALL_SENSOR = A5;
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
// sensors 
int l_light = 0;
int r_light = 0;
const int WALL_THRESHOLD = 200;
int wall_sensor = 0;
int l_wall = 0;
int f_wall = 0;
int figure_8 = 0;
//adc default constants
unsigned int timsko = 0;
unsigned int admux = 0;
unsigned int didro = 0;
unsigned int adcsra = 0;
// direction constants 
enum _direction{E, S, W, N};
int dir = E; // start facing East
int pos[2] = {0,0}; // upper left corner
int curr_pos[2] = {0,0}; 
String _print; // TODO 
int walls[4] = {0, 0, 0, 0}; // absolute direction of wall
int right_wall; 
int left_wall;
int explored[2][3] = {{0, 0, 0},{0, 0, 0}}; 
int all_explored[2][3] = {{1, 1, 1}, {1, 1, 1}};
// RF
const int Y_OFFSET = 0; // y pos
const int X_OFFSET = 4; // x pos
const int W_OFFSET = 8; // walls 
const int T_OFFSET = 12; // treasure
const int R_OFFSET = 16; // robot  
const int E_OFFSET = 17; // explored whole maze 
unsigned long temp = 0;
RF24 radio(9,10);
const uint64_t pipes[2] = { 0x00000000042LL, 0x0000000043LL };

void setup() {
    left.attach(LEFT_PIN);
    right.attach(RIGHT_PIN);
    timsko = TIMSK0;
    admux = ADMUX;
    didro = DIDR0;
    adcsra = ADCSRA;
    stop_moving(); // wait

    Serial.begin(57600); // RF
    printf_begin();
    printf("\n\rRF24/examples/GettingStarted/\n\r");
    radio.begin(); 
    radio.setRetries(15,15);
    radio.setAutoAck(true);
    radio.setChannel(0x50);
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);
    radio.setPayloadSize(4);
    radio.openWritingPipe(pipes[0]); // role == role_ping_out
    radio.openReadingPipe(1,pipes[1]);
    radio.startListening();
    stop_moving();
    start_detected = false;
}
// main function
void loop() {
    if(!start_detected){
        Serial.println("Stop");
        stop_moving();
        check_for_start();
    }
    if(start_detected) {
        Serial.println("STARTED");
        radio.stopListening(); // RF
        l_light     = analogRead(L_LIGHT);
        r_light     = analogRead(R_LIGHT);
        wall_sensor = analogRead(WALL_SENSOR); // right wall sensor 
        l_wall      = analogRead(L_WALL_SENSOR);
        f_wall      = analogRead(F_WALL_SENSOR);
        // IR
        check_for_hat();
        while(hat_detected) {  
            stop_moving();
            delay(1000); 
            check_for_hat();
        }
    
        if(l_light < GREY && r_light < GREY) { // at intersection update gui
            right_wall = (dir + 1) % 4;
            left_wall  = (dir + 3) % 4;
            //check_for_hat();
            walls[right_wall] = !(wall_sensor < WALL_THRESHOLD); 
            //walls[left_wall]  = !(l_wall < WALL_THRESHOLD);
            //walls[dir]        = !(f_wall < WALL_THRESHOLD);
            if(!explored[curr_pos[1]][curr_pos[0]]){ // update gui if reached unexplored intersection
                explored[curr_pos[1]][curr_pos[0]] = 1; // set current intersection as explored 
                // MAZE ALGO
                if(walls[right_wall]){ // detect right wall move forward
                    // dir stays the same
                } else { // turn right
                    dir = right_wall; // change dir
                } 
                // MAZE ALGO END
                if (dir == N || dir == W) { // going west or north
                    pos[dir % 2] --; 
                } else {                    // going east or south
                    pos[dir % 2] ++;    
                }  
                
                temp = 0;
                temp |= (curr_pos[1] << Y_OFFSET); // RF 
                temp |= (curr_pos[0] << X_OFFSET);
                temp |= (walls[W] << (W_OFFSET + W));
                temp |= (walls[N] << (W_OFFSET + N));
                temp |= (walls[S] << (W_OFFSET + S));
                temp |= (walls[E] << (W_OFFSET + E)); //TODO the rest of the bits
                printf("Now sending %lu...",temp);
                bool ok = radio.write( &temp, sizeof(unsigned long) );
                
                if(dir == right_wall){ // maze algo
                    auto_right();
                } else {
                    move_forward();
                }
            } else {
                if(compareArray(explored, all_explored)){
                    stop_moving();
                    while(1){
                        ;
                    } 
                } 
                if(walls[right_wall]){ // maze algo
                    move_forward();
                } else {
                    auto_right();
                }   
            }       
        } else { // exited explored intersection
            curr_pos[0] = pos[0];
            curr_pos[1] = pos[1];      
        } 
        if(l_light < GREY){ // line detection
            turn_left();  
        } else if(r_light < GREY){
            turn_right();
        } else {
            move_forward();
        }
        radio.startListening(); // RF
        //stop_moving();
        //delay(1000);    
    }
    
    
}

//check for start
void check_for_start() {
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x44; // use adc4
  DIDR0 = 0x04; // turn off the digital input for adc4
   cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if (fft_log_out[4] > 185 ){
          count660++;
          Serial.println(count660++);
    } else {
          count660 = 0;
    }
    if (count660 >= 20){
          Serial.println("detected 660Hz signal!!!!");
          start_detected = true;
    }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;
}

//check for other robots
void check_for_hat() {
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  ADCSRA = 0xe5; // set the adc to free running mode
  cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the ff
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if(fft_log_out[40] > 100 || fft_log_out[41] > 100 || fft_log_out[42] > 100 || fft_log_out[43] > 100 || fft_log_out[44] > 100 || fft_log_out[45] > 100 || fft_log_out[46] > 100)
    {hat_detected = true;}
    else {hat_detected = false;}
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;
}

String bool_str(int b){
  return b ? "true" : "false";
}

bool compareArray(int a[2][3], int b[2][3])    {
    int i, j;
    for(i = 0; i < 2; i++){ // num_rows
        for(j = 0; j < 3; j++){ // num_col 
            if (a[i][j] != b[i][j]) {
                return false;
            } 
        }
    }
    return true;
}

void move_forward(){
  left.write(L_FORWARD);
  right.write(R_FORWARD); 
  //Serial.println("MOVE FORWARD");
}

void turn_right(){
  right.write(STOP);
  left.write(L_FORWARD);
  //Serial.println("TURN RIGHT");
}

void turn_left(){
  left.write(STOP);
  right.write(R_FORWARD);
  //Serial.println("TURN LEFT");
}

void stop_moving(){
  right.write(STOP);
  left.write(STOP);
  delay(50);
  //Serial.println("STOP MOVING");  
}

void auto_right(){
    //Serial.println("AUTO RIGHT");
    move_forward();
    delay(50); // reach the center of the intersection before turning
    right.write(R_BACKWARD);
    left.write(L_FORWARD);
    delay(600);
    l_light = analogRead(L_LIGHT);
    //Serial.println(l_light);
    while(l_light > GREY){ // is grey
      l_light = analogRead(L_LIGHT);
      //Serial.println(l_light);
    }
}

void auto_left(){
    move_forward();
    delay(50);
    right.write(R_FORWARD);
    left.write(L_BACKWARD);
    delay(600);
    r_light = analogRead(R_LIGHT);
    while(r_light > GREY){ // is grey
      r_light = analogRead(R_LIGHT);
    }
    //Serial.println("AUTO LEFT");  
}
