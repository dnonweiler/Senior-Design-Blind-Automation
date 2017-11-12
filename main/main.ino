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
char user_input;
int x;
int y;
int state;

//our variables

int startTimeVolts;
int stopTimeVolts;
int topStop;
int bottomStop;
int button_pos;

int current_time;

int hr = 0;
int mn=0;
int sec=0;

bool clockIsSet = false;
bool onIsSet = false;
bool offIsSet = false;



int mode; //mode selected by 5 pos switch

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
  int light_level = analogRead(light);
  delay(5000);
  Serial.println(light_level);
  if (light_level < 4800)
  StepForwardDefault();
  else if (light_level > 5200){
  ReverseStepDefault();
  }
}

int five_ps_mode(int A,int B,int C){
  Serial.println(A);
  Serial.println(B);
  Serial.println(C);
  x=0;
  if (A > 500 && B < 500){
    x = 1;
  }
  else if (A > 500 && B > 500){
    x = 2;
  }
  else if (B > 500 && C < 500){
    x = 3;
  }
  else if (B > 500 && C > 500){
    x = 4;
  }
  else if (B < 500 && C > 500){
    x = 5;
  }
  return x;
}

//digital button on encoder knob
int button_status()
{
  x = 0;
  button_pos = digitalRead(button);

  if (button_pos == HIGH){
    x = x + 1;
    return x;
  }
  else if (button_pos == LOW){
    return x;
  }
}

//Flash LED at different rates
/*  void flash_LED(    char do we need to make an input here?   ) {
if flash_LED(set_current_time);
digitalWrite(LED, HIGH);
delay(500);
digitalWrite(LED, LOW);
delay(500);
else if flash_LED(set_on);
digitalWrite(LED,HIGH);
delay(250);
digitalWrite(LED,LOW);
delay(250);
else if flash_LED(set_off);
digitalWrite(LED,HIGH);
delay(250);
digitalWrite(LED,LOW);
delay(250);
else if flash_LED(manual_override);
digitalWrite(LED,HIGH);
delay(250);
digitalWrite(LED,LOW);
delay(250);
else if flash_LED(off);
digitalWrite(LED,LOW);
}

*/
//Default microstep mode function
void StepForwardDefault()
{
  digitalWrite(stp,HIGH); //Trigger one step forward
  delay(1);
  digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
  delay(1);
  Serial.println("command finished");
  Serial.println();
}


//Reverse default microstep mode function
void ReverseStepDefault(){
  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  for(x= 1; x<1000; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("command finished");
  Serial.println();
}

// 1/16th microstep foward mode function
// We don't really need this resolution... keep as an example in case we change our mind
void SmallStepMode()
{
  Serial.println("Stepping at 1/16th microstep mode.");
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);
  for(x= 1; x<1000; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("command finished");
  Serial.println();
}



void setCurrentTime(){
        int readtime = analogRead(tknob);
        bool pm;
        int ampm = analogRead (am_pm);
        if (ampm == 1023){
          pm = true;
          Serial.println("pm");
        }
        
        //delete this section later, just for testing
        else if (ampm == 0){
          pm = false;
          Serial.println("am");
        }
        else{
          Serial.println("am/pm switch reading error");
        }
        //

        
        hr = readtime*12/1024;
        if (pm==true){
          hr = hr + 12;
        }
        mn = (readtime*12*60/1024) %60;
        sec = (readtime*12*60*60/1024) %60;
        int x = hr;
        int y = mn;
        int z = sec;
        Serial.print("hour = ");
        Serial.println(x);
        Serial.print("minute = ");
        Serial.println(y);
        Serial.print("sec = ");
        Serial.println(z);
   //     setTime(x,y, z,1,1, 2017);
        clockIsSet = true;
}

// Rotary encoder
void Rot_Knob () {
  int counter = 0;
  int aState;
  int aLastState;
  aLastState= digitalRead(pos_knob_A); //This will read the intial state of A
  Serial.println("Original State");
  Serial.println(aLastState);
  delay(300);
  aState=digitalRead(pos_knob_A); //This will read the current state of A
  // If the previous and the current are the different that means the knob has
  // moved.
  Serial.println("New State");
  Serial.println(aState);
  if (aState != aLastState){
    // pos_knob_B compared to pos_knob_A will tell you which direction the
    // encoder is going.
    if (digitalRead(pos_knob_B != aState)){
      counter ++;
      StepForwardDefault();
    }
    else {
      counter --;
      ReverseStepDefault();
    }
    if (counter >=30) {
      counter =0;
    }

    Serial.print("Position:");
    Serial.println(counter);
  }
  aLastState=aState; //This step updates the previous state with the new state
}

/*
---------------------------------------
MAIN LOOP
---------------------------------------
*/

void loop()
{
  while (true){
    int A3level = analogRead(five_psA);
    int A4level = analogRead(five_psB);
    int A5level = analogRead(five_psC);
    mode = five_ps_mode(A3level, A4level, A5level);

    //Determine current position of 5 position switch
    //TODO change if statements, get rid of last_pos
    if (mode == 0)
    {
      Serial.println("Error, mode not set");
      Serial.println();
    }

    //AUTO MODE
    if (mode == 1)
    {
      if (clockIsSet & onIsSet & offIsSet == true)
      {//make sure it's set up (check stops
        // in the stepper functions)
        Serial.println("now in automatic mode :)");
        Serial.println();
        //  if (clock)
        //run photoresistor script on repeat

      }
    }
    //override
    else if (mode == 2)
    {
      delay(500);
      if (mode == 2)
      {
        Serial.println("now in override mode! :o");
        Serial.println();
        Rot_Knob();//take rotary encoder input
        
      }
    }
    //set CT
    else if (mode == 3)
    {
      delay(500);
      if (mode == 3)
      {

        Serial.println("now in set current time mode");
        Serial.println();
        setCurrentTime();
        //read tknob, am_pm to set "current_time"
      }
    }
    else if (mode == 4)
    {
      delay(500);
      if (mode == 4)
      {
        //flash_LED(set_on);
        Serial.println("Set time mode -- ON time (you have 5s)");
        Serial.println();
        onIsSet = true;
        delay(5000);
        // flash_LED(set_off);
        Serial.println("Set time mode -- OFF time (you have 5s... well ok unlimited time for now but I'm working on it)");
        Serial.println();
        offIsSet = true;
      }
    }
    //read tknob and am_pm to set "time_off" value

    //how do we end this loop so that you can't keep changing the time??

    //look for button press to change blind end point MODE


    //set stops
    else if (mode == 5)
    {
      delay(500);
      if (mode == 5)
      {
        if (button_status == 0)
        {
          // flash_LED(set_pos);
          Serial.println("now in position setting mode -- NEUTRAL");
          Serial.println();
          //call manual_override

//          Blind_pos = 0;

          Serial.println("now in position setting mode -- MAX UP");
          Serial.println();

          Serial.println("now in position setting mode -- MAX DOWN");
          Serial.println();
        }
      }
    }
  }
  }
