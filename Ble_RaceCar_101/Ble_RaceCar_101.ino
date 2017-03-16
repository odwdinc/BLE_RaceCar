/*
 * Copyright (c) 2016 Intel Corporation.  All rights reserved.
 * See the bottom of this file for the license terms.
 */

#include <CurieBLE.h>

BLEPeripheral blePeripheral; // create peripheral instance
BLEService MotorService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service
BLECharacteristic MotorCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite,2);


int pwm_a = 3;   //PWM control for motor outputs 1 and 2 is on digital pin 3
int pwm_b = 11;  //PWM control for motor outputs 3 and 4 is on digital pin 11
int dir_a = 12;  //direction control for motor outputs 1 and 2 is on digital pin 12
int dir_b = 13;  //direction control for motor outputs 3 and 4 is on digital pin 13
int val = 0;     //value for fade

float rawLeft;
float rawRight;

float MaxJoy = 255;
float MinJoy = 0;

float MaxValue = 255;
float MinValue = 0;

float RawLeft;
float RawRight;

float ValLeft;
float ValRight;

bool invXL = false;
bool invXR = false;



uint8_t MotorData[] = {0x00, 0x00};

void setup() {
  pinMode(pwm_a, OUTPUT);  //Set control pins to be outputs
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);
  
  analogWrite(pwm_a, 100);  //set both motors to run at (100/255 = 39)% duty cycle (slow)
  analogWrite(pwm_b, 100);
  
  Serial.begin(9600);
  // set the local name peripheral advertises
  blePeripheral.setLocalName("RaceCar");
  // set the UUID for the service this peripheral advertises:
  blePeripheral.setAdvertisedServiceUuid(MotorService.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(MotorService);
  blePeripheral.addAttribute(MotorCharacteristic);

  MotorCharacteristic.setValue(MotorData,2);
  // advertise the service
  blePeripheral.begin();

  Serial.println("Bluetooth device active, waiting for connections...");
  MotorCharacteristic.setEventHandler(BLEWritten, MotorCharacteristicWritten);

}

void loop() {
  // poll peripheral
  blePeripheral.poll();
}

void MotorCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic){
    Serial.println("Right Motor Characteristic Written");
    CalculateTankDrive((float)characteristic.value()[0],(float)characteristic.value()[1]);
    
    Serial.print("Val: ");
    Serial.print(ValLeft);
    Serial.print(" : ");
    Serial.print(ValRight);
    Serial.println();

    if(ValLeft > 0){
      aforw();
    }else{
      aback();
    }
    if(ValRight > 0){
      bforw();
    }else{
      bback();
    }


    apwm(abs(ValLeft));
    bpwm(abs(ValRight));
    
}
//Dir Data
void aforw() // no pwm defined
{ 
  digitalWrite(dir_a, HIGH);  //Reverse motor direction, 1 high, 2 low
}

void bforw() // no pwm defined
{ 
  digitalWrite(dir_b, HIGH);  //Reverse motor direction, 3 low, 4 high  
}

void aback() // no pwm defined
{
  digitalWrite(dir_a, LOW);  //Set motor direction, 1 low, 2 high
}
void bback() // no pwm defined
{
  digitalWrite(dir_b, LOW);  //Set motor direction, 3 high, 4 low
}

//Pwm Data
void apwm(int speed){
  analogWrite(pwm_a, speed);   //set both motors to run at 100% duty cycle (fast)
}

void bpwm(int speed){
  analogWrite(pwm_b, speed);   //set both motors to run at 100% duty cycle (fast)
}


//Stop data
void astop()                   //stop motor A
{
  analogWrite(pwm_a, 0);    //set both motors to run at 100% duty cycle (fast)
}

void bstop()                   //stop motor B
{ 
  analogWrite(pwm_b, 0);    //set both motors to run at 100% duty cycle (fast)
}


/* Let's take a moment to talk about these functions. The forw and back functions are simply designating the direction the motors will turn once they are fed a PWM signal.
If you only call the forw, or back functions, you will not see the motors turn. On a similar note the fade in and out functions will only change PWM, so you need to consider
the direction you were last set to. In the code above, you might have noticed that I called forw and fade in the same grouping. You will want to call a new direction, and then
declare your pwm fade. There is also a stop function. 
*/


void CalculateTankDrive(float x, float y)
 {
   // first Compute the angle in deg
   // First hypotenuse
   float z = sqrt(x * x + y * y);

   // angle in radians
   float rad = acos(abs(x) / z);

   // Cataer for NaN values
   if(isnan(rad) == true){ rad=0; }

   // and in degrees
   float angle = rad * 180 / PI;

   // Now angle indicates the measure of turn
   // Along a straight line, with an angle o, the turn co-efficient is same
   // this applies for angles between 0-90, with angle 0 the co-eff is -1
   // with angle 45, the co-efficient is 0 and with angle 90, it is 1

   float tcoeff = -1 + (angle / 90) * 2;
   float turn = tcoeff * abs(abs(y) - abs(x));
   turn = round(turn * 100) / 100;

   // And max of y or x is the movement
   float mov = max(abs(y), abs(x));

   // First and third quadrant
   if ((x >= 0 && y >= 0) || (x < 0 && y < 0))
   { rawLeft = mov; rawRight = turn; }
   else
   { rawRight = mov; rawLeft = turn; }

   // Reverse polarity
   if (y < 0){ rawLeft = 0 - rawLeft; rawRight = 0 - rawRight; }

   // Update the values
   RawLeft = rawLeft;
   RawRight = rawRight;

   // Map the values onto the defined rang
   ValLeft = map(rawLeft, MinJoy, MaxJoy, MinValue, MaxValue);
   ValRight = map(rawRight, MinJoy, MaxJoy, MinValue, MaxValue);

   // Cater for inverse of direction if needed
   if(invXL){ RawLeft *= -1; ValLeft = MaxValue - ValLeft; }
   if(invXR){ RawRight *= -1; ValRight = MaxValue - ValRight; }
 }




void back() // no pwm defined
{
  digitalWrite(dir_a, LOW);  //Set motor direction, 1 low, 2 high
  digitalWrite(dir_b, LOW);  //Set motor direction, 3 high, 4 low
}

void forw() // no pwm defined
{ 
  digitalWrite(dir_a, HIGH);  //Reverse motor direction, 1 high, 2 low
  digitalWrite(dir_b, HIGH);  //Reverse motor direction, 3 low, 4 high  
}


void forward() //full speed forward
{ 
  digitalWrite(dir_a, HIGH);  //Reverse motor direction, 1 high, 2 low
  digitalWrite(dir_b, HIGH);  //Reverse motor direction, 3 low, 4 high  
  analogWrite(pwm_a, 255);    //set both motors to run at (100/255 = 39)% duty cycle
  analogWrite(pwm_b, 255);
}

void backward() //full speed backward
{
  digitalWrite(dir_a, LOW);  //Set motor direction, 1 low, 2 high
  digitalWrite(dir_b, LOW);  //Set motor direction, 3 high, 4 low
  analogWrite(pwm_b, 255);
}




void stopped() //stop
{ 
  digitalWrite(dir_a, LOW); //Set motor direction, 1 low, 2 high
  digitalWrite(dir_b, LOW); //Set motor direction, 3 high, 4 low
  analogWrite(pwm_a, 0);    //set both motors to run at 100% duty cycle (fast)
  analogWrite(pwm_b, 0); 
}

void fadein()
{ 
  // fade in from min to max in increments of 5 points:
  for(int fadeValue = 0 ; fadeValue <= 255; fadeValue +=5) 
  { 
     // sets the value (range from 0 to 255):
    analogWrite(pwm_a, fadeValue);   
    analogWrite(pwm_b, fadeValue);    
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);                            
  } 
}

void fadeout()
{ 
  // fade out from max to min in increments of 5 points:
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=5) 
  { 
    // sets the value (range from 0 to 255):
    analogWrite(pwm_a, fadeValue);
    analogWrite(pwm_b, fadeValue);
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);  
}
}


