/*

  Solis Comms
  https://github.com/RichardL64
  
  Talk to RS485/Modbus port on a Solis inverter and make available via HTTP protocol

  R.A.Lincoln     July 2022

  Poll the inveter for values periodically
  Respond to web clients requesting the latest address data

  Maintain a list of requested address (from any client)

  Webserver content:

  /                               'Hello world' base page

  /H                              Turn the onboard LED on
  /L                              Turn the onboard LED off

  /R?address=<value>,<value>...   Return the register addresse values as JSON JS array
                                  <value>.1 or <value>.2 indicates single/double address registers

  /S?address=<value>              Stop returning a register address

  /S?all=                         Stop returning all register addresses

  /C                              Output the register address cache

  e.g.
  Request
  GET	R/?addr=33057.2,33070

  Response JSON parsable JS object with address/data pairs
	{
	  “33057": 0,
    "33070": 0
	}


 */
#include <SPI.h>
#include <WiFiNINA.h>

//  WIFI server
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);

//  Register cache
#include "RegisterCache.h"

//  Webserver
#include "WebServer.h"


//  Register collection
#define COLLECT_MILLIS (unsigned long)1000*60 // Collect data every 60 seconds



/*
    Setup
*/
void setup() {
  Serial.begin(9600);            // initialize serial communication

  pinMode(LED_BUILTIN, OUTPUT);  // set the LED pin mode

  // check for the WiFi module:
  //
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);                     // don't continue
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  //
  Serial.print("Attempting to connect to Network named: ");
  Serial.println(ssid);               // print the network name (SSID);
  do {
    status = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network.
  } while (status != WL_CONNECTED);

  server.begin();                     // start the web server on port 80
  printWifiStatus();                  // you're connected now, so print out the status
}


/*
    Main loop
*/
void loop() {
  static unsigned long lastCollect;

  //  Periodic data collection
  //  Copy down register values listed in the cache
  //
  if(lastCollect + COLLECT_MILLIS < millis()) {             // Periodically deiven by COLLECT_MILLIS
    lastCollect = millis();

    for(int i = 0; i < CACHE_SIZE; i++) {                   // Process all cache entries
      if(cacheAddress[i] != 0 && isFresh(i)) {              // If the entry was requested recently
        Serial.print("Collect R:");
        Serial.println(cacheAddress[i]);
        setRegister(cacheAddress[i], millis());             // dummy value for testing
      }
    }
  }


  //  Webserver
  //  Service HTTP requests formatting into lines for parsing
  //
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Serial.println("new client");
    parseLine(client, nextLine(client));    // drive from the first line of each request
    client.stop();                          // client disconnected
    Serial.println("client disconnected");
  }
}



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

