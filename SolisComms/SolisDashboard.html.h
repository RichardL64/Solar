/*

    Dashboard.html.h
    https://github.com/RichardL64

    Simple real time Solis Inverter dashboard
    Periodic retreival of live data and display on guages.

    Call back URL is modified to the Arduino actual IP address when is this content is served
    Call back on mDNS domain name alone was unstable.

    To allow storage in progmeme, wrapped in:
      const char *dashboardHtml = R"====(
                                  )====";

    Inline with the Arduino code space
    
    R.A.Lincoln       July 2022

*/
const char *dashboardHtml = R"====(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta http-equiv="Content-Style-Type" content="text/css">
  <title></title>
</head>

<!--

	Proof concept dashboard for Arduino gathering of SOLIS inverter data
	
	Parameters
		?inverter=solis.local -or- ?inverter=192.168.1.143
		Workaround for mDNS instability - explicit server referencing

	Uses
		SVC Gauges https://github.com/naikus/svg-gauge
		Modified for zero center stroke gauges with +- limits
		

	R.Lincoln		July 2022
	
-->

<script src="gauge.js"></script>

<style>

body {
	font-family: Verdana, sans-serif;
}

.gauge-container > .gauge .dial {
	stroke-width: 5;
}
.gauge-container > .gauge .value {
	stroke:cornflowerblue;
  	stroke-width: 5;
}

#gauges {
	width:90%;
	margin:auto; 
	display:grid;
	grid-template-columns: auto auto auto;
	
/*	border:1px solid black; 	*/
}

#battSOCG > .gauge .value {
	stroke:darkseagreen;
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
		<div>
		</div>
		<div id="battSOCG" class="gauge-container">
		</div>
</div>

<script>
	//	Parse the inverter URL parameter
	//
	function inverterURL() {
		const queryString = window.location.search;
		console.log(queryString);
		
		const urlParams = new URLSearchParams(queryString);
		var inverterURL = urlParams.get('inverter')
		console.log(inverterURL);
		
		if(inverterURL == null) inverterURL = "solis.local";
		
		return inverterURL;
	}

	//	Setup Gauges
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
      		max: 10000,
      		dialStartAngle: 170,
      		dialEndAngle: 10,
      		viewBox: "0 0 100 60",
      		label: function(value) {
        		return (value/1000).toFixed(2) + "kW";
      			}
    	});
  
  
	// Update gauge values from the server
	//
	function update() {
	
/*		Offline testing with random numbers
		fetch('http://solis.local/R?address=101')
        .then(res => res.json())
        	console.log(json);
			solarG.setValueAnimated(json["101"], 2);
*/

		fetch('http://' + URL + '/R?address=33057.2,33135,33149.2,33139,33130.2,33093')
//		fetch('http://solis.local/R?address=33057.2,33135,33149.2,33139,33130.2,33093')
        .then(res => res.json())
        .then(json => {
        	console.log(json);
 
        	solarG.setValueAnimated(json[33057], 2);		// Solar W

        	let battW = json[33149];						// Battery W
        	if(json[33135] == 1) battW = -battW				// 0=chg, 1=dischg
			batteryG.setValueAnimated(battW, 2);

			battSOCG.setValueAnimated(json[33139], 2);		// Battery SOC%
        	gridG.setValueAnimated(json[33130], 2);			// Grid
        	})
        	
        .catch(err => console.error(err))
		.finally(() => {
			setTimeout(update,2000);						// Auto refresh
			});
			
	}
	
	//	Startup
	//
	const URL = inverterURL()								// URL from ?inverter=
	update();												// Start updates!
	
</script>

</body>

</html>
)====";
