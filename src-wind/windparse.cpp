//

#include <Arduino.h>

//#include <ActisenseReader.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <N2kMessages.h>
#include <NMEA2000_esp32.h>
#include <ReactESP.h>
#include <Wire.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <vector>
#include <numeric>
#include <movingAvg.h>
#include <SPI.h>
#include "windparse.h"

int mastRotate, rotateout;
// analog values from rotation sensor
int PotValue=0;
int PotLo=9999;
int PotHi=0;

// Initialize static variables for RotationSensor Class
int RotationSensor::newValue{0};
int RotationSensor::oldValue{0};

double WindSensor::windSpeedKnots{0.0};
int WindSensor::windAngleDegrees{0};

movingAvg honeywellSensor(10);

extern tNMEA2000 *nmea2000;
extern HardwareSerial ESPlink;

#define PRBUF 32

extern volatile bool new_data;
void IRAM_ATTR NewDataReadyISR() {
  new_data = true;
}

int readAnalogRotationValue(){      //returns mastRotate value when called. This is in degrees, and corresponds to the current value of the Honeywell sensor

  // Define Constants
  const int lowset = 290;
  const int highset = 4095;
  // observed sensor range 290..4095 (new resistor)
  
  PotValue = analogRead(POT_PIN);
  if (!PotValue)
    return 0;
  // determine range of A2D values; this might be different on your boat
  if (PotValue < PotLo && PotValue > 0) { 
    PotLo = PotValue;
  } else if (PotValue > PotHi) {
    PotHi = PotValue;
  }
  #ifdef DEBUG
  Serial.println("potvalue: ");
  Serial.println(PotValue);
  #endif
  int newValue = honeywellSensor.reading(PotValue);    // calculate the moving average
  int oldValue = RotationSensor::oldValue;
  
  if (newValue < highset){                 //writes value to oldsensor if below highset threshold
    oldValue = newValue;
  }
  // Update values for new and old values (for the next loop iteration)
  RotationSensor::newValue = newValue;
  RotationSensor::oldValue = oldValue;

  int mastRotate = map(oldValue, lowset, highset, -50, 50);    //maps 10 bit number to degrees of rotation
  return mastRotate; 
}

tN2kMsg correctN2kMsg;

typedef union {
  float floatP;
  byte binary[4];
} binaryFloat;

binaryFloat speed, angle;

void WindSpeed(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  char prbuf[PRBUF];
  double windSpeedMeters;
  double windSpeedKnots;
  double windAngleRadians;
  int windAngleDegrees;
  tN2kWindReference WindReference;

    if (ParseN2kWindSpeed(N2kMsg,SID, windSpeedMeters, windAngleRadians, WindReference) ) {
      windSpeedKnots =  windSpeedMeters * 1.943844; // convert m/s to kts
      float windAngleDegrees = windAngleRadians * (180/M_PI);
      WindSensor::windSpeedKnots = windSpeedKnots;
      WindSensor::windAngleDegrees = windAngleDegrees;
      Serial.print("windSpeedKnots: ");
      Serial.print(windSpeedKnots);
      Serial.print(" windangledegrees: ");
      Serial.print(windAngleDegrees);
      // read rotation value and correct
      int mastRotate = readAnalogRotationValue();
      Serial.print(" mastRotate: ");
      Serial.print(mastRotate);
      float anglesum = windAngleDegrees - mastRotate;
      float rotateout=0.0;
      if (anglesum<0) {                             //ensure sum is 0-359
        rotateout = anglesum + 360;
      } else if (anglesum>359) {   
        rotateout = anglesum - 360;               
      } else {
        rotateout = anglesum;               
      }
      Serial.print(" rotateout: ");
      Serial.println(rotateout);
      speed.floatP = windSpeedKnots;
      angle.floatP = rotateout;
      ESPlink.write(speed.binary,4);
      ESPlink.write(angle.binary,4);
     }
     
}

double ReadWindAngle(int rotateout) {
  return DegToRad(rotateout); // 
}

double ReadWindSpeed() {
  return WindSensor::windSpeedKnots;  //Read the parsed windspeed from NMEA stream
}

int readWindAngleInput() {
  return  WindSensor::windAngleDegrees;
}

/*
void SendN2kWind(int rotateout) {
  static unsigned long WindUpdated=millis();
  tN2kMsg N2kMsg;

  if ( WindUpdated+WindUpdatePeriod<millis() ) {
    SetN2kWindSpeed(N2kMsg, 1, ReadWindSpeed(), ReadWindAngle(rotateout),N2kWind_Apparent); 
    WindUpdated=millis();
    nmea2000->SendMsg(N2kMsg);
  }
}
*/