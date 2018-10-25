#include <Servo.h>
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
//servos
Servo left; 
Servo right;
//IR hat detector constants
bool hat_detected = false;
//microphone constants
bool start_detected = false; 
int count660 = 0;
// light sensor constants
const int GREY       = 500;
// pin constants
const int HAT_SENSOR = A0;
const int R_WALL = A1;
const int L_WALL = 0;
const int F_WALL = 0;
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
//line following
int l_light = 0;
int r_light = 0;
//wall constants
const int WALL_THRESHOLD = 200;
int r_wall = 0;
int l_wall = 0;
int f_wall = 0;
int wait = 1000;
int turn_wait = 900;
//adc default constants
unsigned int timsko = 0;
unsigned int admux = 0;
unsigned int didro = 0;
unsigned int adcsra = 0;
//radio variables
enum _direction{E, S, W, N};
int dir = E; // start facing East
int walls[4] = {0, 0, 0, 0}; // absolute direction of wall
int right_wall; 
int left_wall;
int explored = 0;
RF24 radio(9,10);
const uint64_t pipes[2] = { 0x00000000042LL, 0x0000000043LL };
// encoding
const int W_BW = 4; // bitwidth
const int T_BW = 4;
//const int R_BW = 1;
//const int E_BW = 1;  
const int W_OFFSET = 2; // walls 
//const int T_OFFSET = W_OFFSET + W_BW; // treasure
//const int R_OFFSET = T_OFFSET + T_BW; // robot  
//const int E_OFFSET = R_OFFSET + R_BW; // explored whole maze 
int temp = 0;
// temp = | walls | dir |

void setup() {
  Serial.begin(9600); // use the serial port
  left.attach(LEFT_PIN);
  right.attach(RIGHT_PIN);
  timsko = TIMSK0;
  admux = ADMUX;
  didro = DIDR0;
  adcsra = ADCSRA;

    // RF setup 
    printf_begin();
    printf("\n\rRF24/examples/GettingStarted/\n\r");
    radio.begin(); 
    radio.setRetries(15,15);
    radio.setAutoAck(true);
    radio.setChannel(0x50);
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);
    radio.setPayloadSize(2);
    radio.openWritingPipe(pipes[0]); // role == role_ping_out
    radio.openReadingPipe(1,pipes[1]);
    radio.startListening();
}
// main function
void loop() {
  check_for_start();
  Serial.println(start_detected);
  if(start_detected){
    Serial.println("STARTED");
    check_for_hat();
    while(hat_detected) { //Slash has detected another robot
      stop_moving();
      delay(1000);
      check_for_hat();
    }
    if(l_light < GREY && r_light < GREY) { //Slash has reached an intersection
      detect_treasure();
      detect_walls();
      if (explored == 0){
        rf_transmission();
        explored = 1;
      }   
    } else { //line following
      explored = 0;
      follow_that_shit();
    }
    
  } else { 
    stop_moving();
    Serial.println("Should be stopped");
  }
}

void detect_treasure() {
  //to be written
}
//wall detection
void detect_walls() {
  right_wall = (dir + 1) % 4;
  left_wall  = (dir + 3) % 4;
  l_light = analogRead(L_LIGHT);
  r_light = analogRead(R_LIGHT);
  r_wall = analogRead(R_WALL);
  l_wall = analogRead(L_WALL);
  f_wall = analogRead(F_WALL);
  walls[right_wall] = !(r_wall < WALL_THRESHOLD);
  walls[left_wall]  = !(l_wall < WALL_THRESHOLD);
  walls[dir]        = !(f_wall < WALL_THRESHOLD);
  if(r_wall < WALL_THRESHOLD) {
    dir = right_wall;
    auto_right(); //turn right if possible
  } else if (l_wall < WALL_THRESHOLD) {
    dir = left_wall;
    auto_left(); //if can't turn right, turn left if possible
  } else if (f_wall < WALL_THRESHOLD) {
    move_forward(); //if can't turn right or left, move forward if possible
  } else {
    dir = (dir + 2) % 4;
    //auto_180(); //else it's a dead end, make a u-turn and head back
  }
}
//radio transmission 
void rf_transmission() {
    
    temp = 0;
    temp |= dir;
    temp |= (walls[W] << (W_OFFSET + W));
    temp |= (walls[N] << (W_OFFSET + N));
    temp |= (walls[S] << (W_OFFSET + S));
    temp |= (walls[E] << (W_OFFSET + E)); //TODO the rest of the bits
    printf("Now sending %lu...",temp);
    radio.stopListening();
    bool ok = radio.write( &temp, sizeof(unsigned long) );
    radio.startListening();
}

//line following
void follow_that_shit() {
  l_light = analogRead(L_LIGHT);
  r_light = analogRead(R_LIGHT);
  if(l_light < GREY){ 
      turn_left();  
  } else if(r_light < GREY){
      turn_right();
  } else {
      move_forward();
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
    if (count660 >= 10){
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

void move_forward(){
  left.write(L_FORWARD);
  right.write(R_FORWARD); 
  Serial.println("MOVE FORWARD");
}

void turn_right(){
  right.write(R_BACKWARD);
  left.write(L_FORWARD);
  Serial.println("TURN RIGHT");
}

void turn_left(){
  left.write(L_BACKWARD);
  right.write(R_FORWARD);
  Serial.println("TURN LEFT");
}

void stop_moving(){
  right.write(STOP);
  left.write(STOP);
  Serial.println("STOP MOVING");  
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
}
