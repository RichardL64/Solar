#Dasboard

The standa alone dashboard HTML/JS
Uses a modified SVG Gauges library.

This gets embedded inthe SolisComms Arduino app - simple cut/paste between the delimiters in SolisDashboard.html.h & Gauge.js.h

On loading the Arduino SolisComms webserver seeks out <Script> then <solis.local> and changes the latter to the actual arduino IP address so the client side requests data on IP rather than mDNS domain name. Going raw IP seemed more stable.

