# Dashboard

The standa alone dashboard HTML/JS
Uses a modified SVG Gauges library based on Naikus': https://github.com/naikus/svg-gauge

## Local/testing 
Can be run locally with a ?inverter=<ip> suffix to explicitly tell it where the server is.

## Served from the Arduino
The file is embedded inthe SolisComms Arduino app - simple cut/paste between the delimiters in SolisDashboard.html.h & Gauge.js.h

On loading the Arduino SolisComms webserver seeks out <Script> then <solis.local> and changes the latter to the actual arduino IP address so the client side requests data on IP rather than mDNS domain name. Going raw IP seemed more stable.

## Structure
Some basic CSS/HTML,

An update() routine to call <server>/R?address=.... to retreive JSON formatted inverter registers
  The value is passed for display on gauges.
  Then calls itself with a simple timeout.

A time() routeine to display 'as at' information. The clock/time is local, but checks periodcially that the server retreival routine is returning data and doesn't update if things stop.
