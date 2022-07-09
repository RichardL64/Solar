# Solar
Solis inverter / Pylontech battery arduino integration for remote panel etc outside the cloud.
The inverter and batteries expose RS485 integration potential.

Avoiding Raspberry PI solutions due to current unavailability and trying to minimise solution complexity

Current state:

 - Arduino web server & internal cross client cache answering to solis.local
 - Inverter integration working with Modbus/RS485 connection.
 - Basic HTML/JS Dashboard consumer showing real time/~2 second updates.

Known issues:

mDNS seems to timeout/lockup periodically requiring a ping to the servers IP address to wake up


Intention:

Local physical, moving coil, meter readouts for inverter real time data display.
Simple as possible - avoiding cloud integration.

