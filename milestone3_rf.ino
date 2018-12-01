#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


RF24 radio(9,10);
const uint64_t pipes[2] = { 0x00000000042LL, 0x0000000043LL };

// encoding
const int W_BW = 4; // bitwidth
const int T_BW = 4;
//const int R_BW = 1;
//const int E_BW = 1;  
const int W_OFFSET = 8; // walls 
const int T_OFFSET = W_OFFSET + W_BW; // treasure
//const int R_OFFSET = T_OFFSET + T_BW; // robot  
//const int E_OFFSET = R_OFFSET + R_BW; // explored whole maze

enum _direction{E, S, W, N};
//int pos[2] = {0,0};
int walls[4] = {0, 0, 0, 0};
int dir = S;
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
            //  (data     &  mask << offset) >> offset
            int x = got_time & ((1 << 4) - 1); // 4 bits 
            int y = (got_time & ((1 << 4) - 1) << 4) >> 4; // 4 bits 
            walls[W]    = (got_time & (1 << (W_OFFSET + W))) >> (W_OFFSET + W);
            walls[E]    = (got_time & (1 << (W_OFFSET + E))) >> (W_OFFSET + E);
            walls[N]    = (got_time & (1 << (W_OFFSET + N))) >> (W_OFFSET + N);
            walls[S]    = (got_time & (1 << (W_OFFSET + S))) >> (W_OFFSET + S);
            
            _print = String(y) + ","
                   + String(x) + "," 
                   + "west="  + bool_str(walls[W]) + ","
                   + "north=" + bool_str(walls[N]) + ","
                   + "east="  + bool_str(walls[E]) + ","
                   + "south=" + bool_str(walls[S]);
            //Serial.println(got_time, BIN);
            Serial.println(_print);

            
        }
    }
}

String bool_str(int b){
  return b ? "true" : "false";
}
