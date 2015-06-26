var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getLocationFromCoords(latitude, longitude) {
  // Construct Location URL
  var locquery = encodeURI("select woeid, latitude, longitude, countrycode, statecode, city, street from geo.placefinder where text=\"" + latitude + "," + longitude +"\" and gflags=\"R\"");
  var locurl = "http://query.yahooapis.com/v1/public/yql?q=" + locquery + "&format=json"; 
  
  // Send request to Yahoo!
  xhrRequest(locurl, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      // Do this if placefinder query succeeds
      if (json) {
        // Location
        var countrycode = json.query.results.Result.countrycode;
        var statecode = json.query.results.Result.statecode;
        var city = json.query.results.Result.city;
        if (statecode !== null) {
          var region = statecode;
          region = statecode;
        }
        else {
          var region = countrycode;
          region = countrycode;
        }
        var woeid = json.query.results.Result.woeid; 
        var location = city + ", " + region;
        
        console.log("Location: " + location);
        console.log("WOEID: " + woeid);
        
        // Get the weather
        getWeatherFromWOEID(woeid, location);
      }
      else {
        // Do this if placefinder query fails
        // Assemble dictionary using our keys
        var dictionary = {
          "KEY_LOCATION": "Location Unknown",
          "KEY_CONDITIONS": "Weather Unknown"
        };

        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Weather and location info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending weather and location info to Pebble!");
          }
        );
      }
    }      
  );
}

function getWeatherFromWOEID(woeid, location) {
  var wxquery = encodeURI("select  item.condition, item.forecast from weather.forecast where woeid = " + woeid);
  //var wxquery = encodeURI("select  item.condition, item.forecast from weather.forecast where woeid = " + woeid + " and u='c'");
  var wxurl = "http://query.yahooapis.com/v1/public/yql?q=" + wxquery + "&format=json";
  
  // Send request to Yahoo!
  xhrRequest(wxurl, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      // Do this if forecast query succeeds
      if (json) {
        // Weather
        // Text to replace in returned conditions for the sake of screen space
        var texttoreplace = {
          "Heavy":"Hvy",
          "Freezing":"Frz",
          "Isolated":"Iso",
          "Light":"Lt",
          "Scattered":"Scatt",
          "Severe":"Sev",
          "Snow Showers Early":"Snow Shwrs Early",
          "Thunderstorms":"Tstorms",
          "With":"w/"
        };
        var repstring = new RegExp(Object.keys(texttoreplace).join("|"),"gi");
        
        var temperature = json.query.results.channel[0].item.condition.temp;
        var temperature_c = Math.round((temperature - 32) * (5/9));
        var conditionstext = json.query.results.channel[0].item.condition.text;
        var conditions = conditionstext.replace(repstring, function(matched) {
          return texttoreplace[matched];
        });
        var templow = json.query.results.channel[0].item.forecast.low;
        var templow_c = Math.round((templow - 32) * (5/9));
        var temphigh = json.query.results.channel[0].item.forecast.high;
        var temphigh_c = Math.round((temphigh - 32) * (5/9));
        var forecasttext = json.query.results.channel[0].item.forecast.text;
        var forecast = forecasttext.replace(repstring, function(matched) {
          return texttoreplace[matched];
        });
        var timestamp = new Date();
        var hours = "0" + timestamp.getHours();
        var minutes = "0" + timestamp.getMinutes();
        var lastupdate = hours.substr(hours.length-2) + ":" + minutes.substr(minutes.length-2);
      
        console.log("Current Temp: " + temperature);
        console.log("Current Temp (C): " + temperature_c);
        console.log("Low Temp: " + templow);
        console.log("Low Temp (C): " + templow_c);
        console.log("High Temp: " + temphigh);
        console.log("High Temp (C): " + temphigh_c);
        console.log("Conditions: " + conditions);
        //console.log("Forecast: " + forecast);
        console.log("Last update: " + timestamp);
        
        // Assemble dictionary using our keys
        var dictionary = {
          "KEY_LOCATION": location,
          "KEY_TEMPERATURE": temperature + "\u00B0",
          "KEY_TEMPERATURE_C": temperature_c + "\u00B0",
          "KEY_TEMPLOW": templow,
          "KEY_TEMPLOW_C": templow_c,
          "KEY_TEMPHIGH": temphigh,
          "KEY_TEMPHIGH_C": temphigh_c,
          "KEY_CONDITIONS": conditions,
          //"KEY_FORECAST": forecast, // Maybe I don't want the forecast text included.
          "KEY_LASTUPDATE": lastupdate
        };

        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Weather and location info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending weather and location info to Pebble!");
          }
        );
      }
      else {
        // Do this if forecast query fails
      }
    }      
  );
}

function locationError(err) {
  // Assemble dictionary using our keys
  var dictionary = {
    "KEY_LOCATION": "Unable to Locate",
    "KEY_CONDITIONS": "Weather Unknown"
  };

  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log("Weather and location info sent to Pebble successfully!");
    },
    function(e) {
      console.log("Error sending weather and location info to Pebble!");
    }
  );
  
  Pebble.showSimpleNotificationOnPebble("WeatherFish", "Unable to get location coordinates from phone! Check connectivity.");
}

function locationSuccess(pos) {
  getLocationFromCoords(pos.coords.latitude, pos.coords.longitude);
  var coords = pos.coords;
  var latcoord = coords.latitude;
  var loncoord = coords.longitude;
  var accuracy = coords.accuracy;
  
  console.log("Coordinates: " + latcoord + ", " + loncoord + " (" + accuracy.toFixed(0) + "m accuracy)");
}

function getWeather() {
  // Get location from phone
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {enableHighAccuracy: true, timeout: 15000, maximumAge: 15000}
  );
  console.log("Getting location...");
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    getWeather();
  }
);

//Pebble.addEventListener('showConfiguration', function(e) {
//Show config page
  //Pebble.openURL('http://michaelfisher.github.io/WeatherFish/settings.html');
//});


//Pebble.addEventListener("webviewclosed", function(e) {
    //console.log("Configuration closed.");
    //if (e.response !== '') {
      //var configuration = JSON.parse(decodeURIComponent(e.response));
      //console.log('Configuration window returned: ', JSON.stringify(configuration));
    //}
    //else {
      //console.log("No options received.");
    //}
//});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    var response = JSON.stringify(e.payload);
    if (response.search("Update requested.") != -1) {
      console.log("Update requested!");
      
      getWeather();
    }
    else {
      console.log("AppMessage received! " + response);
    }
  }                     
);