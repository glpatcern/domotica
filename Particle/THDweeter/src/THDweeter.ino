/*************************************************************************
 * Project THDweeter
 * Description: temperature & umidity dweeter
 * Author: Giuseppe Lo Presti
 *
 * Changelog:
 * v1.0 - 1-05-2017: initial revision from https://www.hackster.io/shaiss
 * v2.0 - 6-05-2017: added wifi sleep + readout button + refactorization
 *
 *************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

int dhtPin = 2;           // DHT sensor
int ledPin = D7;          // Internal LED
int pushButtonPin = D5;   // Push button
int readoutTimeIntervalMin = 3;   // perform a readout every given minutes

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
int t = readoutTimeIntervalMin*5*60;    // time counter: force a readout at startup

void setup() {
  Time.zone(+1);
 	dht.begin();
  pinMode(ledPin, OUTPUT);               // D7 pin is output
  pinMode(pushButtonPin, INPUT_PULLUP);  // input with an internal pull-up resistor
}

void readout(bool button) {
  // First wake up WiFi
  WiFi.on();
  WiFi.connect();
  // Switch on LED to signal we're reading out the sensor data
  digitalWrite(ledPin, HIGH);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor)
 	float h = dht.getHumidity();
  // Read temperature as Celsius
 	float t = dht.getTempCelcius();
  // Check if any reads failed and exit early (to try again).
 	if (isnan(h) || isnan(t)) {
    //Particle.publish("DEBUG", "Failed to read from DHT sensor!");
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
  // post to dweet: follow at http://dweet.io/follow/glp_photon_vth
 	request.hostname = "dweet.io";
  request.port = 80;
  request.path = "/dweet/for/glp_photon_vth?temp=" + st + "&humidity=" + sh + "&time=" + Time.local();
 	Particle.publish("DWEET", request.path);
  http.get(request, response, headers);
  //Particle.publish("DEBUG",response.status);
  Particle.publish("DWEET", response.body);
  // Done, switch off LED
  digitalWrite(ledPin, LOW);
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
    if(t >= readoutTimeIntervalMin*5*60) {
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
