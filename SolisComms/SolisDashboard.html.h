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

  V3.0
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

  .gTitle {
    font-size: 50%;
    font-weight: normal;
    fill: black;
    dominant-baseline: hanging;
    text-anchor: middle;
  }

  .kW {
    font-size: 90%;
    font-weight: normal;
    fill: black;
    alignment-baseline: middle;
    dominant-baseline: middle;
    text-anchor: middle;
  }

  .kWh {
    font-size: 40%;
    font-weight: normal;
    fill: black;
    alignment-baseline: middle;
    dominant-baseline: middle;
    text-anchor: middle;
  }

  .right {
    text-anchor: end;
  }

  #gauges {
    width: 100%;
    margin: auto;
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
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
                      2 *Pi *31     = 195

        Dial circ.                    210 degrees
                      251 /360 *210 = 146 pixels
                      195 /360 *210 = 113

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

      <circle id="1020-tick" class="tick"
          cx="50" cy="50" r="40"
          transform="rotate(165, 50, 50)"
          fill="transparent"
          stroke="darkgrey"
          stroke-width="8"
          stroke-dashoffset="-14.6"
          stroke-dasharray=".5, 14, .5, 251">
      </circle>
    </defs>
  </svg>


  <!--

    Guage presentation
    &#8451;   Degrees C

  -->
  <div id="gauges">

      <div>
        <svg viewbox="0 0 100 65">
          <use id="solarG" href="#gauge"
            stroke="gold"
            stroke-dasharray="0, 251">
          </use>
          <text class="gTitle" x="50" y="26">Solar</text>
          <text id="solarGT" class="kW" x="50" y="46">...</text>
          <text id="solarGT2" class="kWh" x="50" y="59">...</text>
        </svg>
      </div>

      <div>
        <svg viewbox="0 0 100 65">
          <use id="batteryG" href="#gauge"
            stroke="darkseagreen"
            stroke-dashoffset="-73"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text class="gTitle" x="50" y="26">Battery</text>
          <text id="batteryGT" class="kW" x="50" y="46">...</text>
          <text id="batteryGTIn" class="kWh right" x="63" y="56"></text>
          <text id="batteryGTOut" class="kWh right" x="63" y="63"></text>
        </svg>
      </div>

      <div>
        <svg viewbox="0 0 100 65">
          <use id="gridG" href="#gauge"
            stroke="orangered"
            stroke-dashoffset="-73"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text class="gTitle" x="50" y="26">Grid</text>
          <text id="gridGT" class="kW" x="50" y="46">...</text>
          <text id="gridGTIn" class="kWh right" x="63" y="56"></text>
          <text id="gridGTOut" class="kWh right" x="63" y="63"></text>
        </svg>
      </div>

      <div>
        <svg viewbox="0 0 100 65">
          <use id="houseG" href="#gauge"
            stroke="cornflowerblue"
            stroke-dasharray="0, 251"
            transform="">
          </use>
          <text class="gTitle" x="50" y="26">Demand</text>
          <text id="houseGT" class="kW" x="50" y="46">...</text>
          <text id="houseGT2" class="kWh" x="50" y="59"></text>
        </svg>
      </div>

      <div>
        <svg viewbox="0 0 100 66">
          <use id="batterySOCG" href="#gauge"
            stroke="darkseagreen"
            stroke-dasharray="0, 251">
          </use>
          <text class="gTitle" x="50" y="28">Charge level</text>
          <text id="batterySOCGT" class="kW" x="50" y="46">...</text>
          <text id="batterySOCGT2" class="kWh" x="50" y="56"></text>
          <text id="batterySOCGTH" class="kWh" x="50" y="63"></text>
          <use href="#1020-tick"</use>
        </svg>
      </div>


      <div>
        <svg viewbox="0 0 100 65">
          <text id="temp" class="kWh right" x="92" y="54"></text>
          <text id="time" class="kWh right" x="92" y="62"></text>
        </svg>
      </div>

  </div>


  <!--

    Script

  -->
  <script>

    //  Inverter register addresses
    //
    const SOLAR_R           = 33057;
    const BATTCD_R          = 33135;          // 0 charge, 1 discharge
    const BATTERY_R         = 33149;
    const GRID_R            = 33130;          // +ve export, -ve import
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

    const HOUSE_R           = 33147;
    const HOUSE_TODAY_R     = 33179;

    //  Calculated fields
    //
    const BATTERY_KWH_R     = 33303;
    const BATTERY_HOURS_R   = 33304;

    //  various constants
    //
    const FETCH_TIMEOUT       = 5000;
    const SERVER_INTERVAL     = 2000;
    const FRAME_CHECK         = 1000;
    const TIMESTAMP_INTERVAL  = 1000;
    const FRAME_OLD           = 2000;
    const DATA_OLD            = 5000;
    const KW_DECIMALS         = 2;
    const KWH_DECIMALS        = 1;
    const FPS                 = 30;         // 30 fps is plenty
    const BATTERY_KWH         = 3.5 *2      // battery total capacity

    //  Chars
    //
    const LEFT_ARROW          = "&#8592;";
    const RIGHT_ARROW         = "&#8594;";
    const LEFTRIGHT_ARROW     = "&#8596;";

    //  Register request constants
    //
    const ADDRESS = [SOLAR_R+.2, GEN_TODAY_R,
                    BATTERY_R+.2, BATTCD_R, BATTSOC_R, CHG_TODAY_R, DIS_TODAY_R,
                    GRID_R+.2, GRID_IMP_TODAY_R, GRID_EXP_TODAY_R,
                    HOUSE_R, HOUSE_TODAY_R,
                    TEMP_R];
    const ADDRESS_LENGTH = ADDRESS.length;
    const ADDRESS_S = "?address=" + ADDRESS.join();
    console.log(ADDRESS_S);

    //  Globals
    //
    let newJson = {};                                       // latest data returned by callServer
    let newJsonTime = new Date()                            // time data returned by callServer
    let dispJson = {};                                      // latest data displayed by updateDashboard

    let lastFrame = 0;                                      // only ever one anim frame request at a time
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

    //  Make sure there is only one animation frame request made at a time
    //  These all seem to get honoured eventually and backup if called in the background
    //
    function requestAnimFrame() {
        cancelAnimationFrame(lastFrame);
        lastFrame = requestAnimationFrame(updateDashboard);
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
        requestAnimFrame();                                 // see if we can get an anim frame
        setTimeout(callServer, FRAME_CHECK, freq, url);     // keep checking more frequently
        return;
      }

      //  Callback to the server for new data - asynchronous this function could return first
      //
      const controller = new AbortController()              // watchdog for max fetch time
      const fetchTimeout = setTimeout(() => controller.abort(), FETCH_TIMEOUT)

      fetch("http://" + url + "/R" + ADDRESS_S, { signal: controller.signal })

        .then(response => response.json())                  // header

        .then(json => {                                     // body

          for(let i = 0; i < ADDRESS_LENGTH; i++) {         // convert passed value only to name/value pairs
            newJson[ADDRESS[i].toFixed(0)] = json["data"][i];
          };
          newJsonTime = Date.now();
          adjustNewJson();
          console.log(newJson);

          requestAnimFrame();                               // Update the dashboard with new values
          })

        .catch(err => console.error(err))

        .finally(() => {
            clearTimeout(fetchTimeout);                     // clear the watchdog
            setTimeout(callServer, freq, freq, url);        // call myself later
          });
    }

    //  Adjust inbound inverter data
    //  Local changes to newJson - generally +ve = generation, -ve = consumption
    //
    //  Performing this before any display means animation etc will work for calculated values
    //
    function adjustNewJson() {

          //  Same scale as other values simplifies the change detection
          //
          newJson[BATTSOC_R] = newJson[BATTSOC_R] *10;

          //  Convert battery charge/discharge to a signed value
          //  -ve = using power to charge
          //  +ve = supplying power
          //
          if(newJson[BATTCD_R] == 1) {                      // Make battery signed - 0=chg, 1=dischg
            newJson[BATTERY_R] *= -1;
          }
          delete newJson[BATTCD_R];

          //  kWh in the battery - hard coded to physical battery capacity
          //
          const capacity1 = BATTERY_KWH /100;               // 1% of battery capacity in kWh
          newJson[BATTERY_KWH_R] = newJson[BATTSOC_R] *capacity1;

          //  Hours battery remaining at the current rate of discharge
          //  ..to 10% capacity, never goes -ve
          //
          let perc80 = newJson[BATTERY_KWH_R] - (capacity1 *20);          // capacity in kWh to 20% (empty)
          newJson[BATTERY_HOURS_R] = Math.max(perc80 / -newJson[BATTERY_R] *10, 0);
          if (newJson[BATTERY_R] ==0) newJson[BATTERY_HOURS_R] = 0;
    }


    //  Update the dashboard
    //  Once no change is detected, waits on the server response before starting again
    //  Called by the browser animation API so no fixed frequency
    //
    function updateDashboard() {
      console.log("frames");
      lastFrameTime = new Date;                             // detect browser slowing screen updates

      //  Move dispJson towards newJson incrementally for each register value
      //  Keep a record of anything changing so we know when to stop asking for animation frames
      //  CSS transition is not used because of the mirrored gauges and going through 0
      //
      let changes = false                                   // posit nothing will change
      for(const reg in newJson) {
        let oldV = dispJson[reg];
        let targetV = newJson[reg];
        const alpha = .8 /FPS                             // exponential towards the new value @ ~60fps
        let newV = (alpha * targetV) + ((1- alpha) * oldV);
        if(isNaN(newV)) newV = targetV;                     // first time through - jump to the target
        dispJson[reg] = newV;

        if(Math.abs(targetV - newV) > 10) changes = true;   // admit something moved significantly
      }

      //  Update the gauges from dispJson for smooth transition
      //
      setValue("solarG",      0,     4000, dispJson[SOLAR_R],   kW(dispJson[SOLAR_R]));
      setValue("batteryG",    -3600, 3600, dispJson[BATTERY_R], lrArrow(dispJson[BATTERY_R]) + kW(Math.abs(dispJson[BATTERY_R])));
      setValue("gridG",       -10000,4000, dispJson[GRID_R],    lrArrow(dispJson[GRID_R]) + kW(Math.abs(dispJson[GRID_R])));
      setValue("houseG",      0,    10000, dispJson[HOUSE_R],   kW(dispJson[HOUSE_R]));
      setValue("batterySOCG", 0,     1000, dispJson[BATTSOC_R], (dispJson[BATTSOC_R]/10).toFixed(0) + "%");

      //  Update from newJson for direct to the target value
      //
      document.getElementById("solarGT2").innerHTML =       kWh(newJson[GEN_TODAY_R]);
      document.getElementById("batteryGTIn").innerHTML =    LEFT_ARROW + kWh(newJson[DIS_TODAY_R]);
      document.getElementById("batteryGTOut").innerHTML =   RIGHT_ARROW + kWh(newJson[CHG_TODAY_R]);
      document.getElementById("batterySOCGT2").innerHTML =  kWh(newJson[BATTERY_KWH_R]);
      document.getElementById("batterySOCGTH").innerHTML =  hours(newJson[BATTERY_HOURS_R]);
      document.getElementById("gridGTIn").innerHTML =       LEFT_ARROW + kWh(newJson[GRID_IMP_TODAY_R]);
      document.getElementById("gridGTOut").innerHTML =      RIGHT_ARROW + kWh(newJson[GRID_EXP_TODAY_R]);
      document.getElementById("houseGT2").innerHTML =       kWh(newJson[HOUSE_TODAY_R]);
      document.getElementById("temp").innerHTML =           "Inverter temp. "+ (newJson[TEMP_R]/10 || 0).toFixed(1) + "&#8451;";

      //  If there were changes - request more animation frames @ ~60fps
      //
      if(changes) setTimeout(requestAnimFrame, 1000 /FPS, updateDashboard);
    }


    //  Update the time every second
    //  Warn if the data is getting old
    //
    function updateTime() {
      let now = new Date();

      if(now - newJsonTime > DATA_OLD) {
        document.getElementById("time").textContent = "Waiting for data";
        return;
      }

      document.getElementById("time").textContent = "Updated at " + now.toLocaleTimeString();
    }

    //  Returns the label format for passed hours
    //
    function hours(val) {
      if(isNaN(val)) return "...";
      if(val == 0) return "";                     // charging
      if(val < 1) return "~ " + Math.floor(60 * val) + " mins capacity";
      return "~ " + Math.floor(val) + " hrs capacity";
    }

    //  Returns the Label format for kW passed the value in watts
    //
    function kW(val) {
      if(isNaN(val)) return "...";
      return (val/1000).toFixed(KW_DECIMALS) + "&thinsp;kW";
    }

    //  Returns the label for kWh passed the value in 10x watt hours
    //
    function kWh(val) {
      if(isNaN(val)) return "...";
      return (val/10).toFixed(KWH_DECIMALS) + "&thinsp;kWh";
    }

    //  Format -ve numbers as left arrow, +ve numbers as right arrow
    //    &#8592;   Left arrow
    //    &#8594;   Right arrow
    //    &#8596;   Left/Right arrow
    //
    function lrArrow(val) {
      if(val <= 0) return LEFT_ARROW;
      if(val >= 0)  return RIGHT_ARROW;
      return "";
    }

    //  Set the value and value text for the passed gauge ID
    //  Updates id elements moving the dial stroke proportionately
    //
    //  Where min is -ve the zero point is always at 12 o'clock
    //  Scales left/right of the central zero point can be different
    //
    function setValue(id, min, max, val, label) {

      if(isNaN(val)) return;                      // no value - ignore

      //  Value text
      //
      document.getElementById(id + "T").innerHTML = label;

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
