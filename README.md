# Solar
Solis inverter / Pylontech battery arduino integration for remote panel etc without a dependency on third party cloud services.
The inverter and batteries expose RS485 integration potential.

Arduino seems lighter/simpler for, effectively an HTML data and Webserver front end on the inverter.

Current state:

 - Self contained dashboard available from <ip>/dashboard
 
 - Arduino web server & internal cross client cache
 - Inverter integration working with Modbus/RS485 connection.
 - Basic HTML/JS Dashboard consumer showing real time/~2 second updates.

Known issues:

mDNS seems to timeout/lockup periodically requiring a ping to the servers IP address to wake up
Using IP address directly to workaround

Intention:

Local physical, moving coil, meter readouts for inverter real time data display.

Potential to locally consuming any inverter parameter for logical control of other devices.

