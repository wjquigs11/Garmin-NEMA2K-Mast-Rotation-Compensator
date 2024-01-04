/*
 Demo: NMEA0183 library. NMEA0183 -> NMEA2000
   Reads messages from NMEA0183 and forwards them to the N2k bus
   Also forwards all NMEA2000 bus messages to the PC (Serial)

 This example reads NMEA0183 messages from one serial port. It is possible
 to add more serial ports for having NMEA0183 combiner functionality.

 The messages, which will be handled has been defined on NMEA0183Handlers.cpp
 on NMEA0183Handlers variable initialization. So this does not automatically
 handle all NMEA0183 messages. If there is no handler for some message you need,
 you have to write handler for it and add it to the NMEA0183Handlers variable
 initialization. If you write new handlers, please after testing send them to me,
 so I can add them for others use.
*/

#define N2K_SOURCE 15
#define ESP32_CAN_RX_PIN GPIO_NUM_26
#define ESP32_CAN_TX_PIN GPIO_NUM_27
// maybe don't need to do this?
#define USE_N2K_CAN 7  // for use with ESP32


#include <Arduino.h>
#include <Time.h>
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>
#include "NMEA0183Handlers.h"
#include "BoatData.h"
#include "SPI.h"
#include "SD.h"
#include "Wire.h"

#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object

#define NMEA0183SourceGPSCompass 3
#define NMEA0183SourceGPS 1

tBoatData BoatData;

tNMEA0183 NMEA0183_232, NMEA0183_422;

#include <HardwareSerial.h>
// set up the RS232 module
HardwareSerial SerialRS232(1);
// ICOM M330 NMEA output wires
// Brown = Talker B/Data L/Data -
// White = Talker A/Data H/Data +
#define RX 16 // GPIO 16 == RX2
#define TX -1
#define BAUD 9600 // NMEA 0183 may be 4800
#define SERBUF 1024
// set up the RS422 module
HardwareSerial SerialRS422(2);
#define RX422 19
// A = R+ // B = R- // Z = T- // Y = T+
// Some people simply wire the RS422 to RS232 by using xmit+/receiv+ and GND rather than xmit+/receiv+ and xmit-/receiv-. 

void SendSystemTime();

void doSDstuff();

void setup() {

  // Setup NMEA2000 system
  Serial.begin(115200);
  while (!Serial) delay(100); delay(500);
  Serial.println("ready...");

  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);  
/*
  while (!SD.begin(SS)){
      Serial.println("Card Mount Failed");
      delay(100);
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  //doSDstuff();
*/
  NMEA2000.SetProductInformation("00000008", // Manufacturer's Model serial code
                                 107, // Manufacturer's product code
                                 "NMEA0183 -> N2k -> PC",  // Manufacturer's Model ID
                                 "1.0.0.1 (2015-11-18)",  // Manufacturer's Software version code
                                 "1.0.0.0 (2015-11-18)" // Manufacturer's Model version
                                 );
  // Det device information
  NMEA2000.SetDeviceInformation(8, // Unique number. Use e.g. Serial number.
                                130, // Device function=PC Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );

  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.
  NMEA2000.SetForwardSystemMessages(true);
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,25);
  //NMEA2000.EnableForward(false);
  NMEA2000.Open();

  // Setup NMEA0183 ports and handlers
  InitNMEA0183Handlers(&NMEA2000, &BoatData);
  DebugNMEA0183Handlers(&Serial);
  NMEA0183_232.SetMsgHandler(HandleNMEA0183Msg);

  SerialRS232.setRxBufferSize(SERBUF);
  SerialRS232.begin(BAUD, SERIAL_8N1, RX, TX);
  //NMEA0183_232.SetMessageStream(&SerialRS232);
  //NMEA0183_232.Open();
  Serial.println("232 open");

  SerialRS422.setRxBufferSize(SERBUF);
  SerialRS422.begin(BAUD, SERIAL_8N1, RX422, TX);
  NMEA0183_422.SetMessageStream(&SerialRS422);
  NMEA0183_422.Open();
  Serial.println("422 open");

}

void loop() {
  NMEA2000.ParseMessages();
  //NMEA0183_232.ParseMessages();
  NMEA0183_422.ParseMessages();

  
  SendSystemTime();
}

#define TimeUpdatePeriod 1000

void SendSystemTime() {
  static unsigned long TimeUpdated=millis();
  tN2kMsg N2kMsg;

  if ( (TimeUpdated+TimeUpdatePeriod<millis()) && BoatData.DaysSince1970>0 ) {
    SetN2kSystemTime(N2kMsg, 0, BoatData.DaysSince1970, BoatData.GPSTime);
    TimeUpdated=millis();
    NMEA2000.SendMsg(N2kMsg);
  }
}


