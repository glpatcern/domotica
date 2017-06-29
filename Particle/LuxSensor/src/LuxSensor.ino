/******************************************************************************
 * Project LuxSensor
 * Description: test for a light sensor
 * Author: Giuseppe Lo Presti, glopresti@gmail.com
 *
 * Changelog:
 * v1.0 - 22-06-2017: initial fork from THDweeter to experiment with analogRead
 * v2.0 - 28-06-2017: use the Particle cloud API instead of dweet.io
 ******************************************************************************/

int photoPin = A5;           // photoresistor pin
int powerPin = A0;
int readoutTimeIntervalMins = 1;   // perform a readout every given minutes

long lastReadoutTime = 0;    // timestamp of the last readout
float lux, lmin, lmax;
int day = 0;
bool connecting = false;


void setup() {
  pinMode(photoPin, INPUT);     // prepare the photoresistor
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


int readout(bool button) {
  // get and store current values
  if(getAndStoreValues()) {
    // failed to read out, try again right away
    return -1;
  }
  // cast floats to strings
  String sl(lux, 3);
  String slmin(lmin, 3);
  String slmax(lmax, 3);
  // generate the payload
  String payload = "{lux: " + sl +
      // also publish daily max and min values when button pressed
      (button ? ", lmin: " + slmin + ", lmax: " + slmax : "") +
      "}";
  // publish data
  if(!Particle.publish("luxdata", payload, PRIVATE, WITH_ACK)) {
    return -1;
  }
  return 0;
}


void loop() {
  int pushButtonState = HIGH;
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
      connecting = true;
      Particle.connect();
      lastReadoutTime = Time.now();
      return;
    }
    if(connecting) {
      connecting = false;
      digitalWrite(D7, LOW);
      if(!readout(false)) {
        digitalWrite(D7, HIGH);
      }
    }
    if(Time.now() - lastReadoutTime >= 2) {
      // turn off WiFi to spare power after 20 secs of inactivity
      WiFi.off();
      digitalWrite(D7, LOW);
    }
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}
