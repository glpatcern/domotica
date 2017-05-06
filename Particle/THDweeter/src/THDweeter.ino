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

#define DHTPIN 2        // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)

int led = D7;           // internal LED
int pushButton = D5;    // Push button is connected to D5
int i = 0;              // time counter

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);
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

void setup() {
 	Particle.publish("DEBUG","DHT22 veranda");
 	dht.begin();
  pinMode(led, OUTPUT);               // D7 pin is output
  pinMode(pushButton, INPUT_PULLUP);  // input with an internal pull-up resistor
}

void readout() {
  // First wake up WiFi
  //Wifi.on();
  // Switch on LED to signal we're reading out the sensor data
  digitalWrite(led, HIGH);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor)
 	float h = dht.getHumidity();
  // Read temperature as Celsius
 	float t = dht.getTempCelcius();

  // Check if any reads failed and exit early (to try again).
 	if (isnan(h) || isnan(t)) {
    Particle.publish("DEBUG","Failed to read from DHT sensor!");
 	  return;
 	}
 	//float hi = dht.getHeatIndex();
 	//float dp = dht.getDewPoint();

 	// cast floats to string
 	String st(t, 2);
 	String sh(h, 2);

 	//post to dweet
 	request.hostname = "dweet.io";
  request.port = 80;
  request.path = "/dweet/for/glp_photon_vth?temp=" + st + "&humidity=" + sh + "&time=" + Time.timeStr();
 	Particle.publish("Sensor data", request.path);
  http.get(request, response, headers);
  //Particle.publish("DEBUG",response.status);
  Particle.publish("DEBUG", response.body);

  // Done, switch off LED
  digitalWrite(led, LOW);
  // and turn off wifi
  //Wifi.off()
 }

void loop() {
  int pushButtonState = digitalRead(pushButton);
  if(pushButtonState == LOW) {
    // button was pressed, readout
    Particle.publish("BUTTON", "Button pressed, reading data");
    readout();
    i = 0;
  }
  else {
    i++;
    // after 5 minutes force a readout
    if(i == 100) {
      readout();
    }
  }
  delay(100);
}
