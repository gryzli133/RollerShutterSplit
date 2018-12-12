// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

#define MY_TRANSPORT_WAIT_READY_MS 1

// uncomment if we want to manually assign an ID
#define MY_NODE_ID 20


// !!!! CHOOSE ONLY ONE !!!!

//#include "ESPGateway.h" // for ESP8266 WiFi gateway -> set your WiFi in ESPGateway.h tab!
#include "SerialGateway.h" // for Serial gateway
//#include "RF24Gateway.h" // for RF24 radio gateway



#include <Bounce2.h>
#include <MySensors.h>
#include <SPI.h>

#include "RollerShutter.h"

/* define your RollerShutter objects here
 RollerShutter(int childId      // Cover/Roller Shutter device ID for MySensors and Controller (Domoticz, HA etc)
              , int setIdUp     // Roll time UP setpoint device ID for MySensors and Controller (Domoticz, HA etc)
              , int setIdDown   // Roll time DOWN setpointdevice ID for MySensors and Controller (Domoticz, HA etc)
              , int initId      // Initialization device ID for MySensors and Controller (Domoticz, HA etc)
              , int buttonUp    // Button Pin for UP
              , int buttonDown  // Button Pin for DOWN
              , int relayUp     // Relay Pin for UP
              , int relayDown   // Relay Pin for DOWN
              , uint8_t initTimeUp          // Initial Roll time UP
              , uint8_t initTimeDown        // Initial Roll time DOWN
              , uint8_t initCalibrationTime // Initial calibration time (time that relay stay ON after reach 0 or 100%)
              , int debaunceTime            // Time to debounce button -> standard = 50
              , bool invertedRelay          // for High level trigger = 0; for Low level trigger = 1
              )
*/

RollerShutter blinds[] =
{
  {1, 21, 41, 61, 14, 15, 22, 23, 60, 55, 15, 50, 0},
  {2, 22, 42, 62, 16, 17, 24, 25, 60, 55, 15, 50, 0},
  {3, 23, 43, 63, 18, 19, 26, 27, 60, 55, 15, 50, 0},
  {4, 24, 44, 64, 20, 21, 28, 29, 60, 55, 15, 50, 0}
};

const int blindsCount = sizeof(blinds) / sizeof(RollerShutter);

void setup() 
{ 
  // Setup locally attached sensors
  delay(5000);
  for(int i = 0; i < blindsCount; i++)
  {
    blinds[i].SyncController(); 
  }
}

void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Cover by Marek", "20.0");
  for(int i = 0; i < blindsCount; i++)
  {
    blinds[i].Present(); 
  }
}

void loop() 
{ 
  for(int i = 0; i < blindsCount; i++)
  {
    blinds[i].Update(); 
  }
}

void receive(const MyMessage &message) {
  for(int i = 0; i < blindsCount; i++)
  {
    blinds[i].Receive(message); 
  }
}
