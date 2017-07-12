/******************************************************************************
 * Project THLMqttSensor
 * Description: An MQTT-enabled temperature-humidity-luminosity sensor
 * Author: Giuseppe Lo Presti, glopresti@gmail.com
 *
 * Changelog:
 * v1.0 - 30-06-2017: initial revision evolved from LuxSensor
 * v2.0 - 02-07-2017: merged in THDweeter
 ******************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include <MQTT.h>

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

SYSTEM_MODE(SEMI_AUTOMATIC);    // do not use the Particle cloud

int dhtPin = 2;              // DHT sensor pin
int pushButtonPin = D5;      // push button pin
int photoPin = A5;           // photoresistor pin
int powerPin = A0;           // the other leg of the photoresistor
int readoutTimeIntervalMins = 2;    // perform a readout every given minutes

DHT dht(dhtPin, DHT22);

/**
 * if want to use IP address,
 * byte server[] = { XXX,XXX,XXX,XXX };
 * MQTT client(server, 1883, callback);
 **/
MQTT client("iot.eclipse.org", 1883, NULL);   // for testing purposes

long lastReadoutTime = 0;    // timestamp of the last readout
float temp, hum, lux, hi,            // current and daily max and min values
      tmax, tmin, hmax, hmin, lmin, lmax;
int day = 0;
bool buttonPressed = false;

void setup() {
  dht.begin();
  pinMode(pushButtonPin, INPUT_PULLUP);  // input with an internal pull-up resistor
  pinMode(photoPin, INPUT);     // prepare the photoresistor
  pinMode(powerPin, OUTPUT);
  pinMode(D7, OUTPUT);       // the integrated LED signals readout errors
  digitalWrite(powerPin, HIGH);
  // first connection to the MQTT broker
  client.connect("glpphoton");
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
  // still, in direct sunlight the reading is too low:
  // override with artificial minimum
  if(hum < 15) {
    hum = 15;
  }
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
  // and now for the luminosity reading
  lux = (analogRead(photoPin) - 1230) * 100.0 / (4095 - 1230);    // scale to [0..100]
  if(lux < 0) {
      lux = 0;
  }
  // reset every day the max and min values
  if(Time.day() != day) {
    day = Time.day();
    tmin = tmax = temp;
    hmin = hmax = hum;
    lmin = lmax = lux;
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
  if(lux < lmin) {
    lmin = lux;
  }
  else if(lux > lmax) {
    lmax = lux;
  }
  return 0;
}


int readout(bool button) {
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
  String sl(lux, 1);
  String stmin(tmin, 1);
  String stmax(tmax, 1);
  String shmin(hmin, 0);
  String shmax(hmax, 0);
  String slmin(lmin, 1);
  String slmax(lmax, 1);
  // generate the payload
  String payload = "{temp: " + st + ", humidity: " + sh +
      (hi > 0 ? ", heatindex: " + shi : "") +
      ", lux: " + sl +
      // also publish daily max and min values when button pressed
      (button ? ", tmin: " + stmin + ", tmax: " + stmax +
                ", hmin: " + shmin + ", hmax: " + shmax +
                ", lmin: " + slmin + ", lmax: " + slmax : "") +
      "}";
  // make sure we're online
  waitUntil(WiFi.ready);
  // connect to the MQTT broker
  client.connect("glpphoton");
  // publish data to the MQTT broker
  if(!client.publish("thlveranda", payload)) {
    return -1;
  }
  // clear the LED in case of previous errors
  digitalWrite(D7, LOW);
  return 0;
}


void loop() {
  // handle button and periodic readouts
  int pushButtonState = digitalRead(pushButtonPin);
  if(pushButtonState == LOW) {
    // button was pressed, readout
    if(readout(true)) {
      // something went wrong, signal the error by switching the LED on
      digitalWrite(D7, HIGH);
    }
    // run a loop of the MQTT client lib
    client.loop();
    // connect to the Particle cloud
    Particle.connect();
    // and keep WiFi on for other Photon activities/upgrades
  }
  else {
    // force a readout after the given time interval
    if(Time.now() - lastReadoutTime >= readoutTimeIntervalMins*60) {
      // for regular readouts, record the time of this readout: this way,
      // we make the readings as evenly spread in time as possible
      lastReadoutTime = Time.now();
      if(readout(false)) {
        // something went wrong, signal the error by switching the LED on
        digitalWrite(D7, HIGH);
      }
      else {
        // run a loop of the MQTT client lib
        client.loop();
        // all right, turn off WiFi to spare power
        WiFi.off();
      }
    }
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}