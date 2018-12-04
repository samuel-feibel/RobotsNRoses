const int R_WALL = A1;
const int L_WALL = A0;
const int F_WALL = A5;
const int button = A2;
int button_val = 0;
int r_wall_low = 0;
int l_wall_low = 0;
int f_wall_low = 0;
int r_wall_high = 0;
int l_wall_high = 0;
int f_wall_high = 0;
int r_wall_thresh = 0;
int l_wall_thresh = 0;
int f_wall_thresh = 0;

int wall_threshold;
int r_wall = 0;
int l_wall = 0;
int f_wall = 0;
int button_thresh = 900;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  button_val = analogRead(button);
  while (button_val < 1000){
    button_val = analogRead(button);  
    Serial.println("Waiting for first press");
  };

  for (int i = 0; i < 50; i++){
    r_wall_high = r_wall_high + analogRead(r_wall);
    f_wall_high = f_wall_high + analogRead(f_wall);
    l_wall_high = l_wall_high + analogRead(l_wall);
    Serial.print("Sampling High ");
    Serial.println(analogRead((r_wall + l_wall + f_wall)/3));
  }

  r_wall_high = r_wall_high/50;
  f_wall_high = f_wall_high/50;
  l_wall_high = l_wall_high/50;
  
  delay(1000);

  button_val = analogRead(button);
  while (button_val < 1000){
    button_val = analogRead(button);  
    Serial.println("Waiting for second press");
  };
  
  for (int i = 0; i < 50; i++){
    r_wall_low = r_wall_low + analogRead(r_wall);
    f_wall_low = f_wall_low + analogRead(f_wall);
    l_wall_low = l_wall_low + analogRead(l_wall);
    Serial.print("Sampling Low");
    Serial.println(analogRead(r_wall + l_wall + f_wall)/3));
  }

  r_wall_low = r_wall_low/50;
  f_wall_low = f_wall_low/50;
  l_wall_low = l_wall_low/50;

  r_wall_thresh = (r_wall_high-r_wall_low)/2;
  f_wall_thresh = (f_wall_high-f_wall_low)/2;
  l_wall_thresh = (l_wall_high-l_wall_low)/2;

  Serial.println(r_wall_thresh);
  Serial.println(f_wall_thresh);
  Serial.println(l_wall_thresh);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  r_wall = analogRead(R_WALL);
  l_wall = analogRead(L_WALL);
  f_wall = analogRead(F_WALL);

  //Serial.print(r_wall);
  //Serial.print(f_wall);
  
  

}
