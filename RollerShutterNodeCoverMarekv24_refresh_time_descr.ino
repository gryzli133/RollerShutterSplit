// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
//#define MY_BAUD_RATE 9600

#define MY_TRANSPORT_WAIT_READY_MS 1

// uncomment if we want to manually assign an ID
#define MY_NODE_ID 3


// !!!! CHOOSE ONLY ONE !!!!

//#include "ESPGateway.h" // for ESP8266 WiFi gateway -> set your WiFi in ESPGateway.h tab!
#include "SerialGateway.h" // for Serial gateway
//#include "RF24Gateway.h" // for RF24 radio gateway
//#include "MQTTGateway.h" // for MQTT Ethernet gateway



#include <Bounce2.h>
#include <PMButton.h>
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

bool service = 0;
int servicePin = 34;
int serviceLedPin = 35; // built-in LED

RollerShutter blinds[] =
{
  //A11
  {1, 21, 41, 61, 22, 23, 38, 39, 21, 20, 21, 50, 0, "Roleta w kuchni"},
  {2, 22, 42, 62, 24, 25, 40, 41, 30, 29, 30, 50, 0, "Roleta w kuchni"},
  {3, 23, 43, 63, 26, 27, 42, 43, 30, 29, 30, 50, 0, "Roleta w kuchni w oknie otwieranym"},
  {4, 24, 44, 64, 28, 29, 44, 45, 30, 29, 30, 50, 0, "Roleta w kuchni w oknie otwieranym"},
  {5, 25, 45, 65, 30, 31, 46, 47, 31, 30, 31, 50, 0, "Roleta HS lewa"},
  {6, 26, 46, 66, 32, 33, 48, 49, 20, 19, 20, 50, 0, "Roleta HS lewa"},
  {7, 27, 47, 67, 34, 35, 50, 51, 17, 16, 17, 50, 0, "Roleta HS prawa"},
  {8, 28, 48, 68, 36, 37, 52, 53, 17, 16, 17, 50, 0, "Roleta HS prawa"},
  {9, 29, 49, 69, A0, A1, A8, A9, 19, 18, 19, 50, 0, "Roleta Garaż 1"},
  {10, 30, 50, 70, A2, A3, A10, A11, 19, 18, 19, 50, 0, "Roleta Garaż 2"}
};

const int blindsCount = sizeof(blinds) / sizeof(RollerShutter);

void setup() 
{ 
  // Service Mode input = D2 -> only if pass-through mode is needed, f.e. for programming the build-in motor's limit switches.
  pinMode(servicePin, INPUT_PULLUP);
  service = !digitalRead(servicePin);
  pinMode(serviceLedPin, OUTPUT);
  digitalWrite(serviceLedPin, service);
  
  // Setup locally attached sensors
  wait(5000);
  for(int i = 0; i < blindsCount; i++)
  {
    blinds[i].SyncController(); 
    if(service)
    {
      blinds[i].enterServiceMode(); 
    }
  }
}

void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("RollerShutter_v23", "23.0");
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
