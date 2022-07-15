/*

    Dashboard.html.h
    https://github.com/RichardL64

    Simple real time Solis Inverter dashboard

    Call back URL is modified to the Arduino actual IP address when is this content is served
    Call back on mDNS domain name alone was unstable.

    To allow storage in progmeme, cut/paste the HTML between:
      const char *dashboardHtml = R"====(
                                  )====";

    Inline with the Arduino code space
    
    R.A.Lincoln       July 2022

*/
const char *dashboardHtml = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="author" content="Richard Lincoln July 2022">
  <meta name="description" content="https://github.com/RichardL64/Solar">
  <title>Solis Dashboard</title>

<!--

  Proof of concept dashboard to present Arduino gathered live SOLIS inverter data
  
  V2.1
  Circle based gauge construction vs. previous arcs
  Simplification & removal of external depdencies
  
  Data download from the server driven asynchronously
  Screen updates driven by browser animation API
  
  
  Parameters
    ?inverter=solis.local -or- ?inverter=192.168.1.143
    Automatically replaced with the live IP addresss when served from the Arduino


  Uses and develops gauge drawing ideas from (Thank you):
    https://www.fullstacklabs.co/blog/creating-an-svg-gauge-component-from-scratch    
    https://github.com/naikus/svg-gauge


  R.Lincoln   July 2022

-->


  <!--
  
    CSS
    
  -->
  <style>

  body {
    font-family: sans-serif;
  } 

  .border {
    border:1px solid black;
  }

  .bottom-left {
    display: flex;
    flex-direction: column;
    align-self: flex-end;
  }

  .bottom-right {
    display: flex;
    flex-direction: row-reverse;  
    align-self: flex-end;
  }

  #gauges {
    width:90%;
    margin:auto; 
    display:grid;
    grid-template-columns: 1fr 1fr 1fr;
    padding: 10px;  
  }


  </style>

</head>



<!--

  Body
  
-->
<body>

  <!--

    SVG shape definitions

    Dial geometry:
        x,y       50, 50          pixels
        
        Radius              = 40  pixels
        Circumference 2 *Pi *40     = 251   pixels
        
        Dial circ.              210   degrees
                251 /360 *210 = 146   pixels
                
          1% of dial  147 / 100   = 1.46
          
          mid point 147 /2      = 73  pixels          
          rotation  270 -210 /2   = 165 degrees
          
    Control
        stroke-dasharray  - 0-146 move the stroke
        stroke-dashoffset   - -73 move the tick to 12 o'clock
        transform     -     mirror left/right

  -->
  <svg width="0" height="0">
    <defs>
      <g id="gauge"
        stroke-width="8"
        fill="transparent"
        transform="rotate(165, 50, 50)">
        <circle class="track"
            cx="50" cy="50" r="40"
            stroke="whitesmoke"
            stroke-dashoffset="0"
            stroke-dasharray="146,251">
        </circle>
        <circle class="stroke"
            cx="50" cy="50" r="40">
        </circle>
        <circle class="tick"
            cx="50" cy="50" r="40"
            stroke="black"
            stroke-width="10"
            stroke-dasharray=".5, 251">
        </circle>
      </g>  
    </defs>
  </svg>


  <!--
  
    Guage presentation
    
  -->
  <div id="gauges">

      <div>
        Solar
        <svg viewbox="0 0 100 65">
          <use id="solarG" href="#gauge" 
            stroke="gold"
            stroke-dasharray="0, 251">
          </use>
          <text id="solarGT"
            x="50" y="50" 
            font-size="100%"
            font-family="sans-serif"
            font-weight="normal"
            fill="darkgrey"
            text-anchor="middle" 
            alignment-baseline="middle"
            dominant-baseline="middle">
          </text> 
        </svg>
      </div>

      <div>
        Battery
        <svg viewbox="0 0 100 65">
          <use id="batteryG" href="#gauge" 
            stroke="cornflowerblue"
            stroke-dashoffset="-73"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text id="batteryGT"
            x="50" y="50" 
            font-size="100%"
            font-family="sans-serif"
            font-weight="normal"
            fill="darkgrey"
            text-anchor="middle" 
            alignment-baseline="middle"
            dominant-baseline="middle">
          </text> 
        </svg>
      </div>
      
      <div>
        Grid
        <svg viewbox="0 0 100 65">
          <use id="gridG" href="#gauge"  
            stroke="cornflowerblue"
            stroke-dashoffset="-73"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text id="gridGT"
            x="50" y="50" 
            font-size="100%"
            font-family="sans-serif"
            font-weight="normal"
            fill="darkgrey"
            text-anchor="middle" 
            alignment-baseline="middle"
            dominant-baseline="middle">
          </text> 
        </svg>
      </div>

      <div class="bottom-left">
        <span>Inverter temp. <label id="temp"></label>&#8451;</span>
      </div>
      
      <div>
        <svg viewbox="0 0 100 65">
          <use id="batterySOCG" href="#gauge"  
            stroke="darkseagreen"
            stroke-dasharray="0, 251">
          </use>
          <text id="batterySOCGT" 
            x="50" y="50" 
            font-size="100%"
            font-family="sans-serif"
            font-weight="normal"
            fill="darkgrey"
            text-anchor="middle" 
            alignment-baseline="middle"
            dominant-baseline="middle">
          </text> 
        </svg>
      </div>

      <div class="bottom-right">
        <span><label id="time"></label></span>
      </div>

  </div>


  <!--
  
    Script
    
  -->
  <script>

    //  Parse the inverter URL parameter for callbacks
    //  When served from the arduino the live ip address replaces the hostname
    //
    //  Arduino searches for <script> then ** etc then "..."
    //  Whatever is between the quotes is replaced with the actual IP address
    //
    function inverterURL() {
      const queryString = window.location.search;
      const urlParams = new URLSearchParams(queryString);
      let inverterURL = urlParams.get('inverter')
    
      //  *** LIVE IP ADDRESS ***
      if(inverterURL == null) inverterURL = "solis.local";
    
      console.log(inverterURL);
      return inverterURL;
    }
  
    //  Call the server for a data update
    //  Calls itself at the passed millis for updates
    //
    let newTime =0;
    let newJson = {};
    function callServer(freq) {
  
      fetch('http://' + URL + '/R?address=33057.2,33135,33149.2,33139,33130.2,33093')
      .then(res => res.json())
      .then(json => {
      
        newJson = json;
        if(json[33135] == 1) json[33149] *= -1      // Make battery signed - 0=chg, 1=dischg
        newTime = Date.now();
        console.log(newJson);

        })
      
      .catch(err => console.error(err))
      .finally(() => {
        setTimeout(callServer, freq, freq);       // Auto refresh
        });
    }


    //  Update the dashboard
    //  Called by the browser animation API so no fixed frequency
    //
    //  If the server update is old - display
    //  Else update from the most recent server response
    //  
    //  ?? consider large gaps between callbacks...??
    //  Gauges seem to keep moving towards infinity?
    //
    let lastCall = Date.now();
    let dispJson = {};
    function updateDashboard() {
      let thisCall = Date.now();            // work out our calling frequency
      let freq = thisCall-lastCall || 16;
      lastCall = thisCall;
                
      if(Date.now() - newTime > 5000) {       // Old data? - warn and exit
        document.getElementById("time").innerHTML = "(Waiting for data)";
        setTimeout(updateDashboard, 2000);      // Wait a couple of seconds
        return;
      }

      //  Move dispJson towards newJson
      //  Naturally slows/inverse square as it gets close
      //
      for(const reg in newJson) {
        if(dispJson[reg] === undefined) dispJson[reg] = 0;
        let delta = (newJson[reg] - dispJson[reg]) || 0;
        let change = delta *freq /2000        // get there in n millis
        dispJson[reg] += change;
      }
      
      //  Update the gauges
      //
      let d = 2;
      setValue("solarG", 0, 4000, dispJson[33057], (dispJson[33057]/1000).toFixed(d) +" kW");     
      setValue("batteryG", -3500, 3500, dispJson[33149], (dispJson[33149]/1000).toFixed(d) +" kW");
      setValue("batterySOCG", 0, 100, dispJson[33139], (dispJson[33139]*1).toFixed(0) + "%");
      setValue("gridG", -10000, 4000, dispJson[33130], (dispJson[33130]/1000).toFixed(d) +" kW");
      
      document.getElementById("temp").innerHTML = (newJson[33093]/10).toFixed(1);

      let now = new Date();
      document.getElementById("time").innerHTML = "Updated at " + now.toLocaleTimeString();

      //  Callback via browser animation API
      //
      requestAnimationFrame(updateDashboard)
    }

    
    //  Startup
    //
    const URL = inverterURL()         // URL from ?inverter=
    callServer(2000);             // Data updates
    updateDashboard();              // Screen updates driven by browser animation
      


    //  Set the value and value text for the passed gauge ID
    //  Updates id and id+"T" elements
    //
    //  Where min is -ve the zero point is always at 12 o'clock
    //  Scales left/right of the central zero point can be different
    //
    function setValue(id, min, max, val, valDisp) {

      //  Value text
      document.getElementById(id + "T").textContent = valDisp;

      //  Min is +ve, draw from the left end
      if(min >= 0) {                    // From left hand end
        let pn = toPercent(min, max, val);        // %
        let v = 1.46 * pn;                // * 1% of dial
        document.getElementById(id).setAttribute("stroke-dasharray", v + ", 251");
        return; 
      }

      //  Min is -ve, value is +ve draw from the middle right
      if(val >= 0) {
        let pn = toPercent(0, max, val);        // % 0...max
        let v = 1.46 * pn /2;             // * 1% of half the dial
        document.getElementById(id).setAttribute("stroke-dasharray", v + ", 251");
        document.getElementById(id).setAttribute("transform", "");
        return;
      }

      // Min is -ve, value is -ve, use -ve side scale and mirror the dial
      let pn = toPercent(0, min,  val);         // % min...0
      let v = 1.46 * pn /2;               // * 1% of half the dial
      document.getElementById(id).setAttribute("stroke-dasharray", v + ", 251");
      document.getElementById(id).setAttribute("transform", "translate(100,0) scale(-1,1)");

    }
    
    //  Give in value in the min-max range convert to 0-100
    //
    function toPercent(min, max, val) {
      let p = val *100 / (max-min);
      let pn = Math.min(Math.max(p, 0), 100);       // normalise 0-100
      return pn;
    }
    
  </script>

</body>

</html>
)====";
