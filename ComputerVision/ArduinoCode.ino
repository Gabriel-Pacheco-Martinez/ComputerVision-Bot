/*----------------------------------
 * Raspberry Pi Computer Vision     |
 * This code will receive info      |
 * from the Raspberry to control    |    
 * the motors                       |
 *                                  |
 * Gabriel Pacheco                  |
 * Electronic Engineering           |
 * ---------------------------------
 */

//Library to use I2C communication
#include <Wire.h>
  
//Defining control for the motor pins
#define PWMa 7
#define PWMb 5
#define PWMc 8
#define PWMd 6


void setup() 
{
  //Start the serial monitor
  Serial.begin(9600);

  //Set Arduino up as an I2C slave for the raspberry at address 0x07
  Wire.begin(0x07); 
  
  //Prepare to receive data
  Wire.onReceive(receiveEvent);
}

void loop()
{
  delay(300);
}

/*-------------------------------------------------------------------------FUNCTIONS------------------------------------------------------------*/
int receiveEvent(int numBytes)
{
  //Define variables needed to receive the integer sent from the Raspberry
  int receive_int=0;
  int count=0;
  int shapeCount=0;

  //Loop to receive ine byte till there are none left
  while(Wire.available())
  {
    char c = Wire.read();    // receive a byte as character
    //Create Int from the Byte Array
    receive_int = c << (8 * count) | receive_int;
    count++;
  }
  
  //Print the value received for debugging and verification
  Serial.print("Received Number: "); 
  Serial.println(receive_int);

  //When the value received is 1 the EEE-bot must go backwards
  if (receive_int == 1) 
  {
    Serial.println("Black S Shape Detected");
    Backwards(170);
    delay(shapeCount*1000);
    //Stop();
  }
  
  else if (receive_int ==2)
  {
    LeftTurn(170,10);
    delay(500);
    Stop();
  }
  else if (receive_int ==3)
  {
    RightTurn(170,10);
    Stop();
  }
  else if (receive_int ==4)
  {
    Serial.println("Green S Shape Detected");
    RightTurn(170,10);
    Stop();
  }
  else if (receive_int ==5)
  {
    RightTurn(170,10);
    Stop();
  }
  else if (receive_int == 6)
  {
    Serial.println("Distance Measurement Shape Detected");
    Forwards(150);
    delay(shapeCount*1000);
    Stop();
  }
  else 
  {
    shapeCount=receive_int;
    Serial.print("Shape Count: ");
    Serial.println(shapeCount); 
  }
}

//------------------------------------------------------------------------MotorFunctions-----------------------------------------------------------------
void Forwards(int16_t baseSpeed)
{
  Serial.println("Forwards");
  constrain(baseSpeed,0,255);
  digitalWrite(PWMa,LOW);
  analogWrite(PWMb,baseSpeed);
  digitalWrite(PWMc,HIGH);
  analogWrite(PWMd,HIGH-(baseSpeed));
}

void Backwards(int16_t baseSpeed)
{
  Serial.println("Backwards");
  constrain(baseSpeed,0,255);
  digitalWrite(PWMa,HIGH);
  analogWrite(PWMb,HIGH-(baseSpeed));
  digitalWrite(PWMc,LOW);
  analogWrite(PWMd,baseSpeed);
}

//Function to make the Bot go to the left
void LeftTurn(int16_t baseSpeed,float m)
{
  Serial.println("Left Turn");
  constrain(baseSpeed,0,255);
  
  //Right wheel forwards
  digitalWrite(PWMa,LOW);
  analogWrite(PWMb,baseSpeed);
  //Left wheel backwards
  digitalWrite(PWMc,LOW);
  analogWrite(PWMd,baseSpeed-m);
}

//Function to make the Bot go to the right
void RightTurn(int16_t baseSpeed,float m)
{
  Serial.println("Right Turn");
  constrain(baseSpeed,0,255);
  
  //Right wheel backwards
  analogWrite(PWMb,255-(baseSpeed-m));
  digitalWrite(PWMa,HIGH);
  //Left wheel forwards
  analogWrite(PWMd,255-(baseSpeed));
  digitalWrite(PWMc,HIGH);
}

//Function to stop all motors
void Stop()
{
  Serial.println("Stop");
  digitalWrite(PWMa,LOW);
  digitalWrite(PWMb,LOW);
  digitalWrite(PWMc,LOW);
  digitalWrite(PWMd,LOW);
  delay(5);
}
