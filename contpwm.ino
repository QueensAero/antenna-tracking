#define M1_en 9
#define M1_pl 8
#define M1_dr 7
#define tolmax 5
#define tolerance 10
#define refuse_min 800
#define low_ceil 900
#define med_ceil 1000
#define refuse_max 2000
#define debug 1
volatile int choice = 0;
boolean dirselect = false;
boolean enaselect = false;
volatile int prev_time = 0;
volatile int pwm_value = 0;
volatile int spd = 100;
int timer = 0;
int output = 0;


volatile int tolval = 0;

volatile int toltimes = 0;

void setup()
{
  pinMode(M1_en, OUTPUT);
  pinMode(M1_pl, OUTPUT);
  pinMode(M1_dr, OUTPUT);
  digitalWrite(M1_en,LOW);
  digitalWrite(M1_dr,LOW);
  digitalWrite(M1_pl,LOW);
  pinMode(11, OUTPUT);
  analogWrite(11,70);
  if(debug != 0)
    Serial.begin(115200);
  attachInterrupt(0, rising1, RISING);
}

void loop()
{
  if(choice == -1)
  {
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
    digitalWrite(M1_pl,HIGH);
    delayMicroseconds(1000);
    digitalWrite(M1_pl,LOW);
  }
  else if(choice == 0)
  {
    
     if(enaselect)
    {
      enaselect = false;
      digitalWrite(M1_en,LOW);
    }
    delayMicroseconds(1000);
  }
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
    delayMicroseconds(1000);
    digitalWrite(M1_pl,LOW);
  }
  if(debug != 0)
  {
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
    else
    {
      timer = timer +1;
    }
  }
}
void rising1() {
  attachInterrupt(0, falling1, FALLING);
  prev_time = micros();
}

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
    tolval = pwm_value;
   if(pwm_value > refuse_min)
    {
      if(pwm_value < low_ceil) // turn left? (check actual dir)
      {
        if(choice != -1)
        {
          delayMicroseconds(1000);
          choice = -1;
        }
        
      }
      else if (pwm_value < med_ceil) // hold
      {
        if(choice != 0)
        {
          delayMicroseconds(1000);
          choice = 0;
        }
      }
      else if(pwm_value < refuse_max)// turn right?
      {
        if(choice != 1)
        {
          delayMicroseconds(1000);
          choice = 1;
        }
      }
      
      if(debug != 0)
        Serial.println(pwm_value);
    }
  }
    attachInterrupt(0, rising1, RISING);
  
}
