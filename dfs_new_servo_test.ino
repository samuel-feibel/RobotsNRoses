#include <Servo.h>
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
// Camera
int color;
int shape0;
int shape1;
// Camera done
//servos
Servo left; 
Servo right;
// pin constants new servos
int l_light_prev;
int r_light_prev;
const byte L_LIGHT    = 2;
const byte R_LIGHT    = 4;
const byte M_LIGHT    = 3;
const byte LEFT_PIN   = 6;
const byte RIGHT_PIN  = 5;
const int STOP = 90;
const byte BASE_SPEED = 6; // between 5 and 10, havent tried pass 7
const int K1 = 50;
const int K2 = 30;
int l_light = 0;
int r_light = 0;
int m_light = 0;
int wait = 1000;
int turn_wait = 900;
//wall constants
const int R_WALL = A1;
const int L_WALL = A2;
const int F_WALL = A5;
//const int WALL_THRESHOLD = 500; //TODO
const int R_WALL_THRESHOLD = 150;
const int L_WALL_THRESHOLD = 160;
const int F_WALL_THRESHOLD = 130;

//adc default 
unsigned int timsko = 0;
unsigned int admux = 0;
unsigned int didro = 0;
unsigned int adcsra = 0;
int count660 = 0;
int emergency_button_thresh= 900;
int button_val;
bool start_detected= false;
bool hat_detected=false;
bool button_press=false;
const int EMERGENCY_BUTTON = A3;
const int IR_AUDIO_SENSOR = A4;
//IR 1, AUDIO 0
const int MUX_CONTROL_PIN = 7;

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

void setup() {
    Serial.begin(9600); // use the serial port
    left.attach(LEFT_PIN);
    right.attach(RIGHT_PIN);   
    timsko = TIMSK0;
    admux = ADMUX;
    didro = DIDR0;
    adcsra = ADCSRA;
    pinMode(MUX_CONTROL_PIN, OUTPUT); // mux setup
    digitalWrite(MUX_CONTROL_PIN, LOW); // start with audio
    radio_setup();
    Serial.println("setup");
    while(!start_detected) {
        stop_moving();
        check_for_start();
        Serial.println("checking for start");
        Serial.println(start_detected);
    }
    Serial.println("running");
    digitalWrite(MUX_CONTROL_PIN, HIGH); // start with audio
}

void loop() {
//    while(1){
//        stop_moving();    
//    }
    l_light = digitalRead(L_LIGHT);
    r_light = digitalRead(R_LIGHT);
    m_light = digitalRead(M_LIGHT);
//    while(!start_detected) {
//        stop_moving();
//        check_for_start();
//        Serial.println("checking for start");
//    }
//    Serial.println("running");
//    digitalWrite(MUX_CONTROL_PIN, HIGH); // start with audio
    check_for_hat();
    while(hat_detected) { //Slash has detected another robot
        stop_moving();
        //Serial.println("found hat");
        //full_180();
        check_for_hat();
        //Serial.println("loop check for hat");
    }
    if (l_light == 0 && r_light == 0) {
        detect_walls();
    } else {
        LINE_FOLLOWING(100);
    }
}

void detect_walls(){
    stop_moving();
    prev_dir = dir;
    byte right_wall = (prev_dir + 1) % 4;
    byte left_wall  = (prev_dir + 3) % 4;
    int r_wall = 0;
    int l_wall = 0;
    int f_wall = 0;
    for (int i = 0; i < 10; i++){
        r_wall = r_wall + analogRead(R_WALL);
        f_wall = f_wall + analogRead(F_WALL);
        l_wall = l_wall + analogRead(L_WALL);
        //Serial.println(f_wall);
    }

    r_wall = r_wall/10;
    f_wall = f_wall/10;
    l_wall = l_wall/10;
    //Serial.println("avg ");
    //Serial.println(f_wall);
    //while(1){
    //    stop_moving();
    //}
//    int r_wall = analogRead(R_WALL);
//    int l_wall = analogRead(L_WALL);
//    int f_wall = analogRead(F_WALL);
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
    walls[right_wall] = r_wall > R_WALL_THRESHOLD;
    walls[left_wall]  = l_wall > L_WALL_THRESHOLD;
    walls[prev_dir]   = f_wall > F_WALL_THRESHOLD;
//    if (walls[right_wall]){
//        detect_treasure(); // TODO 
//    }
    rf_crap(); // RADIO
    // to gui 
    dfs(walls[right_wall], walls[left_wall], walls[prev_dir]); // r , l, f
    //Serial.println(walls[left_wall]);

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

    r_block = (r_x < 0 || r_x > 8 || r_y < 0 || r_y > 8) ? 1 : r_block;
    l_block = (l_x < 0 || l_x > 8 || l_y < 0 || l_y > 8) ? 1 : l_block;
    f_block = (f_x < 0 || f_x > 8 || f_y < 0 || f_y > 8) ? 1 : f_block;

    check_for_hat(); // check for other robots 
    f_block = (hat_detected) ? 1 : f_block; // if hat detected, front is blocked
     
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

void detect_treasure() { // Camera
    shape0 = digitalRead(8);
    shape1 = analogRead(A0);
    //Serial.println(shape1);
    if (shape1 < 500){
        shape1 = 0;
    } else {
        shape1 = 1;
    }
}

void rf_crap() { //radio
    int temp = 0; // temp = | walls | prev_dir |
    temp |= pos[0]; // 1 byte
    temp |= pos[1] << 4; // 1 byte 
    temp |= (walls[W] << (W_OFFSET + W));
    temp |= (walls[N] << (W_OFFSET + N));
    temp |= (walls[S] << (W_OFFSET + S));
    temp |= (walls[E] << (W_OFFSET + E)); //TODO the rest of the bits
    // Camera
//    temp |= shape0 << (W_OFFSET + 4);
//    temp |= shape1 << (W_OFFSET + 5);
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
    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADMUX = 0x44; // use adc0
    DIDR0 = 0x04; // turn off the digital input for adc0
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
    if(fft_log_out[40] > 100 || fft_log_out[41] > 100 || fft_log_out[42] > 100 || fft_log_out[43] > 100 || fft_log_out[44] > 100 || fft_log_out[45] > 100 || fft_log_out[46] > 100){
        hat_detected = true;
        Serial.println("Hat detected!!");
    } else {
        hat_detected = false;
        //Serial.println("No hat detected!!");
    }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;    
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
  delay(50); 
}

void full_left(){
    //move_forward();
    //delay(500);
    right.write(R_FORWARD(100));
    left.write(L_BACKWARD(0));
    delay(500);
    m_light = digitalRead(M_LIGHT);
    while(m_light == 1){ //  not on white
        m_light = digitalRead(M_LIGHT);
    }
//    move_forward();
//    delay(500/BASE_SPEED);
//    right.write(R_FORWARD(100));
//    left.write(L_BACKWARD(100));
//    delay(800/BASE_SPEED);
//    m_light = digitalRead(M_LIGHT);
//    while(m_light == 1){ //  not on white
//        m_light = digitalRead(M_LIGHT);
//    }
//    right.write(R_FORWARD(100));
//    delay(80);
////    move_forward()
////    delay(140);
////    right.write(R_FORWARD(100));
////    left.write(L_BACKWARD(100));
////    delay(200); //800/BASE_SPEED
//    m_light = digitalRead(M_LIGHT);
//    while(m_light == 1){ //  not on white
//        m_light = digitalRead(M_LIGHT);
//    }
}

void full_right(){
    //move_forward();
    //delay(500);
    right.write(R_BACKWARD(0));
    left.write(L_FORWARD(100));
    delay(500);
    m_light = digitalRead(M_LIGHT);
    while(m_light == 1){ //  not on white
        m_light = digitalRead(M_LIGHT);
    }
//    left.write(L_FORWARD(100));
//    delay(80);
////    move_forward();
////    delay(140);
////    right.write(R_BACKWARD(100));
////    left.write(L_FORWARD(100));
////    delay(200); //800/BASE_SPEED
////    m_light = digitalRead(M_LIGHT);
////    while(m_light == 1){ //  not on white
////        m_light = digitalRead(M_LIGHT);
////    }
//    m_light = digitalRead(M_LIGHT);
//    while(m_light == 1){ //  not on white
//        m_light = digitalRead(M_LIGHT);
//    }

}

void full_180(){
    //full_right();
    right.write(R_BACKWARD(100));
    left.write(L_BACKWARD(100));
    delay(100);
    //move_forward();
    //delay(300/BASE_SPEED);
    right.write(R_FORWARD(100));
    left.write(L_BACKWARD(100));
    delay(200);
    m_light = digitalRead(M_LIGHT);
    while(m_light == 1){ //  not on white
        m_light = digitalRead(M_LIGHT);
    }
//    full_left();
//    move_forward();
//    delay(80);
//    right.write(R_FORWARD(100));
//    left.write(L_BACKWARD(100));
//    delay(100);
//    m_light = digitalRead(M_LIGHT);
//    while(m_light == 1){ //  not on white
//        m_light = digitalRead(M_LIGHT);
//    }
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
    l_light = digitalRead(L_LIGHT); // 0 means sees line!!!
    r_light = digitalRead(R_LIGHT);
    m_light = digitalRead(M_LIGHT);
    Serial.print("l  ");
    Serial.print(l_light);
    Serial.print("  r  ");
    Serial.print(r_light);
    Serial.print("  m  ");
    Serial.println(m_light);
    
    
    //Line Following
    if(m_light ==0 && l_light == 0){ 
        left.write(L_FORWARD(K1*TEMP_SPEED/100));
        right.write(R_FORWARD(100*TEMP_SPEED/100));  
    } else if(m_light ==0 && r_light == 0){ 
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
    } else {
        if (!l_light_prev){
            turn_left();
            while(m_light){
                m_light = digitalRead(M_LIGHT);
            }   
        } else if(!r_light_prev){
            turn_right();
            while(m_light) {
                m_light = digitalRead(M_LIGHT);
            }
        } else{
            move_forward(); // change to move forward!!!
        }
    }
    l_light_prev = l_light;
    r_light_prev = r_light;
}

void check_for_start() {
    button_val= analogRead(EMERGENCY_BUTTON);
    //Serial.println(button_val);
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
    } else {
        count660 = 0;
    }
    if (count660 >= 10 || button_val > emergency_button_thresh){
        Serial.println("detected start!!!!");
        start_detected = true;
        Serial.println(start_detected);
        digitalWrite(MUX_CONTROL_PIN, HIGH); // change to IR
    }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;
    // send maze[0][0]to gui
    if (start_detected) {
        prev_dir        = START_DIR;
        byte right_wall = (prev_dir + 1) % 4;
        byte left_wall  = (prev_dir + 3) % 4;
        int r_wall = analogRead(R_WALL);
        int l_wall = analogRead(L_WALL);
        int f_wall = analogRead(F_WALL);
        walls[right_wall] = r_wall > R_WALL_THRESHOLD;
        walls[left_wall]  = l_wall > L_WALL_THRESHOLD;
        walls[prev_dir]   = f_wall > F_WALL_THRESHOLD;
        walls[(prev_dir + 2) % 4] = 1; // assumes wall behind
        pos[0] = 0;
        pos[1] = 0;
        rf_crap();
        pos[0] = X;
        pos[1] = Y; 
    }    
}
