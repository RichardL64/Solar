# Solar - Solis Inverter real time Dashboard
Solis inverter / Pylontech battery arduino integration for remote panel etc without a dependency on third party cloud services.
The inverter and batteries expose RS485 integration potential.

Arduino seems lighter/simpler for, effectively an HTML data and Webserver front end on the inverter.

## Current state:

 - Shelly control integration to enhance SOLIC functionality - turn on when batteries are full and solar in production. + POC off the shelf device control accessing the JSON/HTTP arduino interface.
 
 - Self contained/zero install dashboard served from the Arduino at path solis.local
 
 - Arduino web server & internal cross client cache
 - Inverter integration working with Modbus/RS485 connection.
 - Basic HTML/JS Dashboard consumer showing real time/~2 second updates.
 - Todays total kWh added.
 - mDNS hostname to IP lookup
 - Basic wifi reconnection on loss logic
 - Integration with browser animation callbacks to reduce load for background windows

## Known issues:

 - mDNS seems to lockup sometimes esp. moving between mesh wifi transmitters.

## Intention:

- Local physical, moving coil, meter readouts for inverter real time data display.

- Potential to locally consuming any inverter parameter for logical control of other devices.



## Current dashboard:
![alt text](https://github.com/RichardL64/Solar/blob/main/Solis%20Dashboard.jpg)

## Current breadboard hardware:
 - Arduino Nano 33 IOT
 - RS485 board
 - Solis Inverter RS485 port plug

![alt text](https://github.com/RichardL64/Solar/blob/main/Solis%20comms%20hardware%20test.jpeg)
