/*************************************************************************
 * Project THDweeter
 * Description: temperature & umidity dweeter
 * Author: Giuseppe Lo Presti
 *
 * Changelog:
 * v1.0 - 1-05-2017: initial revision from https://www.hackster.io/shaiss
 * v2.0 - 6-05-2017: added wifi sleep + readout button, refactorization
 * v2.1 - 7-05-2017: dropped LED, minor changes
 * v2.2 - 8-05-2017: added daily max and min reports, use Time
 *************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

int dhtPin = 2;                        // DHT sensor pin
int pushButtonPin = D5;                // push button pin
int readoutTimeIntervalMin = 3;        // perform a readout every given minutes
String thingName = "glp_photon_vth";   // thing name for Dweeter

DHT dht(dhtPin, DHT22);
HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
  //  { "Content-Type", "application/json" },
  //  { "Accept" , "application/json" },
  { "Accept" , "*/*"},
  { NULL, NULL }  // NOTE: Always terminate headers will NULL
};
http_request_t request;
http_response_t response;
long lastReadoutTime = 0;       // timestamp of the last readout
float temp, tmax, tmin, hum, hmax, hmin;    // current and daily max and min values
int day = 0;

void setup() {
 	dht.begin();
  pinMode(pushButtonPin, INPUT_PULLUP);  // input with an internal pull-up resistor
}


int getAndStoreValues() {
  // reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor). Also the lib has a typo...
  temp = dht.getTempCelcius();
  hum = dht.getHumidity();
  //float hi = dht.getHeatIndex();
  //float dp = dht.getDewPoint();
  // Check if any reads failed and exit early
  if (isnan(hum) || isnan(temp)) {
    //Particle.publish("DEBUG", "Failed to read from DHT sensor!");
    return -1;
  }
  // reset every day the max and min values
  if(Time.day() != day) {
    tmin = tmax = temp;
    hmin = hmax = hum;
    day = Time.day();
  }
  // and update them at every readout
  if(temp < tmin) {
    tmin = temp;
  }
  else if(temp > tmax) {
    tmax = temp;
  }
  if(hum < hmin) {
    hmin = hum;
  }
  else if(hum > hmax) {
    hmax = hum;
  }
  return 0;
}


void readout(bool button) {
  // First wake up WiFi
  WiFi.on();
  WiFi.connect();
  // get and store current values
  if(getAndStoreValues()) {
    // failed to read out, try again right away
    lastReadoutTime = 0;
    return;
  }
  // cast floats to strings
  String st(temp, 1);
  String sh(hum, 1);
  // make sure WiFi is ready
  while(!WiFi.ready()) {
    delay(300);
  }
  // log what we're doing
  if(button) {
    Particle.publish("STATE", "Button pressed, data readout");
  }
  else {
    Particle.publish("STATE", "Periodic data readout");
  }
  // post to dweet: follow at http://dweet.io/follow/thingName
 	request.hostname = "dweet.io";
  request.port = 80;
  request.path = "/dweet/for/" + thingName +
      "?temp=" + st + "&humidity=" + sh + "&time=" + Time.now();
  if(button) {
    // also publish daily max and min values
    String stmin(tmin, 1);
    String stmax(tmax, 1);
    String shmin(hmin, 1);
    String shmax(hmax, 1);
    request.path += "&tmin=" + stmin + "&tmax=" + stmax +
        "&hmin=" + shmin + "&hmax=" + shmax;
  }
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
    // and keep WiFi on for other photon activities/upgrades
  }
  else {
    // force a readout after the given time interval
    if(Time.now() - lastReadoutTime >= readoutTimeIntervalMin*60) {
      // for regular readouts, record the time of this readout
      lastReadoutTime = Time.now();
      readout(false);
      delay(1000);
      // turn off WiFi to spare power
      WiFi.off();
    }
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}
