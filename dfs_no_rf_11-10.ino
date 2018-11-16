#include <Servo.h>
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//servos
Servo left; 
Servo right;
// IR hat detector 
bool hat_detected = false;
// microphone 
bool start_detected = false;
int count660 = 0;
// light sensor constants
const int GREY       = 700;
// pin constants
const int HAT_SENSOR = A0; // TODO same as L_WALL
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
const int R_WALL = A1;
const int L_WALL = A0;
const int F_WALL = A5;
const int WALL_THRESHOLD = 100;
//adc default 
unsigned int timsko = 0;
unsigned int admux = 0;
unsigned int didro = 0;
unsigned int adcsra = 0;
//radio variables
byte walls[4] = {0, 0, 0, 0};
RF24 radio(9,10);
const uint64_t pipes[2] = { 0x00000000042LL, 0x0000000043LL };
// encoding
//const int W_BW = 4; // bitwidth
//const int T_BW = 4;
//const int R_BW = 1;
//const int E_BW = 1;  
const byte W_OFFSET = 2; // walls 
//const int T_OFFSET = W_OFFSET + W_BW; // treasure
//const int R_OFFSET = T_OFFSET + T_BW; // robot  
//const int E_OFFSET = R_OFFSET + R_BW; // explored whole maze 

// DFS variables 
enum _direction{E, S, W, N};
byte curr_dir = E;
byte dir      = E; // start facing East
byte pos[2]   = {0,0};
typedef struct {
    byte node_dir; // dir robot was facing when it entered that node 
    bool visited;
} node;
const byte ROW = 5; 
const byte COL = 4;
node zero_node = {0, 0};
node maze[ROW][COL] = {
    {zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node}     
}; // maze[x][y]
node curr_node = maze[0][0];
bool exited = 0;
// intersection detection variables 
const int thresh = 750;
//How responsive Slash is to detecting intersections, the higher the value, the less responsive.
int count_enter = 0;
//How long Slash must wait after detecting an intersection before he can detect another one, the higher the value, the longer the wait.
int count_exit = 0;
int count_thresh_enter = 250;
int count_thresh_exit = 25;
bool intersection_detect = false;

void setup() {
    Serial.begin(9600); // use the serial port
    left.attach(LEFT_PIN);
    right.attach(RIGHT_PIN);   
    timsko = TIMSK0;
    admux = ADMUX;
    didro = DIDR0;
    adcsra = ADCSRA;
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);
    // radio_setup();
    Serial.println("setup");
}

void loop() {
    l_light = analogRead(L_LIGHT);
    r_light = analogRead(R_LIGHT);
//    while(!start_detected) {
//        stop_moving();
//        check_for_start();
//        Serial.println("Started");
//    }
//    Serial.println("PIN 12:");
//    int value = digitalRead(12);
//    Serial.println(value);
//    while(hat_detected) { //Slash has detected another robot
//        stop_moving();
//        delay(1000);
//        check_for_hat();
//    }
//    if (l_light < GREY && r_light < GREY) {
//        if(!exited){
//            detect_walls();
//            exited = 1;
//        } 
//    }

//    detect_intersection();
//    if (intersection_detect){
//        detect_walls();
    if (l_light < GREY && r_light < GREY) {
        detect_walls();
    } else if(l_light < GREY){ 
        exited = 0;
        nudge_left();  
    } else if(r_light < GREY){
        exited = 0;
        nudge_right();
    } else {
        exited = 0;
        move_forward();
    }
    
}

bool detect_intersection(){
    l_light = analogRead(L_LIGHT);
    r_light = analogRead(R_LIGHT);
    
    //Detect the possibility of an intersection
    if (l_light<thresh && r_light<thresh){
        count_enter++;
        
        //Detecting a real intersection
        if (count_enter > count_thresh_enter && count_exit>count_thresh_exit){
            //Serial.println("Intersection Detected");
            intersection_detect=true;
            //Must start waiting before we can detect the next
            count_exit=0;
        }
    }
    //No intersection means that the timer is running to allow Slash to detect another intersection. 
    else {
        //Serial.println("No Intersection Detected");
        intersection_detect=false;
        count_enter=0;
        count_exit++;
        //Serial.println(count_exit);
    }
}

void detect_walls(){
    byte right_wall = (curr_dir + 1) % 4;
    byte left_wall  = (curr_dir + 3) % 4;
    int r_wall = analogRead(R_WALL);
    int l_wall = analogRead(L_WALL);
    int f_wall = analogRead(F_WALL);
    walls[0] = 0; // reset wall array
    walls[1] = 0;
    walls[2] = 0;
    walls[3] = 0;
    walls[right_wall] = r_wall > WALL_THRESHOLD;
    walls[left_wall]  = l_wall > WALL_THRESHOLD;
    walls[curr_dir]   = f_wall > WALL_THRESHOLD;
    // to gui
    String _print = String(pos[1]) + ","
                   + String(pos[0]) + "," 
                   + "west="  + bool_str(walls[W]) + ","
                   + "north=" + bool_str(walls[N]) + ","
                   + "east="  + bool_str(walls[E]) + ","
                   + "south=" + bool_str(walls[S]);
    Serial.println(_print);
    if(walls[right_wall] && walls[left_wall] && walls[dir]){
        Serial.println("turn around");
        auto_180();
        curr_dir = curr_dir + 2 % 4;
    } else if (!walls[right_wall] && walls[dir]) {
        Serial.println("turn right");
        auto_right();
        curr_dir = right_wall;
    } else if (!walls[left_wall] && walls[dir]) {
        Serial.println("turn left");
        auto_left();
        curr_dir = left_wall;
    } else {
        Serial.println("move forward");
        move_forward();
        delay(200);
    }
    // to gui 
//    curr_dir = dir;
//    dfs(walls[right_wall], walls[left_wall], walls[dir]); // r , l, f
//    if (dir == curr_dir){ // if curr_dir == next dir
//        move_forward();
//        delay(200);
//    } else if (dir == left_wall) {
//        auto_left();
//    } else if (dir == right_wall) {
//        auto_right();
//    } else {
//        auto_180();    
//    }
}

String bool_str(int b){
  return b ? "true" : "false";
}

int get_x(int x, int d){ // dfs helper function
    if (d == W) { // going west
        return x - 1; 
    } else if (d == E) { // going east                   
        return x + 1;    
    } else {
        return x;    
    }
}

int get_y(int y, int d){ // dfs helper function
    if (d == N) { // going north 
        return y - 1; 
    } else if (d == S){ // going south                 
        return y + 1;    
    } else {
        return y;
    }
}


void dfs(int right_wall, int left_wall, int forward_wall) {  
    byte right = (dir + 1) % 4;
    byte left = (dir + 3) % 4;
    
    byte x = pos[0]; // current node 
    byte y = pos[1];
    if (!maze[y][x].visited) { // if curr_node not visited 
        maze[y][x].node_dir = dir;
        maze[y][x].visited = 1;
    }
    maze[0][0].node_dir = W; // corner case 
    curr_node = maze[y][x];

    int f_x = get_x(x, dir); // pos of node in front of the current node
    int f_y = get_y(y, dir);
    
    int r_x = get_x(x, right); // pos of node to the right of the current node  
    int r_y = get_y(y, right);
    
    int l_x = get_x(x, left); // pos of node to the left of the current node
    int l_y = get_y(y, left);
    
    bool r_block = (right_wall)   ? 1 : maze[r_y][r_x].visited;
    bool l_block = (left_wall)    ? 1 : maze[l_y][l_x].visited;
    bool f_block = (forward_wall) ? 1 : maze[f_y][f_x].visited;
     
    if (r_block && l_block && f_block) { // 3 sides blocked 
        dir = (curr_node.node_dir + 2) % 4; 
        Serial.println("back track");
    } else if (f_block && !r_block) { // front blocked and right not blocked
        dir = right;
        Serial.println("turn right");
    } else if (f_block && !l_block) { //front blocked and left not blocked
        dir = left;  
        Serial.println("turn left"); 
    } else {
        Serial.println("move forward");    
    }
    // update pos
    if (dir == N || dir == W) { // going west or north
        pos[dir % 2] --; 
    } else {                   // going east or south
        pos[dir % 2] ++;    
    }
}

void dfs_setup() {
    curr_node = {dir, 1};
    for(int i = 0;  i < ROW;  i++) {
        for(int j = 0; j < COL; i++) {
            maze[i][j] = {0, 0};
        }
    }
}

void detect_treasure() {
  //to be written
}

void rf_transmission() { //radio
    int temp = 0; // temp = | walls | dir |
    temp |= dir;
    temp |= (walls[W] << (W_OFFSET + W));
    temp |= (walls[N] << (W_OFFSET + N));
    temp |= (walls[S] << (W_OFFSET + S));
    temp |= (walls[E] << (W_OFFSET + E)); //TODO the rest of the bits
    radio.stopListening();
    bool ok = radio.write( &temp, sizeof(unsigned long) );
    radio.startListening();
}

void radio_setup() {
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

void check_for_hat() {
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
    fft_reorder(); // reorder the data before doing the ff
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    Serial.println(fft_log_out[42]);
    sei();
    Serial.println("HAT VARIABLE:");
    Serial.println(hat_detected);
    if(fft_log_out[40] > 100 || fft_log_out[41] > 100 || fft_log_out[42] > 100 || fft_log_out[43] > 100 || fft_log_out[44] > 100 || fft_log_out[45] > 100 || fft_log_out[46] > 100) {
        hat_detected = true;
    } else {
        hat_detected = false;
    }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;
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
    stop_moving();
}

void auto_180(){
    r_light = analogRead(R_LIGHT);
    move_forward();
    delay(150);
    turn_right();
    delay(1250);

    l_light = analogRead(L_LIGHT);
    while(l_light > GREY){   // is grey
        l_light = analogRead(L_LIGHT);
        turn_right();
    }
}

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
          digitalWrite(12,HIGH);
    }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;   
}
