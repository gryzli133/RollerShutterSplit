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

// define your RollerShutter objects here
// RollerShutter(int childId, int setIdUp, int setIdDown, int buttonUp, int buttonDown, int relayUp, int relayDown, int debaunceTime, bool invertedRelay)
RollerShutter blinds[] =
{
  {0, 20, 40, 14, 15, 22, 23, 50, 0},
  {1, 21, 41, 16, 17, 24, 25, 50, 0},
  {2, 22, 42, 18, 19, 26, 27, 50, 0},
  {3, 23, 43, 20, 21, 28, 29, 50, 0}
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
  sendSketchInfo("RollerShutter as class object", "v18");
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
