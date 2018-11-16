const int R_WALL = A1;
const int L_WALL = A0;
const int F_WALL = A5;

int wall_threshold = 100;
int r_wall = 0;
int l_wall = 0;
int f_wall = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  r_wall = analogRead(R_WALL);
  l_wall = analogRead(L_WALL);
  f_wall = analogRead(F_WALL);

  if(r_wall > wall_threshold){
    Serial.println("RIGHT WALL DETECTED");
  }

  else if(l_wall > wall_threshold){
    Serial.println("LEFT WALL DETECTED");
  }

  else if(f_wall > wall_threshold){
    Serial.println("FRONT WALL DETECTED");
  }

  else if((f_wall < wall_threshold)&&(r_wall < wall_threshold)&&(l_wall < wall_threshold)){
    Serial.println("NO WALL DETECTED");
  }
  

}
