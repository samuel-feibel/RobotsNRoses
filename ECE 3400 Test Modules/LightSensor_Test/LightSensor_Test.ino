const int thresh       = 750;
const int L_LIGHT    = A2;
const int R_LIGHT    = A3;
int l_light = 0;
int r_light = 0;
//How responsive Slash is to detecting intersections, the higher the value, the less responsive.
int count_enter =0;

//How long Slash must wait after detecting an intersection before he can detect another one, the higher the value, the longer the wait.
int count_exit =0;

int count_thresh_enter=500;
int count_thresh_exit=50;

bool intersection_detect=false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}
bool detect_intersection(){
  l_light = analogRead(L_LIGHT);
  r_light = analogRead(R_LIGHT);

  //Detect the possibility of an intersection
  if (l_light<thresh && r_light<thresh){
    count_enter++;
 
    //Detecting a real intersection
    if (count_enter > count_thresh_enter && count_exit>count_thresh_exit){
      Serial.println("Intersection Detected");
      intersection_detect=true;
      //Must start waiting before we can detect the next
      count_exit=0;
    }
  }
  //No intersection means that the timer is running to allow Slash to detect another intersection. 
  else {
    Serial.println("No Intersection Detected");
    intersection_detect=false;
    count_enter=0;
    count_exit++;
    //Serial.println(count_exit);
  }
}

void loop() {
  l_light = analogRead(L_LIGHT);
  r_light = analogRead(R_LIGHT);
  detect_intersection();
 
//  Serial.println("LEFT SENSOR");
//  Serial.println(l_light);
//  Serial.println("RIGHT SENSOR");
//  Serial.println(r_light);
}
