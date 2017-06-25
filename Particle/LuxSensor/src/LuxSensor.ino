/******************************************************************************
 * Project LuxSensor
 * Description: test for a light sensor
 * Author: Giuseppe Lo Presti, glopresti@gmail.com
 *
 * Changelog:
 * v1.0 - 22-06-2017: initial fork from THDweeter to experiment with analogRead
 ******************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>

int photoPin = A5;                      // photoresistor pin
int powerPin = A0;
String thingName = "glp_photon_lux";    // thing name for Dweeter

HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
  //  { "Content-Type", "application/json" },
  //  { "Accept" , "application/json" },
  { "Accept" , "*/*"},
  { NULL, NULL }  // NOTE: Always terminate headers will NULL
};
float lux, lmin, lmax;
int day = 0;


void setup() {
  Time.zone(+2);     // set your time zone if you wish a local time display
  pinMode(photoPin, INPUT);       // prepare the photoresistor
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
}


int getAndStoreValues() {
  lux = analogRead(photoPin) / 4096.0;    // scale to [0..1]
  // reset every day the max and min values
  if(Time.day() != day) {
    day = Time.day();
    lmin = lmax = lux;
  }
  // and update them at every readout
  if(lux < lmin) {
    lmin = lux;
  }
  else if(lux > lmax) {
    lmax = lux;
  }
  return 0;
}


int doReadout(bool button) {
  http_request_t request;
  http_response_t response;
  // get and store current values
  if(getAndStoreValues()) {
    // failed to read out, try again right away
    return -1;
  }
  // cast floats to strings
  String sl(lux, 3);
  String slmin(lmin, 3);
  String slmax(lmax, 3);
  // generate the payload for dweet.io
  unsigned long now = Time.now();
  String payload = "lux=" + sl +
      // also publish daily max and min values when button pressed
      (button ? "&lmin=" + slmin + "&lmax=" + slmax : "") +
      "&timestamp=" + now + "&time=" + Time.format(now, "%Y-%m-%dT%H:%M:%S");
  // post to dweet: follow at http://dweet.io/follow/thingName
  request.hostname = "dweet.io";
  request.port = 80;
  request.path = "/dweet/for/" + thingName + "?" + payload;
  http.get(request, response, headers);
  //Particle.publish("Lux", sl);
  return 0;
}


void loop() {
  doReadout(true);
  delay(2000);
}
