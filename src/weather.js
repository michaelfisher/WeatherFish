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
  var tempunit = localStorage.getItem(8);
  if (tempunit !== null) {
    var wxquery = encodeURI("select  item.condition, item.forecast from weather.forecast where woeid = " + woeid + " and u='" + tempunit + "'");
  }
  else {
    var wxquery = encodeURI("select  item.condition, item.forecast from weather.forecast where woeid = " + woeid);
  }
  
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
        var conditionstext = json.query.results.channel[0].item.condition.text;
        var conditions = conditionstext.replace(repstring, function(matched) {
          return texttoreplace[matched];
        });
        var templow = json.query.results.channel[0].item.forecast.low;
        var temphigh = json.query.results.channel[0].item.forecast.high;
        var forecasttext = json.query.results.channel[0].item.forecast.text;
        var forecast = forecasttext.replace(repstring, function(matched) {
          return texttoreplace[matched];
        });
        var timestamp = new Date();
        var hours = "0" + timestamp.getHours();
        var minutes = "0" + timestamp.getMinutes();
        var lastupdate = hours.substr(hours.length-2) + ":" + minutes.substr(minutes.length-2);
      
        console.log("Current Temp: " + temperature);
        console.log("Low Temp: " + templow);
        console.log("High Temp: " + temphigh);
        console.log("Conditions: " + conditions);
        //console.log("Forecast: " + forecast);
        console.log("Last update: " + timestamp);
        
        // Assemble dictionary using our keys
        var dictionary = {
          "KEY_LOCATION": location,
          "KEY_TEMPERATURE": temperature + "\u00B0",
          "KEY_TEMPLOW": templow,
          "KEY_TEMPHIGH": temphigh,
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
    {timeout: 15000, maximumAge: 0}
  );
  console.log("Getting location...");
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    getWeather();
  }
);

// Listen for the configuration page to open
Pebble.addEventListener("showConfiguration", function() {
  console.log("Showing Configuration...");
  Pebble.openURL('https://s3.amazonaws.com/michael.fisher/WeatherFish/settings.html?tempunit=' + localStorage.getItem(8));
  console.log('https://s3.amazonaws.com/michael.fisher/WeatherFish/settings.html?tempunit=' + localStorage.getItem(8));
});

// Listen for the configuration page to close
Pebble.addEventListener("webviewclosed", function(e) {
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log("Configuration closed.");
  localStorage.setItem(8, (options.KEY_TEMPUNIT));
  
  getWeather();
});

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