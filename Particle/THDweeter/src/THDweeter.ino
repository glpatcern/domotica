/*************************************************************************
 * Project THDweeter
 * Description: temperature & umidity dweeter
 * Author: Giuseppe Lo Presti
 *
 * Changelog:
 * v1.0 - 1-05-2017: initial revision from https://www.hackster.io/shaiss
 * v2.0 - 6-05-2017: added wifi sleep + readout button, refactorization
 * v2.1 - 7-05-2017: dropped LED, minor changes
 *************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

int dhtPin = 2;                        // DHT sensor pin
int pushButtonPin = D5;                // Push button pin
int readoutTimeIntervalMin = 3;        // perform a readout every given minutes
String thingName = "glp_photon_vth";   // thing name for Dweeter

DHT dht(dhtPin, DHT22);
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
  //  { "Content-Type", "application/json" },
  //  { "Accept" , "application/json" },
  { "Accept" , "*/*"},
  { NULL, NULL } // NOTE: Always terminate headers will NULL
};
http_request_t request;
http_response_t response;
int t = readoutTimeIntervalMin*60*5;    // time counter: force a readout at startup

void setup() {
  Time.zone(+1);
 	dht.begin();
  pinMode(pushButtonPin, INPUT_PULLUP);  // input with an internal pull-up resistor
}

void readout(bool button) {
  // First wake up WiFi
  WiFi.on();
  WiFi.connect();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor)
 	float h = dht.getHumidity();
  // Read temperature as Celsius - the lib has a typo...
 	float t = dht.getTempCelcius();
  // Check if any reads failed and exit early
 	if (isnan(h) || isnan(t)) {
    //Particle.publish("DEBUG", "Failed to read from DHT sensor!");
    // try again in 10 secs
    t = readoutTimeIntervalMin*60*5 - 10*5;
    return;
 	}
 	//float hi = dht.getHeatIndex();
 	//float dp = dht.getDewPoint();
 	// cast floats to string
 	String st(t, 1);
 	String sh(h, 1);
  // make sure WiFi is ready
  while(!WiFi.ready()) {
    delay(100);
  }
  if(button) {
    Particle.publish("STATE", "Button pressed, data readout");
  }
  else {
    Particle.publish("STATE", "Periodic data readout");
  }
  // post to dweet: follow at http://dweet.io/follow/THINGNAME
 	request.hostname = "dweet.io";
  request.port = 80;
  request.path = "/dweet/for/" + thingName +
      "?temp=" + st + "&humidity=" + sh + "&time=" + Time.local();
 	Particle.publish("DWEET", request.path);
  http.get(request, response, headers);
  //Particle.publish("DEBUG",response.status);
  Particle.publish("DWEET", response.body);
}

void loop() {
  int pushButtonState = digitalRead(pushButtonPin);
  if(pushButtonState == LOW) {
    // button was pressed, readout
    readout(true);
    // don't reset the timer, to keep the regular sampling
  }
  else {
    t++;
    // force a readout after the given time interval
    if(t >= readoutTimeIntervalMin*60*5) {
      readout(false);
      t = 0;
    }
    if(t == 10*5) {
      // turn off wifi to spare power, but allow a few seconds
      // for upgrades/other photon activities
      WiFi.off();
    }
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}
