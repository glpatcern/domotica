/******************************************************************************
 * Project THLMqttSensor
 * Description: An MQTT-enabled temperature-humidity-luminosity sensor
 * Author: Giuseppe Lo Presti, glopresti@gmail.com
 *
 * Changelog:
 * v1.0 - 30-06-2017: initial revision merging THDweeter and LuxSensor
 ******************************************************************************/

// This #include statement was automatically added by the Particle IDE.
#include <MQTT.h>

SYSTEM_MODE(SEMI_AUTOMATIC);    // do not use the Particle cloud

int photoPin = A5;           // photoresistor pin
int powerPin = A0;           // the other leg of the photoresistor
int readoutTimeIntervalMins = 2;    // perform a readout every given minutes

/**
 * if want to use IP address,
 * byte server[] = { XXX,XXX,XXX,XXX };
 * MQTT client(server, 1883, callback);
 **/
MQTT client("broker.hivemq.com", 1883, callback);

long lastReadoutTime = 0;    // timestamp of the last readout
float lux, lmin, lmax;
int day = 0;
bool buttonPressed = false;

void setup() {
  pinMode(photoPin, INPUT);     // prepare the photoresistor
  pinMode(powerPin, OUTPUT);
  pinMode(D7, OUTPUT);       // the integrated LED signals readout errors
  digitalWrite(powerPin, HIGH);

  // connect to the MQTT server
  client.connect("glp_photon");

  // publish/subscribe
  if(client.isConnected()) {
    client.publish("message","hello world");
    client.subscribe("readout");
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  //char p[length + 1];
  //memcpy(p, payload, length);
  //p[length] = NULL;
  // signal we got the callback  
  digitalWrite(D7, HIGH);
  delay(500);
  digitalWrite(D7, LOW);
  delay(100);
  digitalWrite(D7, HIGH);
  delay(500);
  digitalWrite(D7, LOW);
  // and force a readout as if the button were pressed
  readout(true);
}


int getAndStoreValues() {
  lux = analogRead(photoPin) / 4096.0 * 100;    // scale to [0..100]
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
  String sl(lux, 0);
  String slmin(lmin, 0);
  String slmax(lmax, 0);
  // generate the payload
  String payload = "{lux: " + sl +
      // also publish daily max and min values when button pressed
      (button ? ", lmin: " + slmin + ", lmax: " + slmax : "") +
      "}";
  // make sure we're online
  waitUntil(WiFi.ready);
  // publish data to the MQTT broker
  if(!client.publish("luxdata", payload)) {
    return -1;
  }
  digitalWrite(D7, LOW);
  return 0;
}


void loop() {
  // main MQTT client loop
  if (client.isConnected())
    client.loop();
  // handle button and periodic readouts
  int pushButtonState = HIGH;
  if(pushButtonState == LOW) {
    // button was pressed, readout
    if(readout(true)) {
      // something went wrong, signal the error by switching the LED on
      digitalWrite(D7, HIGH);
    }
    // keep WiFi on for other Photon activities/upgrades
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
        // all right, turn off WiFi to spare power
        WiFi.off();
      }
    }
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}
