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

  /dashboard                      Real time gauge dashboard of Inverter information
  /

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
#include "arduino_secrets.h"

//  Used in AP mode for setup
#define AP_SSID "solis-setup"
#define AP_PASSWORD "admin"

//  Used in mDNS discovery - i.e. hostname.local or bonjour name
#define HOSTNAME "solis"
#define SERVICENAME "solis._http"

//  Data collection - ms between register requests
#define MODBUS_DELAY 1

/*
 * Global objects
 * 
 */
//  WIFI server
int status = WL_IDLE_STATUS;
WiFiServer server(80);

//  mDNS service broadcast
WiFiUDP udp;
MDNS mdns(udp);


/*
 * 
 *  Setup
 *  May be re-called by the main loop if connectivity drops
 *  
 */
void setup() {
  Serial.begin(9600);                               // initialize serial communication
  
  pinMode(LED_BUILTIN, OUTPUT);                     // LED control
  digitalWrite(LED_BUILTIN, HIGH);                  // LED lit during setup - should go out if sucessful

  String fv = WiFi.firmwareVersion();               // Wifi Firmware check
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  randomSeed(analogRead(0));                        // Random numbers used in test data generation

  setupWiFi();                                      // Bring WiFi and mDNS up

  Serial.println("Webserver begin");                // Bring Webserver up
  server.begin();                              

  Serial.println("Modbus begin");                   // Bring Modbus up
  if (!ModbusRTUClient.begin(9600)) {      
    Serial.println("Modbus RTU Client start failed");
    while (true);                                   // lockup
  }

  digitalWrite(LED_BUILTIN, LOW);
}

/*
 * Setup
 * May be called more than once at startup and if the wifi goes down
 * 
 */
void setupWiFi() {
  
  Serial.println("WiFi begin");                     // Bring WiFi up
  while(WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(4000);
  }
  
  Serial.println("mDNS begin");                     // Advertise as HOSTNAME
  mdns.begin(WiFi.localIP(), HOSTNAME);
  mdns.addServiceRecord(SERVICENAME, 80, MDNSServiceTCP);

  printWifiStatus();
}
 
/*
 * 
 *  Main lop
 * 
 */
void loop() {

  //  Connectivity
  //
  if(WiFi.status() != WL_CONNECTED) setupWiFi();             // re-connect if disconnected
  mdns.run();
  serviceWiFi();

  //  Data collection
  //  Copy down register data values listed in the cache, one entry per loop
  //
  static unsigned long lastCollect;
  static int index;
  
  cacheAgeCheck(index);                                     // cull unaccessed cache entries

  int address =  cacheAddress[index];
  if(address != 0
    && (millis() - lastCollect > MODBUS_DELAY)) {           // if there is an address to collect & not too frequent
    lastCollect = millis();

    Serial.print(address);

    int size = cacheSize[index];
    if(!ModbusRTUClient.requestFrom(1, INPUT_REGISTERS, address, size)) {
        setCache(index, 0, DATA_ERROR);

        Serial.println(" = Data error");

    } else {                                                // else good data
        long data = 0;
        if(size == 2) data = ModbusRTUClient.read() <<16;   // 32 bit High 16
        data |=  ModbusRTUClient.read();                    // Low 16 bits
        setCache(index, data);

        Serial.print(" = ");
        Serial.println(data);
    }
  }
  if(address == 101) setCache(index, random(0, 1000));      // Random number test data on 101  
  
  index++;                                                  // Next cache entry for next loop
  index %= CACHE_SIZE;                                      // Limit 0 ... CACHE_SIZE -1  

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

//  Example Arduino reset code
//
//  void(* resetFunc) (void) = 0;
//
