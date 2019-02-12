//Code written by Allen Mitro, for the use of the
//Queen's Aero Design Team.

//For any other purpose, the following code is provided on
//an AS-IS basis, and comes without any warranties, express or implied.

//Latest recorded revision: Feb 11 2019

//pins for controlling motor 1
#define M1_en 9
#define M1_pl 8
#define M1_dr 7

//PWM tolerance values: ignore values until
//tolmax # of readings within tolerance are recorded.
#define tolmax 5
#define tolerance 10

//Three control states are possible. Left, Stop or Right Rotation.
//PWM values under refuse_min or over refuse_max are discarded.
//Between refuse_min and low_ceil is Left
//Between low_ceil and med_ceil is Stop
//Between med_ceil and refuse_max is Right
#define refuse_min 800
#define low_ceil 900
#define med_ceil 1000
#define refuse_max 2000

//Activates debug features if not zero.
//>>Outputs periodically changing PWM values from pin 11.
//>>Periodically displays latest PWM reading and intended pin 11 direction.
#define debug 1

//choice controls which of three states it is in
//-1 = Left, 0 = stop, 1 = Right
volatile int choice = 0;
//variables holding state of direction and enable pins.
boolean dirselect = false;
boolean enaselect = false;
//used in the interrupt function to calculate PWM value
volatile int prev_time = 0;
volatile int pwm_value = 0;
//microseconds to wait between motor steps
volatile int spd = 1000;
//used for timing the debug PWM value changes
int timer = 0;
int output = 0;
//Current PWM reading and number of similar readings.
volatile int tolval = 0;
volatile int toltimes = 0;

void setup()
{
  //set pin modes and initial values
  pinMode(M1_en, OUTPUT);
  pinMode(M1_pl, OUTPUT);
  pinMode(M1_dr, OUTPUT);
  digitalWrite(M1_en,LOW);
  digitalWrite(M1_dr,LOW);
  digitalWrite(M1_pl,LOW);
  pinMode(11, OUTPUT);
  analogWrite(11,70);
  //activate debug?
  if(debug != 0)
    Serial.begin(115200);
  //start PWM calculating
  attachInterrupt(0, rising1, RISING);
}

void loop()
{
  //choice indicates go left.
  if(choice == -1)
  {
    //update flags and pins. We don't want repeated changes.
    if(dirselect)
    {
      dirselect = false;
      digitalWrite(M1_dr,LOW);
    }
    if(enaselect)
    {
      enaselect = false;
      digitalWrite(M1_en,LOW);
    }
    //pulse motor
    digitalWrite(M1_pl,HIGH);
    delayMicroseconds(spd);
    digitalWrite(M1_pl,LOW);
  }
  //choice indicates stop
  else if(choice == 0)
  {
    //disable motor
     if(enaselect)
    {
      enaselect = true;
      digitalWrite(M1_en,HIGH);
    }
    //now wait
    delayMicroseconds(spd);
  }
  //choice indicates right
  else if (choice == 1)
  {
    if(!dirselect)
    {
      dirselect = true;
      digitalWrite(M1_dr,HIGH);
    }
    if(enaselect)
    {
      enaselect = false;
      digitalWrite(M1_en,LOW);
    }
    digitalWrite(M1_pl,HIGH);
    delayMicroseconds(spd);
    digitalWrite(M1_pl,LOW);
  }
  //enabled debug?
  if(debug != 0)
  {
    //change set pwm value
    if(timer > 4000)
    {
      timer = 0;
      output = (output+1) %3;
      Serial.println(pwm_value);
      if(output == 0)
      {
        analogWrite(11,110);
      }
      else if(output == 1)
      {
        analogWrite(11,120);
      }
      else if(output == 2)
      {
        analogWrite(11,130);
      }
      Serial.println(output);
    }
    //increment timer for debug purpose
    else
    {
      timer = timer +1;
    }
  }
  //end of debug section
}
//interrupt at the rising edge of signal at pin 2
void rising1() {
  attachInterrupt(0, falling1, FALLING);
  prev_time = micros();
}
//interrupt method at the falling edge of signal at pin 2
void falling1() {
  pwm_value = micros()-prev_time;
  //now action based on pwm value
  
  //change value only if value is repeatedly outside range
  if(pwm_value > tolval + tolerance || pwm_value < tolval - tolerance)
  {
    toltimes++;
  }
  if(toltimes > tolmax)
  {
    //new value is written
    tolval = pwm_value;
   //choose action based on pwm_value. See line 19 for details.
   if(pwm_value > refuse_min)
    {
      if(pwm_value < low_ceil) // turn left? (check actual dir)
      {
        if(choice != -1)
        {
          delayMicroseconds(spd);
          choice = -1;
        }
        
      }
      else if (pwm_value < med_ceil) // hold
      {
        if(choice != 0)
        {
          delayMicroseconds(spd);
          choice = 0;
        }
      }
      else if(pwm_value < refuse_max)// turn right?
      {
        if(choice != 1)
        {
          delayMicroseconds(spd);
          choice = 1;
        }
      }
      //print if debug is on
      if(debug != 0)
        Serial.println(pwm_value);
    }
  }
  //reattach next rising edge interrupt
  attachInterrupt(0, rising1, RISING);
}
//end of code. Thank you for reading.
