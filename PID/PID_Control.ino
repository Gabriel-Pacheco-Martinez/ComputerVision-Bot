/*----------------------------------
 * PID control:                     |
 * This code aims to achieve        |
 * automatization of the EEE-Bot    |
 * using PID Control                |
 *                                  |
 * Gabriel Pacheco                  |
 * Electronic Engineering           |
 * ---------------------------------
 */

//Defining control pins
#define PWMa 7
#define PWMb 5
#define PWMc 8
#define PWMd 6

//IR Pins
float analogPin1 = A0;  //right most IR LED (blue wire)
float analogPin2 = A1;  //centre right IR LED (grey wire)
float analogPin3 = A2;  //centre left IR LED (white wire)
float analogPin4 = A3;  //left most IR LED (purple wire)
  
//PID Control variable
float u;

//Variable for the basespeed of the motors
float BaseSpeed=100;

//Define Delay Time
#define DelayTime 1000
/*----------------------------------------------------------------------------MAIN CODE----------------------------------------------------------------------------*/
void setup() 
{
  //Starting the serial monitor
  Serial.begin(9600);

  //Defining the motors as output
  pinMode(PWMa, OUTPUT);
  pinMode(PWMb, OUTPUT);
  pinMode(PWMc, OUTPUT);
  pinMode(PWMd, OUTPUT);

  //Defining the IR Pins as Input
  pinMode(analogPin1,INPUT);
  pinMode(analogPin2,INPUT);
  pinMode(analogPin3,INPUT);
  pinMode(analogPin4,INPUT);
  
}

void loop() 
{
  //Callback of the whole function PID which runs everything
  PID();
  
  //Starting the motors on a low so the system doesn't collapse
  digitalWrite(PWMa,LOW);
  digitalWrite(PWMb,LOW);
  digitalWrite(PWMc,LOW);
  digitalWrite(PWMd,LOW);
  delay(50);

  //If statement to define the velocity and direction of motors
  if ((u<0.5)&&(u>-0.5))
  {
    Forwards(BaseSpeed); //Callback to function to go forwards
  }
  else if (u>0.5)
  {
    RightTurn(BaseSpeed,u); //Callback to function to make a right turn
  }
  else if (u<-0.5)
  {
    LeftTurn(BaseSpeed,u); //Callback to function to make a left turn
  }
}

/*-----------------------------------------------------------------------------FUNCTIONS--------------------------------------------------------------------------*/

//Function to return the Weighted Average
float Weighted_Average()
{
  //Variable for weighted average
  float Xpk; 

  //Variables used for read ins
  float s1; //right most IR LED
  float s2; // centre right IR LED
  float s3; // centre left IR LED
  float s4; // left most IR LED
  
  //Read ins
  s1 = analogRead(analogPin1);//right most IR LED
  Serial.print("s1:");
  Serial.println(s1);
  s2 = analogRead(analogPin2); //centre right IR LED
  Serial.print("s2:");
  Serial.println(s2);
  s3 = analogRead(analogPin3); //centre left IR LED
  Serial.print("s3:");
  Serial.println(s3);
  s4 = analogRead(analogPin4); //left most IR LED
  Serial.print("s4:");
  Serial.println(s4);

  //Distances (defined from the left edge of the yellow board)
  float x1 = 55;  // right most IR LED    (eveything is in mm)
  float x2 = 40; // centre right IR LED
  float x3 = 25; // centre left IR LED
  float x4 = 9; // left most  IR LED

  //Calculate and return the value for the weighted average value
  Xpk=((x1 * s1) + (x2 * s2) + (x3 * s3) + (x4 * s4)) / (s1 + s2 + s3 + s4);
  Serial.print("Weighted Average:");
  Serial.println(Xpk);
  //delay(DelayTime);
  return Xpk;
}

//Function to return the value of u and achieve PID control
float PID ()
{
 //Values of Kp,Ki,Kd for the formula
  float Kp=10; //Proportional
  float Ki=0.1; //Integral
  float Kd=1; //Derivative
  
  //Variables to be used
  float setPoint=32.32; //Value of weighted average when the car is in the middle of the black line
  float XPK=Weighted_Average();
  float error;
  static float lastError=0;
  
  //Variables for the errors of each section of the formula
  float P; //Current error
  static float I; //Combination of errors
  float D; //Change in errors

  //Calculation of error with slight room for chnages without having to make the Bot do turns
  if ((XPK>32.30)&&(XPK<32.34))
  {
    error=0;
  }
  else
  {
    error=setPoint-XPK; //Obtaining error from the position of the Bot
  }
  
  
  //For formula
  P=error; 
  I=I+error; 
  D=error-lastError; 
  lastError=error;
  
  //Formula used over the same duration of time, so defined as
  u=Kp*P+Ki*I+Kd*D;
  Serial.print("u:");
  Serial.println(u);
  //Returning value of "u" that will be used in the loop
  return u;
}

//Function to make the BOT go forwards
void Forwards(int16_t baseSpeed)
{
  Serial.println("Forwards");
  constrain(baseSpeed,0,255);
  digitalWrite(PWMa,LOW);
  analogWrite(PWMb,baseSpeed);
  digitalWrite(PWMc,HIGH);
  analogWrite(PWMd,HIGH-(baseSpeed));
}

//Function to make the Bot go to the left
void LeftTurn(int16_t baseSpeed,float m)
{
  Serial.println("Left Turn");
  Serial.print("Value of m:"); 
  Serial.println(m);
  constrain(baseSpeed,-255,255);

  if(baseSpeed<0)
  {    digitalWrite(PWMa,HIGH);
    analogWrite(PWMb,255+baseSpeed);
  }
  else
  {
    //Right wheel speed increases
    digitalWrite(PWMa,LOW);
    analogWrite(PWMb,baseSpeed-m);
    
    //Left wheel speed decreases
    digitalWrite(PWMc,HIGH);
    analogWrite(PWMd,255-(baseSpeed+m));
  }
}

//Function to make the Bot go to the right
void RightTurn(int16_t baseSpeed,float m)
{
  Serial.println("Right Turn");
  Serial.print("Value of m:"); 
  Serial.println(m);
  constrain(baseSpeed,-255,255);

  if(baseSpeed<0)
  {
    digitalWrite(PWMd,HIGH);
    analogWrite(PWMc,LOW);
  }
  else
  {
    //Right wheel speed decreases
    analogWrite(PWMb,baseSpeed-m);
    digitalWrite(PWMa,LOW);
    
    //Left wheel speed increases
    analogWrite(PWMd,255-(baseSpeed+m));
    digitalWrite(PWMc,HIGH);
  }
}
