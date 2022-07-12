/*

    Dashboard.html.h
    https://github.com/RichardL64

    Simple real time Solis Inverter dashboard
    Periodic retreival of live data and display on guages.

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
<html>
<head>
  <meta charset="utf-8" />
  <meta http-equiv="Content-Style-Type" content="text/css">
  <title></title>
</head>

<!--

  Proof concept dashboard to present Arduino gathered live SOLIS inverter data
  
  Parameters
    ?inverter=solis.local -or- ?inverter=192.168.1.143
    Automatically replaced with the live IP addresss when served from the Arduino

  Uses
    SVC Gauges https://github.com/naikus/svg-gauge
    Modified for zero center stroke gauges with +- limits
    

  R.Lincoln   July 2022
  
-->

<script src="gauge.js"></script>

<style>

body {
  font-family: sans-serif;
}

.gauge-container > .gauge .dial {
  stroke-width: 6;
}
.gauge-container > .gauge .value {
  stroke:cornflowerblue;
    stroke-width: 6;
}

#gauges {
  width:90%;
  margin:auto; 
  display:grid;
  grid-template-columns: 1fr 1fr 1fr;
  padding: 10px;
  
/*  border:1px solid black;   */
}

#battSOCG > .gauge .value {
  stroke:darkseagreen;
}

#time {
  float:right;
} 

</style>

<body>

<div id="gauges">
    <div id="solarG" class="gauge-container">
    Solar
    </div>
    <div id="batteryG" class="gauge-container">
    Battery
    </div>
    <div id="gridG" class="gauge-container">
    Grid
    </div>
    <div></div>
    <div id="battSOCG" class="gauge-container">
    </div>
    <div>
    </div>
    <div>
      Inverter temp. <label id="temp"></label>&#176C
    </div>
    <div>
    </div>
    <div>
      <label id="time"></label>
    </div>
</div>

<script>
  //  Parse the inverter URL parameter
  //
  function inverterURL() {
    const queryString = window.location.search;
    console.log(queryString);
    
    const urlParams = new URLSearchParams(queryString);
    var inverterURL = urlParams.get('inverter')
    console.log(inverterURL);
    
    //  Default value if none passed
    //  Replaced with the live IP address when served from the Arduino
    //
    if(inverterURL == null) inverterURL = "solis.local";
    
    return inverterURL;
  }

  //  Setup Gauges
  //
  var solarG = Gauge(document.getElementById("solarG"),
      {
        max: 4000,
          dialStartAngle: 170,
          dialEndAngle: 10,
          viewBox: "0 0 100 60",
          label: function(value) {
            return (value/1000).toFixed(2) + "kW";
              }
    });
    
  var batteryG = Gauge(document.getElementById("batteryG"),
      {
          min: -3500,
          max: 3500,
          dialStartAngle: 170,
          dialEndAngle: 10,
          viewBox: "0 0 100 60",
          label: function(value) {
            return (value/1000).toFixed(2) + "kW";
            }
    });
    
    var battSOCG = Gauge(document.getElementById("battSOCG"),
    {
          min: 0,
          max: 100,
          dialStartAngle: 170,
          dialEndAngle: 10,
          viewBox: "0 0 100 60",
          label: function(value) {
            return value.toFixed(0) + "%";
            }
      });
      
  var gridG = Gauge(document.getElementById("gridG"),
    {
          min: -10000,
          max: 3600,
          dialStartAngle: 170,
          dialEndAngle: 10,
          viewBox: "0 0 100 60",
          label: function(value) {
            return (value/1000).toFixed(2) + "kW";
            }
      });
  
    
  // Update gauge values from the server
  //
  var lastUpdate =0;
  function update() {
  
/*    Offline testing with random numbers
    fetch('http://solis.local/R?address=101')
        .then(res => res.json())
          console.log(json);
      solarG.setValueAnimated(json["101"], 2);
*/

    fetch('http://' + URL + '/R?address=33057.2,33135,33149.2,33139,33130.2,33093')
        .then(res => res.json())
        .then(json => {
          console.log(json);
 
          solarG.setValueAnimated(json[33057], 2);    // Solar W

          let battW = json[33149];            // Battery W
          if(json[33135] == 1) battW = -battW       // 0=chg, 1=dischg
      batteryG.setValueAnimated(battW, 2);

      battSOCG.setValueAnimated(json[33139], 2);    // Battery SOC%
          gridG.setValueAnimated(json[33130], 2);     // Grid
 
      document.getElementById("temp").innerHTML = (json[33093]/10).toFixed(1);  // Temp
      
      //  Watchdog - record the last time we got data
      lastUpdate = Date.now();
          })
          
        .catch(err => console.error(err))
    .finally(() => {
      setTimeout(update,2000);            // Auto refresh
      });
      
  }

  //  Provided the update loop ran in the last few seconds, update the time
  //  
  function time() {
    if(Date.now() - lastUpdate > 5000) {
          document.getElementById("time").innerHTML = "(Waiting for data)";
      } else {      
      var now = new Date();
          document.getElementById("time").innerHTML = "Updated at " + now.toLocaleTimeString();
      }
      setTimeout(time,1000);
  }
  
  //  Startup
  //
  const URL = inverterURL()               // URL from ?inverter=
  time();
  update();                       // Start updates!
  
</script>

</body>

</html>
)====";
