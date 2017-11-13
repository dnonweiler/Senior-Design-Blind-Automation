//#include <Time.h>
//#include <TimeLib.h>

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
int current_time;

int hr = 0;
int mn=0;
int sec=0;

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
  Serial.println("Hiya. Let's fuck up some blinds!");
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
  int light_level_1 = analogRead(light);
  delay(5000);
  int light_level_2 = analogRead(light);
  int difference = light_level_1-light_level_2;

if (abs(difference) < 20) {
  Serial.println(light_level_2);
  if (light_level_2 < 900){
    StepForwardDefault();
    StepForwardDefault();
    StepForwardDefault();
    }
  else if (light_level_2 > 900){
    ReverseStepDefault();
    ReverseStepDefault();
    ReverseStepDefault();
    }
}
}

int five_ps_mode(){
  A3level = analogRead(five_psA);
  A4level = analogRead(five_psB);
  A5level = analogRead(five_psC);
  x=0;
  int threshold=0;
  if (A3level == threshold && A4level != threshold){
    x = 1;
  }
  else if (A3level == threshold && A4level == threshold){
    x = 2;
  }
  else if (A4level == threshold && A5level != threshold){
    x = 3;
  }
  else if (A4level == threshold && A5level == threshold){
    x = 4;
  }
  else if (A4level != threshold && A5level == threshold){
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
  Serial.println("Moving forward at default step mode.");
  digitalWrite(dir,LOW); //Trigger one step forward
  for(x= 1; x<20; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
 //   delay(1);
  }
  //  Serial.println("command finished");
  //  Serial.println();
}


//Reverse default microstep mode function
void ReverseStepDefault(){
  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  for(x= 1; x<20; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
//    delay(1);
  }
  //  Serial.println("command finished");
  //  Serial.println();
}

void setCurrentTime(){
  int readtime = analogRead(tknob);
  bool pm;
  int ampm = analogRead (am_pm);
  if (ampm == 1023){
    pm = true;
  }

  hr = readtime*12/1024;
  if (pm==true){
    hr = hr + 12;
  }
  mn = (((readtime*12*60/1024) %60)+60)%60;
  sec = (((readtime*12*60*60/1024)%60)+60)%60;
  int x = hr;
  int y = mn;
  int z = sec;
  Serial.print("Now setting time to ");
  Serial.print(x);
  Serial.print(":");
  Serial.print(y);
  Serial.print(":");
  Serial.println(z);
//     setTime(x,y,z,1,1,2017);
  clockIsSet = true;
}

void setSchedOn (){
  int readtime = analogRead(tknob);
  bool pm;
  int ampm = analogRead (am_pm);
  if (ampm == 1023){
    pm = true;
    Serial.println("pm");
  }

  hr = readtime*12/1024;
  if (pm==true){
    hr = hr + 12;
  }
  mn = (((readtime*12*60/1024) %60)+60)%60;
  sec = (((readtime*12*60*60/1024)%60)+60)%60;
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
  if (ampm == 1023){
    pm = true;
    Serial.println("pm");
  }
  hr = readtime*12/1024;
  if (pm==true){
    hr = hr + 12;
  }
  mn = (((readtime*12*60/1024) %60)+60)%60;
  sec = (((readtime*12*60*60/1024)%60)+60)%60;
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
     Serial.print("knob moving ");
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
      counter ++;
      StepForwardDefault();
      Serial.print("forward");
    }
    else {
      counter --;
      ReverseStepDefault();
      Serial.print("backward");
    }
    Serial.println();
    Serial.print("Counter:");
    Serial.println(counter);
    Serial.println();
  }
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
      Serial.println("Error, mode not set");
    }

    //AUTO MODE
    if (five_ps_mode() == 1)
    {
      if (clockIsSet && onIsSet && offIsSet == true)
      {//make sure it's set up (check stops
        // in the stepper functions)
        Serial.println("now in automatic mode :)");
        Serial.println();
        light_level();
        //  if (clock)
        //run photoresistor script on repeat

      }
    }
    //override
     if (five_ps_mode() == 2)
    {
      delay(500);
      if (counterMin!= -9999 && counterMax !=9999){
        while (five_ps_mode() == 2 )
        {
             Serial.println("now in override mode! :o");
          //   Serial.println();
          if (counter>counterMin && counter<counterMax){
            Rot_Knob();//take rotary encoder input
          }
          else if (counter <= counterMin){
            counter ++;
            StepForwardDefault();
          }
          else {// counter > max
            counter --;
            ReverseStepDefault();
          }
          //what to do if it goes too far and you want to be able to turn it back?
          Serial.print("CounterMin is: ");
          Serial.println(counterMin);
          Serial.print("CounterMax is: ");
          Serial.println(counterMax);
          Serial.print("counter value: ");
          Serial.println(counter);

        }
      }
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
      //     delay(500);
      if (five_ps_mode() == 4){
        Serial.println("Set time mode -- ON time (you have 5s)");
        Serial.println();

        // now() function is part of timeLib, uncomment when that works
        //       unsigned long finishAtOn = now() + 5;
        //       while (now() < finishAtOn) //run set_on for 5 seconds
        //

        {
          setSchedOn();
        }
        //flash_LED(set_on);
        onIsSet = true;
        Serial.println("Set time mode -- OFF time (you have 5s... well ok unlimited time for now but I'm working on it)");
        Serial.println();
        // now() function is part of timeLib, uncomment when that works
        //       unsigned long finishAtOff = now() + 5;
        //       while (now() < finishAtOff) //run set_on for 5 seconds {
        setSchedOff();

      }
      // flash_LED(set_off);
      offIsSet = true;
    }
    //read tknob and am_pm to set "time_off" value

    //how do we end this loop so that you can't keep changing the time??

    //look for button press to change blind end point MODE


    //set stops
 if (five_ps_mode() == 5)
    {
      //  Serial.println("Now you will set the stops");
        digitalWrite(button, HIGH);
        delay(1);
        button_pos = digitalRead(button);
        Serial.println("now in position setting mode -- UP");
        while (button_pos !=0 && five_ps_mode() == 5){
          Rot_Knob();
          button_pos = digitalRead(button);
        }
        // flash_LED(set_pos);
        if  (five_ps_mode() == 5){
          counterMax = counter; //may be max not min
          Serial.print("UP position set. counterMax is: ");
          Serial.println(counterMax);
          Serial.println();

          digitalWrite(button, HIGH);
          delay(1);
          button_pos = digitalRead(button);
          Serial.println("now in position setting mode -- DOWN");
          while (button_pos !=0 && five_ps_mode() == 5){
            Rot_Knob();
            button_pos = digitalRead(button);
          }
          if (five_ps_mode() == 5){
            // flash_LED(set_pos);
            counterMin = counter;
            Serial.println("DOWN position set. counterMin is: ");
            Serial.println(counterMin);
          }
      }
    }
  }
}
