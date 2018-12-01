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
const int GREY = 600;
bool hat_detected = false;
// microphone 
bool start_detected = false;
byte count660 = 0;
// pin constants
const int HAT_SENSOR = A0; // TODO same as L_WALL
// pin constants new servos
const int L_LIGHT    = 2;
const int R_LIGHT    = 4;
const int M_LIGHT    = 3;
const int LEFT_PIN   = 6;
const int RIGHT_PIN  = 5;
const int STOP = 90;
const int BASE_SPEED = 5;
const int K1 = 50;
const int K2 = 30;
int l_light = 0;
int r_light = 0;
int m_light = 0;
int wait = 1000;
int turn_wait = 900;
int l_light_prev;
int r_light_prev; // new servos
//wall constants
const int R_WALL = A1;
const int L_WALL = A0;
const int F_WALL = A5;
const int WALL_THRESHOLD = 120;
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
const byte W_OFFSET = 8; // walls 
// DFS variables 
enum _direction{E, S, W, N};
const byte START_DIR = S;  // start facing East
byte prev_dir        = START_DIR;
byte dir             = START_DIR;
const int Y          = 1; // start y 
const int X          = 0; // start x 
byte pos[2]          = {X,Y};
typedef struct {
    byte node_dir; // dir robot was facing when it entered that node 
    bool visited;
} node;
//const byte ROW = 2; //5; 
//const byte COL = 3; //4;
node zero_node = {0, 0};
node maze[9][9] = {
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
    {zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node, zero_node},
};
node curr_node = maze[0][0];
//int red = 7; // right//
//int green = 1; // forward
//int yellow = 0; // left

void setup() {
    Serial.begin(9600); // use the serial port
    left.attach(LEFT_PIN);
    right.attach(RIGHT_PIN);   
    timsko = TIMSK0;
    admux = ADMUX;
    didro = DIDR0;
    adcsra = ADCSRA;
    radio_setup();
    Serial.println("setup");
//    pinMode(red, OUTPUT);
//    pinMode(green, OUTPUT);
//    pinMode(yellow, OUTPUT);
//    digitalWrite(red, LOW);
//    digitalWrite(green, LOW);
}

void loop() {
    l_light = digitalRead(L_LIGHT);
    r_light = digitalRead(R_LIGHT);
    m_light = digitalRead(M_LIGHT);
//    while(!start_detected) {
//        stop_moving();
//        check_for_start();
//        Serial.println("Started");
//    }
//    while(hat_detected) { //Slash has detected another robot
//        stop_moving();
//        delay(1000);
//        check_for_hat();
//    }
    if (l_light == 0 && r_light == 0) {
        detect_walls();
        //full_180();
        //delay(50);
    } else {
        LINE_FOLLOWING(100);
    }
}

void detect_walls(){
    stop_moving();
    prev_dir = dir;
    byte right_wall = (prev_dir + 1) % 4;
    byte left_wall  = (prev_dir + 3) % 4;
    int r_wall = analogRead(R_WALL);
    int l_wall = analogRead(L_WALL);
    int f_wall = analogRead(F_WALL);
    walls[0] = 0; // reset wall array
    walls[1] = 0;
    walls[2] = 0;
    walls[3] = 0;
    walls[right_wall] = r_wall;
    walls[left_wall]  = l_wall;
    walls[prev_dir]   = f_wall;
    // to gui
    //Serial.println (String(pos[1]));
    String _print = String(pos[1]) + ","
                   + String(pos[0]) + "," 
                   + "curr_dir" + String(prev_dir) + "," 
                   + "west="  + String(walls[W]) + "," 
                   + "north=" + String(walls[N]) + ",";
    String _print2 = "east="  + String(walls[E]) + ","
                   + "south=" + String(walls[S]);
    Serial.print (_print);
    Serial.println (_print2);
    walls[right_wall] = r_wall > WALL_THRESHOLD;
    walls[left_wall]  = l_wall > WALL_THRESHOLD;
    walls[prev_dir]   = f_wall > WALL_THRESHOLD;
    rf_crap(); // RADIO
    // to gui 
    dfs(walls[right_wall], walls[left_wall], walls[prev_dir]); // r , l, f
    if (dir == prev_dir){ 
        Serial.println("Forward");
        move_forward();
        delay(200);
        
    } else if (dir == left_wall) {
        Serial.println("Left");
        full_left();
    } else if (dir == right_wall) {
        Serial.println("Right");
        full_right();
    } else {
        Serial.println("180");
        full_180();    
    }
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
    byte right = (prev_dir + 1) % 4;
    byte left = (prev_dir + 3) % 4;
    
    byte x = pos[0]; // current node 
    byte y = pos[1];
    if (!maze[y][x].visited) { // if curr_node not visited 
        maze[y][x].node_dir = prev_dir;
        maze[y][x].visited = 1;
    }
    maze[0][0].node_dir = (START_DIR + 2) % 4; // corner case
    maze[0][0].visited = 1; // start node always starts as visited  
    curr_node = maze[y][x];

    int f_x = get_x(x, prev_dir); // pos of node in front of the current node
    int f_y = get_y(y, prev_dir);
    
    int r_x = get_x(x, right); // pos of node to the right of the current node  
    int r_y = get_y(y, right);
    
    int l_x = get_x(x, left); // pos of node to the left of the current node
    int l_y = get_y(y, left);
    
    bool r_block = (right_wall)   ? 1 : maze[r_y][r_x].visited;
    bool l_block = (left_wall)    ? 1 : maze[l_y][l_x].visited;
    bool f_block = (forward_wall) ? 1 : maze[f_y][f_x].visited;
     
    if (r_block && l_block && f_block) { // 3 sides blocked 
        dir = (curr_node.node_dir + 2) % 4; 
//        digitalWrite(red, HIGH);
//        digitalWrite(green, HIGH);
//        //digitalWrite(yellow, HIGH);
        Serial.println("back track");
    } else if (f_block && !r_block) { // front blocked and right not blocked
        dir = right;
//        digitalWrite(red, HIGH);
//        digitalWrite(green, LOW);
//        //digitalWrite(yellow, LOW);
        Serial.println("turn right");
    } else if (f_block && !l_block) { //front blocked and left not blocked
        dir = left;  
//        digitalWrite(red, LOW);
//        digitalWrite(green, HIGH);
//        //digitalWrite(yellow, HIGH);
        Serial.println("turn left"); 
    } else {
        dir = prev_dir;
//        digitalWrite(red, LOW);
//        digitalWrite(green, LOW);
//        //digitalWrite(yellow, LOW);    
        Serial.println("move forward");    
    }
    // update pos
    if (dir == N || dir == W) { // going west or north
        pos[dir % 2] --; 
    } else {                   // going east or south
        pos[dir % 2] ++;    
    }
}

void detect_treasure() {
  //to be written
}

void rf_crap() { //radio
    int temp = 0; // temp = | walls | prev_dir |
    temp |= pos[0]; // 1 byte
    temp |= pos[1] << 4; // 1 byte 
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
    radio.setPayloadSize(4); // 3 bytes
    radio.openWritingPipe(pipes[0]); // role == role_ping_out
    radio.openReadingPipe(1,pipes[1]);
    radio.startListening();   
    Serial.println("radio setup finished"); 
}

void check_for_hat() {
   
}

// new servos
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

void full_left(){
  move_forward();
  delay(300/BASE_SPEED);
  right.write(R_FORWARD(100));
  left.write(L_BACKWARD(100));
  delay(1200/BASE_SPEED);
  m_light = digitalRead(M_LIGHT);
    while(m_light == 1 && l_light == 0){ //  not on white
        m_light = digitalRead(M_LIGHT);
        l_light = digitalRead(L_LIGHT);
    }
}

void full_right(){
  move_forward();
  delay(300/BASE_SPEED);
  right.write(R_BACKWARD(100));
  left.write(L_FORWARD(100));
  delay(2000/BASE_SPEED);
  m_light = digitalRead(M_LIGHT);
    while(m_light == 1 && r_light == 0){ //  not on white
      m_light = digitalRead(M_LIGHT);
      r_light = digitalRead(R_LIGHT);
    }
//    while(1){
//        stop_moving();
//    }
}

void full_180(){
    move_forward();
    delay(200);
    full_left();
    right.write(R_FORWARD(100));
  left.write(L_BACKWARD(100));
  delay(500);
  l_light = digitalRead(L_LIGHT);
    while(l_light){ //  not on white
      l_light = digitalRead(L_LIGHT);
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

void check_for_start() {
      
}
