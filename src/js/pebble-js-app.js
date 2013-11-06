
function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}

function fetchWeather() {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.1/find/city?lat=37.830310&lon=-122.270831&cnt=1', true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
  
        var temperature, icon;
        if (response && response.list && response.list.length > 0) {
          var weatherResult = response.list[0];
          temperature = Math.round(weatherResult.main.temp - 273.15);
          icon = iconFromWeatherId(weatherResult.weather[0].id);
          console.log(temperature);
          console.log(icon);
          Pebble.sendAppMessage({ "icon":icon, "temperature":temperature + "\u00B0C"});
        }

      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

PebbleEventListener.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          fetchWeather();
                          console.log(e.type);
                        });

PebbleEventListener.addEventListener("appmessage",
                        function(e) {
                          console.log(e.type);
                          console.log(e.payload.temperature);
                          console.log("message!");
                          fetchWeather();
                        });

PebbleEventListener.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });

//Pebble.openURL("http://www.google.com");

