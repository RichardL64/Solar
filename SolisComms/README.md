# SolisComms

Arduino application, built on NANO 33 IOT and RS485 board
Provides a WiFi webserver/HTTP interface to fucntionality described in SolisComms.ino

The hostname is registered with mDNS so can be accessed via http://solis.local in dynamic IP address environments.

Local network access only - no security.

The application ayschronously polls the Inverter via Modbus/RS485 for values of its internal registers.
The register list to poll is maintained based on HTTP requests from remote clients.

The single register list is maintained across all clients and culled if no repeat requests received in a time limit,
Should scale with minimal impact to the Inverter, until the Arduino runs out of HTTP bandwidth.


## Webserver interface:

  http://solis.local

**/dashboard**                      
**/** 
Real time gauge dashboard of Inverter information


**/H** 
Turn the onboard LED on

**/L** 
Turn the onboard LED off

**/R?refresh=<seconds>&address=<value>,<value>...**   
Return the register addresse values as JSON JS array
<value>.1 or <value>.2 for single/double address registers
If the optional Refresh setting is passed the page will auto refresh
  
e.g.
  Request
  
  http://solis.local/R/?addr=33057.2,33070

  Response 
  JSON parsable JS object with address/data pairs, Null value if no available data from the server.
  
	{ “33057": 0, "33070": null }


**/S?address=<value>**
  
**/S?all** 
  
Stop returning a register address, removes it from the internal cache list

**/C** 

Output the register address cache

