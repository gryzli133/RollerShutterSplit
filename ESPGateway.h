#define MY_GATEWAY_ESP8266

#define MY_WIFI_SSID "WiFi_SSID"                   //need to be adjusted!!!
#define MY_WIFI_PASSWORD "WiFi_PASSWORD"           //need to be adjusted!!!


// Enable UDP communication
//#define MY_USE_UDP

// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.

#define MY_HOSTNAME "ESP8266_MySensorsGW"

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)

#define MY_IP_ADDRESS 192,168,0,214                   //need to be adjusted!!!

// If using static ip you need to define Gateway and Subnet address as well

#define MY_IP_GATEWAY_ADDRESS 192,168,0,1             //need to be adjusted!!!
#define MY_IP_SUBNET_ADDRESS 255,255,255,0            //need to be adjusted!!!

// The port to keep open on node server mode
#define MY_PORT 5003

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 5                      //need to be adjusted!!!

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68

#define MY_DEFAULT_ERR_LED_PIN 2  // Error led pin
#define MY_DEFAULT_RX_LED_PIN  2  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  2  // the PCB, on board LED

#if defined(MY_USE_UDP)
#include <WiFiUdp.h>
#endif

#include <ESP8266WiFi.h>

