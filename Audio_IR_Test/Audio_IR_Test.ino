#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

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

//Analog pins
const int EMERGENCY_BUTTON = A2;
const int IR_AUDIO_SENSOR = A4;

//IR 1, AUDIO 0
const int MUX_CONTROL_PIN = 7;


void setup() {
  // put your setup code here, to run once:
  digitalWrite(MUX_CONTROL_PIN, HIGH);
  Serial.begin(9600);
  timsko = TIMSK0;
  admux = ADMUX;
  didro = DIDR0;
  adcsra = ADCSRA;
  pinMode(MUX_CONTROL_PIN, OUTPUT);
  //while(!start_detected){
   // check_for_start();
  //}
  //digitalWrite(MUX_CONTROL_PIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  check_for_hat();
 

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
          //Serial.println(count660++);
    } else {
          count660 = 0;
          
    }
    if (count660 >= 10 || button_val>emergency_button_thresh){
          Serial.println("detected start!!!!");
          start_detected = true;
          
    }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;
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
      
      }
    else {
      hat_detected = false;
      Serial.println("No hat detected!!");
      }
    TIMSK0 = timsko;
    ADMUX = admux;
    DIDR0 = didro;
    ADCSRA = adcsra;
}
