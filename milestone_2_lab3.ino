#include <Servo.h>
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

bool hat_detected = false;
bool start_detected = false;
int count660 = 0;
Servo left; 
Servo right;

// light sensor constants
const int GREY       = 500;
// pin constants
const int HAT_SENSOR = A0;
const int WALL_SENSOR = A1;
const int L_LIGHT    = A2;
const int R_LIGHT    = A3;
const int LEFT_PIN   = 11;
const int RIGHT_PIN  = 9;
// speed constants 
const int STOP       = 90;
const int R_FORWARD  = 0;
const int L_FORWARD  = 180;
const int R_BACKWARD = 180;
const int L_BACKWARD = 0;
int l_light = 0;
int r_light = 0;
int wall_threshold = 200;
int wall_sensor = 0;
int figure_8 = 0;
int wait = 1000;
int turn_wait = 900;
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
int walls[4] = {0, 0, 0, 0};
int right_wall;
int left_wall;

void setup() {
  Serial.begin(9600); // use the serial port
  left.attach(LEFT_PIN);
  right.attach(RIGHT_PIN);
  timsko = TIMSK0;
  admux = ADMUX;
  didro = DIDR0;
  adcsra = ADCSRA;
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}
// main function
void loop() {
  check_for_start();
  Serial.println(start_detected);
  if(start_detected){ 
    //Serial.println("STARTED");
    l_light = analogRead(L_LIGHT);
    r_light = analogRead(R_LIGHT);
    wall_sensor = analogRead(WALL_SENSOR);
    check_for_hat();
    while(hat_detected) {
      stop_moving();
      delay(1000);
      check_for_hat();
    }
    if(l_light < GREY && r_light < GREY) { // at intersection update gui
      right_wall = (dir + 1) % 4;
      left_wall  = (dir + 3) % 4;
      if(wall_sensor < wall_threshold){
        walls[right_wall] = 0;
        pos[(dir + 1) % 2] ++; // turn right
        //auto_right();
      } else {
        walls[right_wall] = 1; // right wall detected
        //move_forward();
        pos[dir % 2] ++; // move forward
        delay(250);
      }
      _print = String(curr_pos[1]) + ","
             + String(curr_pos[0]) + "," 
             + "west="  + bool_str(walls[W]) + ","
             + "north=" + bool_str(walls[N]) + ","
             + "east="  + bool_str(walls[E]) + ","
             + "south=" + bool_str(walls[S]);
      delay(1000); // TODO
      if (curr_pos[0] < 3) {
        Serial.println(_print);
        curr_pos[0] = pos[0];
        curr_pos[1] = pos[1];
      } else {
        Serial.println("reset");
        curr_pos[0] = 0;
        pos[0] = 0;
      }
      if(walls[right_wall]){
        move_forward();
      } else {
        auto_right();
      }
    } 
     if(l_light < GREY){ 
      turn_left();  
    } else if(r_light < GREY){
      turn_right();
    } else {
      move_forward();
    }
  } else { 
    stop_moving();
    Serial.println("Should be stopped");
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

String bool_str(int b){
  return b ? "true" : "false";
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
  //Serial.println("STOP MOVING");  
}

void auto_right(){
    turn_right();
    delay(1200);
    l_light = analogRead(L_LIGHT);
    
    while(l_light > GREY){ // is grey
      l_light = analogRead(L_LIGHT);
      turn_right();
    }
    //Serial.println("AUTO RIGHT");
}

void auto_left(){
    turn_left();
    delay(1200);
    r_light = analogRead(R_LIGHT);
    
    while(r_light > GREY){ // is grey
      r_light = analogRead(R_LIGHT);
      turn_left();
    }
    //Serial.println("AUTO LEFT");  
}
