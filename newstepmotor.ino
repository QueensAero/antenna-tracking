//Code written by Allen Mitro, for the use of the
//Queen's Aero Design Team.

//For any other purpose, the following code is provided on
//an AS-IS basis, and comes without any warranties, express or implied.

//Latest recorded revision: Apr 3 2019

//Ardutracker related code
String cmd = "";
String panStr = "";
String tiltStr = "";
#define led 13
long panVal = 0L;
long tiltVal = 0L;
long interpVal = 0L;
int tiltDelay = 15;

//This function controls a stepmotor

//pins for controlling motor 1
#define M1_en 7
#define M1_dr 8
#define M1_pl 9

//PWM tolerance values: ignore values until
//tolmax # of readings within tolerance are recorded.
#define tolmax 1
#define tolerance 3

//Three control states are possible. Left, Stop or Right Rotation.
//PWM values under refuse_min or over refuse_max are discarded.
//range of 0 and 360 between these values.
#define refuse_min 900
#define refuse_max 1600
//number of microsteps on the stepper motor
#define totalsteps 6400

//Directional servo values
volatile int rotatedir = -1000;//default is -1000 so that it is set later
int currentdir = -100;

//Activates debug features if not zero.
//>>Outputs periodically changing PWM values from pin 11.
//>>Periodically displays latest PWM reading and intended pin 11 direction.
#define debug 1
#define debugpwmpin 11

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
    Serial.begin(115200);
    Serial.println("Started.");
}

//Main loop of code. Checks whether goal is different than current.
//first valid position will be assumed to be the correct direction.
//
void loop()
{
  //serial functions.
  if (Serial.available()) 
  {
    char ch = Serial.read();

    if (ch == 10)
    {
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
      
      // Line feed is the command char
      if (cmd.startsWith("!!!PAN:"))
      {
        panStr = cmd.substring(7,11);
      
        if (cmd.substring(12,16) == "TLT:")
        {
          tiltStr = cmd.substring(16,20);
        }

        if (panStr != "")//detected pan value.
        {
           panVal = panStr.toInt();
           //change pan pwm based on value.
           adjustpwm(panVal);
        }
         
           
      
        if (tiltStr != "")
        {
           tiltVal = tiltStr.toInt();
        }
      }
      else if (cmd.startsWith("!!!TLTDLY:"))
      {
        cmd = cmd.substring(10,14);
        tiltDelay = cmd.toInt();
      }
      
      panStr = "";
      tiltStr == "";
      cmd = "";

      digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW         
    }
    else
    {
      cmd += ch;
    }
  }
  //set starting position to first valid goal direction.
  if(currentdir == -100 && rotatedir != -1000)
  {
     currentdir = rotatedir;
     choice = 0;
  }
  else if(currentdir == rotatedir || rotatedir == -1000) //now stop.
  {
     choice = 0;
  }
  else if(rotatedir - currentdir > 0 && (rotatedir - currentdir) < totalsteps/2 || rotatedir - currentdir < -1*totalsteps / 2)
  { //this is case if it is faster to decrement (go CCW)
     if(debug>1)
      {
        Serial.print("W");
        Serial.println(rotatedir - currentdir);
      }
     choice = 1;
  }
  else
  { //otherwise go CW
     choice = -1;
     if(debug>1)
      {
        Serial.print("C");
        Serial.println(rotatedir - currentdir);
      }
  }
  //retaining old continuous servo code

  //choice indicates go left.
  if(choice == -1)
  {
    //update flags and pins. We don't want repeated changes.
    if(dirselect)
    {
      dirselect = false;
      digitalWrite(M1_dr,HIGH);
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
      digitalWrite(M1_dr,LOW);
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
  
    if(debug > 1 && choice != 0)
    {
        Serial.print("choice: ");
        Serial.print(choice);
        Serial.print(" current ");
        Serial.print(currentdir);
        Serial.print(" goal ");
        Serial.println(rotatedir);
    }
  //end of debug section
  if(currentdir != -100 && choice != 0) //update currentdir if needed
  {
    currentdir = (currentdir + totalsteps + choice) % totalsteps;
  }
}

//changed to interperet serial input values.
void adjustpwm(long pwm_value) {
  //now action based on pwm value
  
  //change value only if value is repeatedly outside range
  if(pwm_value > tolval + tolerance || pwm_value < tolval - tolerance)
  {
    toltimes++;
  }
  else
  {
    toltimes = 0;
  }
  //added check for !=pwm_value to reduce constant writing.
  if(toltimes > tolmax && tolval!= pwm_value)
  {
    //new value is written
    tolval = pwm_value;
   //choose action based on pwm_value. See line 19 for details.
   if(pwm_value > refuse_min)
    {
      if(pwm_value < refuse_max) // Valid turn signal
      {
          rotatedir = (int)((pwm_value - refuse_min) * (long)totalsteps /(refuse_max - refuse_min));
          
      }
    }
    if(debug!=0)
      {
        float calcangle = rotatedir*360.0/totalsteps;
        Serial.print("Received PWM: ");
        Serial.print(pwm_value);
        Serial.print(" -dir ");
        Serial.println(rotatedir);
        Serial.print("Calc angle: ");
        Serial.println(calcangle);
      }
  }
  //reattach next rising edge interrupt
  //attachInterrupt(0, rising1, RISING);
}
//end of code. Thank you for reading.
