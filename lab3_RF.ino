#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


RF24 radio(9,10);
const uint64_t pipes[2] = { 0x00000000042LL, 0x0000000043LL };

const int Y_OFFSET = 0; // y pos
const int X_OFFSET = 4; // x pos
const int W_OFFSET = 8; // walls 
const int T_OFFSET = 12; // treasure
const int R_OFFSET = 16; // robot  
const int E_OFFSET = 17; // explored whole maze 
enum _direction{E, S, W, N};
int curr_pos[2] = {0,0};
int walls[4] = {0, 0, 0, 0};
String _print = "";

void setup(void) {
    Serial.begin(9600);
    radio.begin();
    radio.setRetries(15,15);
    radio.setAutoAck(true);
    radio.setChannel(0x50);
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);
    radio.setPayloadSize(4); // 4 byte payload (unsigned long) TODO
    radio.openWritingPipe(pipes[1]); // role != role_ping_out
    radio.openReadingPipe(1,pipes[0]);
    radio.startListening();
    //radio.printDetails();
}

void loop(void) {
    if ( radio.available() ) {
        // Dump the payloads until we've gotten everything
        unsigned long got_time;
        bool done = false;
        while (!done) {
            // Fetch the payload, and see if this was the last one.
            done = radio.read( &got_time, sizeof(unsigned long));
            
            // Spew it
            //printf("Got payload %lu...",got_time);
            //printf("Got payload %X...",got_time);

            curr_pos[1] = got_time & (((1 << 4) - 1) << Y_OFFSET); // y pos
            curr_pos[0] = (got_time & (((1 << 4) - 1) << X_OFFSET)) >> X_OFFSET; //x pos
            walls[W]    = (got_time & (1 << (W_OFFSET + W))) >> (W_OFFSET + W);
            walls[E]    = (got_time & (1 << (W_OFFSET + E))) >> (W_OFFSET + E);
            walls[N]    = (got_time & (1 << (W_OFFSET + N))) >> (W_OFFSET + N);
            walls[S]    = (got_time & (1 << (W_OFFSET + S))) >> (W_OFFSET + S);
            
            _print = String(curr_pos[1]) + ","
                   + String(curr_pos[0]) + "," 
                   + "west="  + bool_str(walls[W]) + ","
                   + "north=" + bool_str(walls[N]) + ","
                   + "east="  + bool_str(walls[E]) + ","
                   + "south=" + bool_str(walls[S]);
            //Serial.println(temp);
            Serial.println(_print);
            // Delay just a little bit to let the other unit
            // make the transition to receiver
            //delay(20);
        }
    }
}

String bool_str(int b){
  return b ? "true" : "false";
}
