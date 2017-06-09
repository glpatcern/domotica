/******************************************************************************
 * Project THDweeter
 * Description: temperature & umidity dweeter
 * Author: Giuseppe Lo Presti, glopresti@gmail.com
 *
 * Changelog:
 * v1.0 - 01-05-2017: initial revision from https://www.hackster.io/shaiss
 * v2.0 - 06-05-2017: added wifi sleep + readout button, refactorization
 * v2.1 - 07-05-2017: dropped LED, minor changes
 * v2.2 - 08-05-2017: added daily max and min reports, use Time
 * v2.3 - 06-06-2017: minor refactoring, added heat index, reduced logs
 * v2.4 - 09-06-2017: rough calibration for humidity, better WiFi handling
 ******************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

int dhtPin = 2;                        // DHT sensor pin
int pushButtonPin = D5;                // push button pin
int readoutTimeIntervalMins = 5;       // perform a readout every given minutes
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
long lastReadoutTime = 0;       // timestamp of the last readout
float temp, hum, hi,            // current and daily max and min values
      tmax, tmin, hmax, hmin;
int day = 0;

void setup() {
  Time.zone(+2);     // set your time zone if you wish a local time display
  dht.begin();
  pinMode(pushButtonPin, INPUT_PULLUP);  // input with an internal pull-up resistor
}


int getAndStoreValues() {
  // reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor). Also, the lib has a typo...
  temp = dht.getTempCelcius();
  hum = dht.getHumidity();
  //dp = dht.getDewPoint();
  // Check if any reads failed and exit early
  if (isnan(hum) || isnan(temp)) {
    //Particle.publish("DEBUG", "Failed to read from DHT sensor!");
    return -1;
  }
  // apply a correction factor to the humidity reading, following
  // experimental observations: the factor is 2 up to 30C and 2.2 at 40C
  hum = hum * (2 + (temp > 30 ? (temp-30)/50 : 0));
  // get the felt temperature only when it makes sense
  if(temp >= 25 && hum >= 35) {
    // this equation comes from the DHT library at:
    // https://github.com/zackzachariah/hacknest/blob/master/sensor/Adafruit_DHT.cpp
    float tempF = dht.getTempFarenheit();
    hi = ((-42.379 +
             2.04901523 * tempF +
            10.14333127 * hum +
            -0.22475541 * tempF * hum +
            -0.00683783 * pow(tempF, 2) +
            -0.05481717 * pow(hum, 2) +
             0.00122874 * pow(tempF, 2) * hum +
             0.00085282 * tempF * pow(hum, 2) +
            -0.00000199 * pow(tempF, 2) * pow(hum, 2)
          ) - 32) * 5/9;    // convert back to Celsius
  }
  else {
    hi = 0.0;
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


int readout(bool button) {
  http_request_t request;
  http_response_t response;
  // wake up WiFi
  WiFi.on();
  WiFi.connect();
  // get and store current values
  if(getAndStoreValues()) {
    // failed to read out, try again right away
    lastReadoutTime = 0;
    return -1;
  }
  // cast floats to strings
  String st(temp, 1);
  String sh(hum, 0);
  String shi(hi, 1);
  String stmin(tmin, 1);
  String stmax(tmax, 1);
  String shmin(hmin, 0);
  String shmax(hmax, 0);
  // generate the payload for dweet.io
  unsigned long now = Time.now();
  String payload = "temp=" + st + "&humidity=" + sh +
      (hi > 0 ? "&heatindex=" + shi : "") +
      // also publish daily max and min values when button pressed
      (button ? "&tmin=" + stmin + "&tmax=" + stmax +
                "&hmin=" + shmin + "&hmax=" + shmax : "") +
      "&timestamp=" + now + "&time=" + Time.format(now, "%Y-%m-%dT%H:%M:%S");
  // post to dweet: follow at http://dweet.io/follow/thingName
  request.hostname = "dweet.io";
  request.port = 80;
  request.path = "/dweet/for/" + thingName + "?" + payload;
  // make sure WiFi is ready
  int i = 0;
  while(!WiFi.ready()) {
    delay(300);
    if(++i == 100) {
      // the WiFi did not connect for 30 seconds, give up
      return -1;
    }
  }
  if(button) {
    Particle.publish("Button triggered readout", payload);
  }
  else {
    Particle.publish("Periodic readout", payload);
  }
  http.get(request, response, headers);
  Particle.publish("Dweet", response.body);
  return 0;
}


void loop() {
  int pushButtonState = digitalRead(pushButtonPin);
  if(pushButtonState == LOW) {
    // button was pressed, readout
    readout(true);
    // and keep WiFi on for other Photon activities/upgrades
  }
  else {
    // force a readout after the given time interval
    if(Time.now() - lastReadoutTime >= readoutTimeIntervalMins*60) {
      // for regular readouts, record the time of this readout: this way,
      // we make the readings as evenly spread in time as possible
      lastReadoutTime = Time.now();
      readout(false);
      // yield to the Particle cloud
      Particle.process();
      // and turn off WiFi to spare power
      WiFi.off();
    }
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}
