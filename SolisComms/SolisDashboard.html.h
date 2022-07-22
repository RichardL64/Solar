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
  <meta name="referrer" content="no-referrer" />
  <meta name="author" content="Richard Lincoln July 2022" />
  <meta name="description" content="https://github.com/RichardL64/Solar" />
  <title>Solar Dashboard</title>

<!--

  Proof of concept dashboard to present Arduino gathered live SOLIS inverter data

  V2.1
  Circle based gauge construction vs. previous arcs
  Simplification & removal of external depdencies

  Data download from the server driven asynchronously
  Screen updates driven by browser animation API


  Parameters
    ?server=solis.local -or- ?server=192.168.1.143
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

  .gTitle {
    font-size: 50%;
    font-weight: normal;
    fill: black;
    dominant-baseline: hanging;
  }

  .kW {
    font-size: 100%;
    font-weight: normal;
    fill: darkgrey;
    alignment-baseline: middle;
    dominant-baseline: middle;
    text-anchor: middle;
  }

  .underkW {
    font-size: 40%;
    font-weight: normal;
    fill: darkgrey;
    alignment-baseline: middle;
    dominant-baseline: middle;
    text-anchor: middle;
  }

  #gauges {
    width: 90%;
    margin: auto;
    display: grid;
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
        x,y           50, 50              pixels

        Radius                        40  pixels
        Circumference 2 *Pi *40     = 251 pixels

        Dial circ.                    210 degrees
                      251 /360 *210 = 146 pixels

          1% of dial  147 /100      = 1.46

          mid point   147 /2        = 73  pixels
          rotation    270 -210 /2   = 165 degrees

    Dial control
        move the stroke       stroke-dasharray  0-146
        move the tick to top  stroke-dashoffset -73
        mirror left/right     transform translate & scale

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
            stroke-dasharray="146, 251">
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

      <circle id="20percent-tick" class="tick"
          cx="50" cy="50" r="40"
          transform="rotate(165, 50, 50)"
          fill="transparent"
          stroke="lightgreen"
          stroke-width="8"
          stroke-dashoffset="-29"
          stroke-dasharray=".5, 251">
      </circle>
    </defs>
  </svg>


  <!--

    Guage presentation

  -->
  <div id="gauges">

      <div>
        <svg viewbox="0 0 100 65">
          <use id="solarG" href="#gauge"
            stroke="gold"
            stroke-dasharray="0, 251">
          </use>
          <text class="gTitle" y="2">Solar</text>
          <text id="solarGT" class="kW" x="50" y="50">...</text>
          <text id="solarGT2" class="underkW" x="50" y="60">...</text>
        </svg>
      </div>

      <div>
        <svg viewbox="0 0 100 65">
          <use id="batteryG" href="#gauge"
            stroke="cornflowerblue"
            stroke-dashoffset="-73"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text class="gTitle" y="2">Battery</text>
          <text id="batteryGT" class="kW" x="50" y="50">...</text>
          <text id="batteryGT2" class="underkW" x="50" y="60">...</text>
        </svg>
      </div>

      <div>
        <svg viewbox="0 0 100 65">
          <use id="gridG" href="#gauge"
            stroke="cornflowerblue"
            stroke-dashoffset="-73"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text class="gTitle" y="2">Grid</text>
          <text id="gridGT" class="kW" x="50" y="50">...</text>
          <text id="gridGT2" class="underkW" x="50" y="60">...</text>
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
          <text id="batterySOCGT" class="kW" x="50" y="50">...</text>
          <use href="#20percent-tick"
          </use>
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

    //  Inverter register addresses
    //
    const SOLAR_R           = 33057;
    const BATTCD_R          = 33135;
    const BATTERY_R         = 33149;
    const GRID_R            = 33130;
    const BATTSOC_R         = 33139;
    const TEMP_R            = 33093;

    const GEN_TODAY_R       = 33035;
    const GEN_YEST_R        = 33036;

    const CHG_TODAY_R       = 33163;          // 0.1 kWh
    const CHG_YEST_R        = 33164;

    const DIS_TODAY_R       = 33167;          // 0.1 kWh
    const DIS_YEST_R        = 33168;

    const GRID_IMP_TODAY_R  = 33171;
    const GRID_IMP_YEST_R   = 33172;
    const GRID_EXP_TODAY_R  = 33175;
    const GRID_EXP_YEST_R   = 33176;

    //  Timing constants
    //
    const FETCH_TIMEOUT       = 5000;
    const SERVER_INTERVAL     = 2000;
    const FRAME_CHECK         = 1000;
    const TIMESTAMP_INTERVAL  = 1000;
    const FRAME_OLD           = 2000;
    const DATA_OLD            = 5000;
    const KW_DECIMALS         = 2;
    const KWH_DECIMALS        = 1;

    //  Globals
    //
    let newJsonTime = new Date()                            // time data returned by callServer
    let newJson = {};                                       // latest data returned by callServer
    let dispJson = {};                                      // latest data displayed by updateDashboard

    let lastFrameTime = new Date();                         // time updateDashboard last called


    //  Startup
    //
    callServer(SERVER_INTERVAL, serverURL());               // continuous periodic data retrieval
    setInterval(updateTime, TIMESTAMP_INTERVAL);            // regular timestamp updates


    //  Parse the server URL parameter for callbacks
    //  When served from the arduino the live ip address replaces the hostname
    //
    //  Arduino searches for <script> then ** etc then "..."
    //  Whatever is between the quotes is replaced with the actual IP address
    //
    function serverURL() {
      const queryString = window.location.search;
      const urlParams = new URLSearchParams(queryString);
      let url = urlParams.get('server')

      //  *** LIVE IP ADDRESS ***
      if(url == null) url = "solis.local";

      console.log(url);
      return url;
    }

    //  Call the server for a data update & calls itself once the query completes
    //  When I get new data kick off the dashboard, assuming its stopped animating the previous update
    //  Variable length async response depending how long the fetch takes
    //
    function callServer(freq, url) {

      //  If the last dashboard frame was a while ago - its probably throttled by the browser
      //  We're probably in the background
      //  Stop doing the server round trip until the browser will show frames again
      //
      if(new Date - lastFrameTime > FRAME_OLD) {
        console.log("no frames");
        requestAnimationFrame(updateDashboard);
        setTimeout(callServer, FRAME_CHECK, freq, url);     // keep checking more frequently
        return;
      }

      //  Callback to the server for new data - asynchronous so finishes later
      //
      const controller = new AbortController()                                            // watchdog for max fetch time
      const fetchTimeout = setTimeout(() => controller.abort(), FETCH_TIMEOUT)

      fetch('http://'
          + url
          + '/R?address=33057.2,33135,33149.2,33139,33130.2,33093,33035,33171,33163,33167',
            { signal: controller.signal })

        .then(response => response.json())                  // header

        .then(json => {                                     // body
          if(json[BATTCD_R] == 1) json[BATTERY_R] *= -1     // Make battery signed - 0=chg, 1=dischg
          delete json[BATTCD_R]                             // do this here to avoid issues with animation frames on the 0/1 value

          newJson = json;                                   // newJson is the new target for display
          newJsonTime = Date.now();
          console.log(newJson);

          requestAnimationFrame(updateDashboard);           // Update the dashboard with new values
          })

        .catch(err => console.error(err))

        .finally(() => {
            clearTimeout(fetchTimeout);                     // clear the watchdog
            setTimeout(callServer, freq, freq, url);        // call myself later
          });
    }

    //  Update the dashboard
    //  Once no change is detected, waits on the server response before starting again
    //  Called by the browser animation API so no fixed frequency
    //
    function updateDashboard() {
      console.log("frames");

      lastFrameTime = new Date;                             // detect browser slowing screen updates

      //  Move dispJson towards newJson incrementally
      //  Keep a record of anything changing
      //
      let changes = false                                   // posit nothing will change
      for(const reg in newJson) {
        if(dispJson[reg] === undefined) {                   // first time through - copy the latest data
          dispJson[reg] = newJson[reg];
          changes = true;
          continue;
        }

        let delta = newJson[reg] - dispJson[reg];
        if(Math.abs(delta) <= 1 ) {                         // delta is close enough - copy the latest data
          dispJson[reg] = newJson[reg];
          continue;
        }

        let change = Math.max(Math.abs(delta)/60, 1)
                      * Math.sign(delta);                   // proportional & minimum move per frame, +ve or -ve
        dispJson[reg] += change;
        changes = true;                                     // at least one value moved!
      }

      //  Update the gauges from dispJson
      //
      setValue("solarG",      0,      4000, dispJson[SOLAR_R],    (dispJson[SOLAR_R]  /1000).toFixed(KW_DECIMALS) + " kW");
      setValue("batteryG",    -3600,  3600, dispJson[BATTERY_R],  (dispJson[BATTERY_R]/1000).toFixed(KW_DECIMALS) + " kW");
      setValue("gridG",       -10000, 4000, dispJson[GRID_R],     (dispJson[GRID_R]   /1000).toFixed(KW_DECIMALS) + " kW");
      setValue("batterySOCG", 0,      100,  dispJson[BATTSOC_R],  (dispJson[BATTSOC_R]*1).toFixed(0) + "%");

      if(!isNaN(dispJson[GEN_TODAY_R]))
        document.getElementById("solarGT2").innerHTML = (dispJson[GEN_TODAY_R]/10).toFixed(KWH_DECIMALS) + " kWh";

      if(!isNaN(dispJson[CHG_TODAY_R])
      && !isNaN(dispJson[DIS_TODAY_R])) {                 // battery net position today
        let net = (dispJson[CHG_TODAY_R] - dispJson[DIS_TODAY_R])/10;
        document.getElementById("batteryGT2").innerHTML = net.toFixed(KWH_DECIMALS) + " kWh";
      }

      if(!isNaN(dispJson[GRID_IMP_TODAY_R])) {            // grid import. Export irrelevant (to me)
        let net = (dispJson[GRID_IMP_TODAY_R])/-10;
        document.getElementById("gridGT2").innerHTML = net.toFixed(KWH_DECIMALS) + " kWh";
      }
/*
      if(!isNaN(dispJson[GRID_IMP_TODAY_R]) && !isNaN(dispJson[GRID_EXP_TODAY_R])) {
        let net = (dispJson[GRID_EXP_TODAY_R] - dispJson[GRID_IMP_TODAY_R])/10;
        document.getElementById("gridGT2").innerHTML = net.toFixed(KWH_DECIMALS) + " kWh";
      }
*/
      if(!isNaN(dispJson[TEMP_R]))
        document.getElementById("temp").innerHTML = (dispJson[TEMP_R]/10).toFixed(1);

      //  If there are changes - request more animation frames = ~60fps or more
      //
      if(changes)
        requestAnimationFrame(updateDashboard);
    }

    //  Update the time every second
    //  Warn if the data is getting old
    //
    function updateTime() {
      let now = new Date();

      if(now - newJsonTime < DATA_OLD) {
        document.getElementById("time").innerHTML = "Updated at " + now.toLocaleTimeString();
        return;
      }

      document.getElementById("time").innerHTML = "Waiting for data";
    }

    //  Set the value and value text for the passed gauge ID
    //  Updates id elements moving the dial stroke proportionately
    //
    //  Where min is -ve the zero point is always at 12 o'clock
    //  Scales left/right of the central zero point can be different
    //
    function setValue(id, min, max, val, valDisp) {

      if(isNaN(val)) return;                      // no value - ignore

      //  Value text
      //
      document.getElementById(id + "T").textContent = valDisp;

      //  Min is +ve, draw from the left end
      //
      if(min >= 0) {                              // From left hand end
        let pn = toPercent(min, max, val);        // %
        let v = 1.46 * pn;                        // * 1% of dial
        document.getElementById(id).setAttribute("stroke-dasharray", v + ", 251");
        return;
      }

      //  Min is -ve, value is +ve draw from the middle right
      //
      if(val >= 0) {
        let pn = toPercent(0, max, val);          // % 0...max
        let v = 1.46 * pn /2;                     // * 1% of half the dial
        document.getElementById(id).setAttribute("stroke-dasharray", v + ", 251");
        document.getElementById(id).setAttribute("transform", "");
        return;
      }

      // Min is -ve, value is -ve, draw from the middle left and mirror the dial
      //
      let pn = toPercent(0, min,  val);           // % min...0
      let v = 1.46 * pn /2;                       // * 1% of half the dial
      document.getElementById(id).setAttribute("stroke-dasharray", v + ", 251");
      document.getElementById(id).setAttribute("transform", "translate(100,0) scale(-1,1)");

    }

    //  Return the value in the min-max range as percent 0-100
    //
    function toPercent(min, max, val) {
      let p = val *100 / (max-min);
      let pn = Math.min(Math.max(p, 0), 100);     // normalise 0-100
      return pn;
    }

  </script>
</body>
</html>
)====";
