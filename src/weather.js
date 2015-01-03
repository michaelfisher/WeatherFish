var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getLocation(latitude, longitude) {
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
        if (statecode != null) {
          var location = json.query.results.Result.city + ", " + json.query.results.Result.statecode;
        } else {
          var location = json.query.results.Result.city + ", " + json.query.results.Result.countrycode;
        }
        var woeid = json.query.results.Result.woeid;
      
        // Weather
        getWeatherFromWOEID(woeid, location);
      }
      else {
        // Do this if placefinder query fails
        var location = "Unable to get location from coordinates!";
        
        var dictionary = {
          "KEY_LOCATION": location
        };
        
        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Location info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending location info to Pebble!");
          }
        );
        
        console.log("Unable to get location from coordinates!");
      }
    }      
  );
}

function getWeatherFromWOEID(woeid, location) {
  var wxquery = encodeURI("select  item.condition, item.forecast from weather.forecast where woeid = " + woeid);
	var wxurl = "http://query.yahooapis.com/v1/public/yql?q=" + wxquery + "&format=json";
  
  // Send request to Yahoo!
  xhrRequest(wxurl, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      // Do this if forecast query succeeds
      if (json) {
        // Weather
        var temperature = json.query.results.channel[0].item.condition.temp;
        var conditions = json.query.results.channel[0].item.condition.text;
        var templow = json.query.results.channel[0].item.forecast.low;
        var temphigh = json.query.results.channel[0].item.forecast.high;
        var forecast = json.query.results.channel[0].item.forecast.text;
        var timestamp = new Date();
        var hours = "0" + timestamp.getHours();
        var minutes = "0" + timestamp.getMinutes();
        var lastupdate = hours.substr(hours.length-2) + ":" + minutes.substr(minutes.length-2);
      
        console.log("Location: " + location);
        console.log("WOEID: " + woeid);
        console.log("Current Temp: " + temperature);
        console.log("Low Temp: " + templow);
        console.log("High Temp: " + temphigh);
        console.log("Conditions: " + conditions);
        console.log("Forecast: " + forecast);
        console.log("Last update: " + timestamp);
      
        // Assemble dictionary using our keys
        var dictionary = {
          "KEY_LOCATION": location,
          "KEY_TEMPERATURE": temperature + "\u00B0",
          "KEY_CONDITIONS": conditions,
          "KEY_TEMPLOW": templow,
          "KEY_TEMPHIGH": temphigh,
          "KEY_FORECAST": forecast,
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
    }      
  );
}

function locationError(err) {
  // Assemble dictionary using our keys
  var dictionary = {
    "KEY_LOCATION": "Unable to locate!"
  };

  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log("Message info sent to Pebble successfully!");
    },
    function(e) {
      console.log("Error sending message info to Pebble!");
    }
  );
      
  // Send notification to Pebble
  Pebble.showSimpleNotificationOnPebble("WeatherFish", "Unable to get location. Check connectivity.");
  
  console.log("Unable to locate before timeout!");
}

function locationSuccess(pos) {
  getLocation(pos.coords.latitude, pos.coords.longitude);
  console.log("Coordinates: " + pos.coords.latitude + ", " + pos.coords.longitude);
}

function getWeather() {
  // Assemble dictionary using our keys
  var dictionary = {
    "KEY_LOCATION": "Locating..."
  };

  // Send to Pebble
  Pebble.sendAppMessage(dictionary);
  
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Setup configuration page
Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://michaelfisher.github.io/WeatherFish/settings.html');
});

// Listen for setting page close
Pebble.addEventListener('webviewclosed',
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));
  }
);


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    getWeather();
  }
);