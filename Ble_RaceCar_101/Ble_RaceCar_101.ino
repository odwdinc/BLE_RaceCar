/*
 * Copyright (c) 2016 Intel Corporation.  All rights reserved.
 * See the bottom of this file for the license terms.
 */

#include <CurieBLE.h>

BLEPeripheral blePeripheral; // create peripheral instance
BLEService MotorService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service
BLECharacteristic LeftMotorCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite,2);
BLECharacteristic RightMotorCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite,2);


int pwm_a = 3;   //PWM control for motor outputs 1 and 2 is on digital pin 3
int pwm_b = 11;  //PWM control for motor outputs 3 and 4 is on digital pin 11
int dir_a = 12;  //direction control for motor outputs 1 and 2 is on digital pin 12
int dir_b = 13;  //direction control for motor outputs 3 and 4 is on digital pin 13
int val = 0;     //value for fade




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
  blePeripheral.addAttribute(LeftMotorCharacteristic);
  blePeripheral.addAttribute(RightMotorCharacteristic);

  LeftMotorCharacteristic.setValue(MotorData,2);
  RightMotorCharacteristic.setValue(MotorData,2);

  // advertise the service
  blePeripheral.begin();

  Serial.println("Bluetooth device active, waiting for connections...");

    // assign event handlers for characteristic
  LeftMotorCharacteristic.setEventHandler(BLEWritten, LeftMotorCharacteristicWritten);
  RightMotorCharacteristic.setEventHandler(BLEWritten, RightMotorCharacteristicWritten);

  
  forw();         //Set Motors to go forward Note : No pwm is defined with the for function, so that fade in and out works
  fadein();       //fade in from 0-255
  fadeout();      //Fade out from 255-0
  stopped();      // stop for 2 seconds
}

void loop() {
  // poll peripheral
  blePeripheral.poll();
}

void RightMotorCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic){
    Serial.println("Right Motor Characteristic Written");

  if(characteristic.value()[1] == 1){
    aforw();
  }else if (characteristic.value()[1] == 2){
    aback();
  }else{
    astop();
  }
  apwm(characteristic.value()[0]);
}



void LeftMotorCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic){
  Serial.println("Left Motor Characteristic Written");
  if(characteristic.value()[1] == 1){
    bforw();
  }else if (characteristic.value()[1] == 2){
    bback();
  }else{
    bstop();
  }
  bpwm(characteristic.value()[0]);
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


