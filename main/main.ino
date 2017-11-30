

int light_calibration = 972; //WHAT IS YOUR CURRENT PERFECT LIGHTING?





#include <Time.h>
#include <TimeLib.h>

//Declare pin functions on Arduino
//digital pins
#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define MS3 6
#define EN  7
#define pos_knob_A 8 //rotary encoder inputA
#define pos_knob_B 9 //rotary encoder inputB
#define button 10
#define LED 13

//analog pins
#define light A0 // photo resistor
#define am_pm A1 // am/pm switch
#define tknob A2 // time knob
#define five_psA A3 // 5 position switch
#define five_psB A4 // 5 position switch
#define five_psC A5 // 5 position switch


//Declare variables for functions
//from stepper code:
int x;

//our variables

int A3level;
int A4level;
int A5level;

int button_pos;
int counter = 0;
int counterMin = -9999;
int counterMax = 9999;
int counter_midpoint;
int current_time;

int light_level_1;
int light_level_2;
int difference;

int onHr;
int onMin;
int onSec;
int offHr;
int offMin;
int offSec;

bool clockIsSet = false;
bool onIsSet = false;
bool offIsSet = false;

int mode; //mode selected by 5 pos switch

int aState;
int aLastState = digitalRead(pos_knob_A);
int bState;
int bLastState = digitalRead(pos_knob_B);

void resetBEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
  digitalWrite(EN, HIGH);
}
void setup() {

  //initialize inputs and outputs

  //digital
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);
  resetBEDPins(); //Set step, direction, microstep and enable pins to default states
  pinMode(pos_knob_A, INPUT);
  pinMode(pos_knob_B, INPUT);
  pinMode(button, INPUT);
  pinMode(LED, OUTPUT);

  //analog
  pinMode(light, INPUT);
  // pinMode(five_ps, INPUT);
  pinMode(tknob, INPUT);
  pinMode(am_pm, INPUT);

  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println("Serial initialized.");
  Serial.println();
  digitalWrite(EN, LOW); //unlock motor

}


/*
----------------------------------------------------
BEGIN FUNCTION DEFINITIONS
----------------------------------------------------
*/
//PHOTORESISTOR LIGHT LEVEL
void light_level()
{
  light_level_1 = analogRead(light);
  delay(5000);
  light_level_2 = analogRead(light);
  difference = light_level_1-light_level_2;
  Serial.print("light difference, 5s: ");
  Serial.println(difference);
  Serial.println();
  Serial.print("final light level: ");
  Serial.println(light_level_2);
  Serial.println();
}

int five_ps_mode(){
  A3level = analogRead(five_psA);
  A4level = analogRead(five_psB);
  A5level = analogRead(five_psC);
  x=0;
  int threshold=0;
  /*
  Serial.print("pin 1 = ");
  Serial.println(A3level);
  Serial.print("pin 2 = ");
  Serial.println(A4level);
  Serial.print("pin 3 = ");
  Serial.println(A5level);
  */
  if (A3level == threshold && A4level != threshold && A5level != threshold){
    x = 1;
  }
  else if (A3level == threshold && A4level == threshold){
    x = 2;
  }
  else if (A4level == threshold && A5level != threshold && A3level != threshold){
    x = 3;
  }
  else if (A4level == threshold && A5level == threshold){
    x = 4;
  }
  else if (A4level != threshold && A5level == threshold && A3level != threshold){
    x = 5;
  }
  // Serial.print("Mode number: ");
  //  Serial.println(x);
  return x;
}

//Flash LED at different rates
void flash_LED() {
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}

//Default microstep mode function
void StepForwardDefault()
{
  if (counter >= counterMin && counter < counterMax){
  counter ++;
  Serial.println("Moving forward at default step mode.");
  digitalWrite(dir,LOW);
    for(x= 1; x<50; x++){  //Loop the stepping for 1/4 turn
      digitalWrite(stp,HIGH); //Trigger one step forward
      delay(1);
      digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
      delay(1);
    }
  }
   else{
   Serial.println("Error in endpoints -- step forward");
   }
}


//Reverse default microstep mode function
void ReverseStepDefault(){
  if (counter <= counterMax && counter > counterMin){
    counter --;
    Serial.println("Moving in reverse at default step mode.");
    digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
    for(x= 1; x<50; x++){  //Loop the stepping for 1/4 turn
      digitalWrite(stp,HIGH); //Trigger one step
      delay(1);
      digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
      delay(1);
    }
  }
  else{
  Serial.println("Error in endpoints -- reverse step");
  }
}

void setCurrentTime(){
  int readtime = analogRead(tknob);
  //  Serial.print("Time knob reading ");
  //Serial.println(readtime);

  bool pm = false;
  int ampm = analogRead (am_pm);
  //  Serial.print("Two pos switch reading ");
  //  Serial.print(ampm);
  if (ampm == 0){
    pm = true;
  }
  int hr = readtime*12/1024;
  if (pm==true){
    hr = hr + 12;
  }
  int mn = (((readtime*12*60/1024) %60)+60)%60;
  int sec = (((readtime*12*60*60/1024)%60)+60)%60;
  int x = hr;
  int y = mn;
  int z = sec;

  Serial.print("  Now setting time to ");
  Serial.print(x);
  Serial.print(":");
  Serial.print(y);
  Serial.print(":");
  Serial.println(z);

  setTime(x,y,z,1,1,2017);
  clockIsSet = true;
  time_t t = now();
  Serial.print("the hour is: ");
  Serial.print(hour(t));
}

void setSchedOn (){
  int readtime = analogRead(tknob);
  bool pm = false;
  int ampm = analogRead (am_pm);
  if (ampm == 0){
    pm = true;
    Serial.println("pm");
  }

  int hr = readtime*12/1024;
  if (pm==true){
    hr = hr + 12;
  }
  int mn = (((readtime*12*60/1024) %60)+60)%60;
  int sec = (((readtime*12*60*60/1024)%60)+60)%60;
  onHr = hr;
  onMin = mn;
  onSec = sec;
  Serial.print("on hour = ");
  Serial.println(onHr);
  Serial.print("on minute = ");
  Serial.println(onMin);
  Serial.print("on sec = ");
  Serial.println(onSec);
  
}
void setSchedOff (){
  int readtime = analogRead(tknob);
  bool pm;
  int ampm = analogRead (am_pm);
  if (ampm == 0){
    pm = true;
    Serial.println("pm");
  }
  int hr = readtime*12/1024;
  if (pm==true){
    hr = hr + 12;
  }
  int mn = (((readtime*12*60/1024) %60)+60)%60;
  int sec = (((readtime*12*60*60/1024)%60)+60)%60;
  offHr = hr; //extra variable definition may not be necessary
  offMin = mn;
  offSec = sec;
  Serial.print("on hour = ");
  Serial.println(offHr);
  Serial.print("on minute = ");
  Serial.println(offMin);
  Serial.print("on sec = ");
  Serial.println(offSec);
}

// Rotary encoder
void Rot_Knob () {
  aState=digitalRead(pos_knob_A); //This will read the current state of A
  bState=digitalRead(pos_knob_B);
  // If the previous and the current are the different that means the knob has
  // moved.
  if (aState != aLastState){
 /*
    Serial.print("A Original State ");
    Serial.println(aLastState);
    Serial.print("B Original State ");
    Serial.println(bLastState);
    Serial.print("A New State ");
    Serial.println(aState);
    Serial.print("B New State ");
    Serial.println(bState);
*/

    // pos_knob_B compared to pos_knob_A will tell you which direction the
    // encoder is going.
    //probably clockwise - check!
    if (bState!= aState){
      StepForwardDefault();
 //     Serial.print("forward");
    }
    else {
      ReverseStepDefault();
 //     Serial.print("backward");
    }
    Serial.println();
    Serial.print("Counter:");
    Serial.println(counter);
    Serial.println();
  }
  /*
  else {
  if (bState == aState){
  ++counter;
  StepForwardDefault();
  Serial.print("forward");
}
else {
--counter;
ReverseStepDefault();
Serial.print("backward");
}

}
*/
aLastState=aState; //This step updates the previous state with the new state
bLastState=bState;
}

/*
---------------------------------------
MAIN LOOP
---------------------------------------
*/

void loop()
{

  while (true){

    //    mode = five_ps_mode();

    //Determine current position of 5 position switch
    //TODO change if statements, get rid of last_pos
    if (five_ps_mode() == 0)
    {
      Serial.println("Error, device not configured yet");
    }

    //AUTO MODE
    if (five_ps_mode() == 1)
    {
      if (clockIsSet && onIsSet && offIsSet == true){
      //make sure clock and schedule are set up
        int ontime = onHr*1000+onMin;
        int offtime = offHr*1000 + offMin;
        int rn = hour() * 1000 + minute();
        if (rn > ontime && rn < offtime){
        light_level();
  if (abs(difference) < 75) {
    if (abs(light_level_2 - light_calibration) > 20){
      Serial.print("COUNTER MIDPOINT:");
      Serial.println(counter_midpoint);
      if (light_level_2 < light_calibration){ //TOO DARK
        if (counter < counter_midpoint){ //check counter
          StepForwardDefault(); //OPEN UP
          Serial.println("TOO DARK");
          Serial.print("counter: ");
          Serial.println(counter);
        }
        else if (counter > counter_midpoint){ //check counter
          ReverseStepDefault();//OPEN DOWN
          Serial.println("TOO DARK");
          Serial.print("counter: ");
          Serial.println(counter);
        }
        else
        Serial.println("TOO DARK");
        Serial.println("Blinds already max open");
      }
      
      else if (light_level_2 > light_calibration){ //TOO BRIGHT
        if (counter >= counter_midpoint){ //MOVE UP
          StepForwardDefault();
          Serial.println("TOO BRIGHT");
          Serial.print("counter: ");
          Serial.println(counter);
        }
        else if (counter < counter_midpoint){ //MOVE DOWN
          ReverseStepDefault();
          Serial.println("TOO BRIGHT");
          Serial.print("counter: ");
          Serial.println(counter);
        }
      }
    }
        else{
    Serial.println("light is in comfy place :)");
  }
      }
    }
    }
    }
    
    //OVERRIDE
    if (five_ps_mode() == 2){
        Rot_Knob();//take rotary encoder input
    }

//set CT
if (five_ps_mode() == 3)
{
  delay(500);
  while (five_ps_mode() == 3)
  {
    //  Serial.println("now in set current time mode");
    //  Serial.println();
    setCurrentTime();
  }
}
if (five_ps_mode() == 4)
{
  delay(500);
  if (five_ps_mode() == 4){
    Serial.println("Set time mode -- ON time (you have 5s)");
    Serial.println();

    // now() function is part of timeLib, uncomment when that works
    unsigned long finishAtOn = now() + 5; //tells you what time to finish
    while (now() < finishAtOn) //run setSchedon for 5 seconds
    //

    {
      setSchedOn();
    }
    flash_LED(); //flash when on setting is set
    onIsSet = true;
    Serial.println("Set time mode -- OFF time (you have 5s... well ok unlimited time for now but I'm working on it)");
    Serial.println();
    // now() function is part of timeLib, uncomment when that works
    unsigned long finishAtOff = now() + 8;
    while (now() < finishAtOff) //run set_on for 5 seconds
    {
      setSchedOff();

    }
    flash_LED();
    offIsSet = true;
  }
}

//set stops
if (five_ps_mode() == 5)
{
  //  Serial.println("Now you will set the stops");
  digitalWrite(button, HIGH);
  delay(1);
  button_pos = digitalRead(button);
  Serial.println("now in position setting mode -- UP");
  while (button_pos !=0 ){
    Rot_Knob();
    button_pos = digitalRead(button);
  }

  counterMax = counter; //may be max not min
  Serial.print("UP position set. counterMax is: ");
  Serial.println(counterMax);
  Serial.println();

  digitalWrite(button, HIGH);
  delay(1000);
  button_pos = digitalRead(button);
  Serial.println("now in position setting mode -- DOWN");
  while (button_pos !=0){
    Rot_Knob();
    button_pos = digitalRead(button); //exit while loop if clicked
  }
  counterMin = counter;
  Serial.print("DOWN position set. counterMin is: ");
  Serial.println(counterMin);
  //  }
  counter_midpoint = counterMax-((counterMax+abs(counterMin))/2);
  Serial.print("MIDPOINT:");
  Serial.println(counter_midpoint);

delay(5000);
}
  }
}
