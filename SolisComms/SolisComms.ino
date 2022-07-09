/*

  SolisComms
  https://github.com/RichardL64
  
  Talk to RS485/Modbus port on a Solis inverter and make available via HTTP protocol

  R.A.Lincoln     July 2022

  Hardware:
    Arduino Nano 33 IOT

  Libraries required:
    SPI
    WiFiNINA
    MDNS_Generic
    Arduino_Modbus


  Poll the inveter for values periodically
  Respond to web clients requesting the latest address data

  Maintain a list of requested address (from any client)

  Webserver content:

  http://solis.local

  /                               'Hello world' base page

  /H                              Turn the onboard LED on
  /L                              Turn the onboard LED off

  /AP?ssid=<value>&password=<value>
                                  Access point mode - Change the network connection credentials

  /R?refresh=<seconds>&address=<value>,<value>...   
                                  Return the register addresse values as JSON JS array
                                  <value>.1 or <value>.2 for single/double address registers
                                  If the optional Refresh setting is passed the page will auto refresh

  /S?address=<value>              Stop returning a register address

  /S?all                          Stop returning all register addresses

  /C                              Output the register address cache

  e.g.
  Request
  GET	R/?addr=33057.2,33070

  Response JSON parsable JS object with address/data pairs
  Null value if no available data from the server
	{
	  “33057": 0,
    "33070": null
	}


 */



 /*
  * Issues
  * 
  * Signed longs! +  when to sign extend 16 bits
  * Browser refresh driven from client side eg. &refresh=5
  * 
  */
#include <SPI.h>
#include <WiFiNINA.h>         // (note _generic version locks up on closed connections)
#include <MDNS_Generic.h>
#include <ArduinoModbus.h>

#include "RegisterCache.h"
#include "WebServer.h"

//  Used in AP mode for setup
#define AP_SSID "solis-setup"
#define AP_PASSWORD "admin"

//  Used in mDNS discovery - i.e. hostname.local or bonjour name
#define HOSTNAME "solis"
#define SERVICENAME "solis._http"

//  Data collection - ms between register requests
#define COLLECT_GAP 20


//  WIFI server
#include "arduino_secrets.h"

//  retreive ssid/password from NINA storage here

char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);

//  mDNS service broadcast
WiFiUDP udp;
MDNS mdns(udp);



/*
 * 
 *  Setup
 *  
 */
void setup() {
  Serial.begin(9600);                 // initialize serial communication

  randomSeed(analogRead(0));          // random used in test data generation

  pinMode(LED_BUILTIN, OUTPUT);       // set the LED pin mode
  digitalWrite(LED_BUILTIN, HIGH);    // LED lit during setup - should go out if sucessful

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // start the web server on port 80
  server.begin();              

  // start the Modbus RTU client
  if (!ModbusRTUClient.begin(9600)) {
    Serial.println("Modbus RTU Client start failed");
    while (true);                       // spin
  }

  digitalWrite(LED_BUILTIN, LOW);
}


/*
 * 
 *  Main lop
 * 
 */
void loop() {
  static int state;

  //  Wifi connect/reconnect if lost
  //
  if(WiFi.status() != WL_CONNECTED) state = 0;  // detect unexpected disconnection

  switch(state) {
    case 0:                                     // not connected
      Serial.println("Connecting");
      WiFi.begin(ssid, pass);
      delay(1000);
      state++;
      break;
      
    case 1:                                     // connected, re publish mDNS etc
      printWifiStatus();     
                     
      Serial.println("mDNS update");
      mdns.begin(WiFi.localIP(), HOSTNAME);
      mdns.addServiceRecord(SERVICENAME, 80, MDNSServiceTCP);
      state++;
      break;
      
    default:
      break;
  }

  //  Data collection
  //  Copy down register data values listed in the cache, one entry per loop
  //
  static unsigned long lastCollect;
  static int index;
  cacheAgeCheck(index);                                     // cull unaccessed cache entries

  int address =  cacheAddress[index];
  if(address != 0
    && (lastCollect + COLLECT_GAP) < millis()) {            // if there is an address to collect & not too frequent
    lastCollect = millis();

    Serial.print(address);

    int size = cacheSize[index];
    if(!ModbusRTUClient.requestFrom(1, INPUT_REGISTERS, address, size)) {
        setCache(index, 0, DATA_ERROR);

        Serial.println(" Data error");

    } else {                                                // else good data
        long data = 0;
        if(size == 2) data = ModbusRTUClient.read() <<16;   // 32 bit High 16
        data |=  ModbusRTUClient.read();                    // Low 16 bits
        setCache(index, data);

        Serial.print(" = ");
        Serial.println(data);
    }
  }
  
  // Useful for offline testing, address 101 returns a random integer
  if(address == 101) setCache(index, random(0, 1000));   
  
  index++;                                                  // Increment for next loop
  index %= CACHE_SIZE;                                      // Limit 0 ... CACHE_SIZE -1


  //  mDNS
  //
  mdns.run();

  //  Webserver
  //
  if(state == 2) {
    serviceWiFi();
  }
  

  delay(1);
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
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("URL http://");
  Serial.print(HOSTNAME);
  Serial.println(".local");
}

//  Reset the arduino
//
void(* resetFunc) (void) = 0;
