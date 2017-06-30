/******************************************************************************
 * Project LuxSensor
 * Description: test for a light sensor
 * Author: Giuseppe Lo Presti, glopresti@gmail.com
 *
 * Changelog:
 * v1.0 - 22-06-2017: initial fork from THDweeter to experiment with analogRead
 * v2.0 - 29-06-2017: use the Particle cloud API instead of dweet.io and include
 *                    a state machine to properly handle reconnections
 ******************************************************************************/

int photoPin = A5;           // photoresistor pin
int powerPin = A0;           // the other leg of the photoresistor
int readoutTimeIntervalMins = 2;    // perform a readout every given minutes

const int PHOTON_OFFLINE = 0;
const int PHOTON_CONNECTING = 1;
const int PHOTON_ONLINE = 2;

long lastReadoutTime = 0;    // timestamp of the last readout
float lux, lmin, lmax;
int day = 0;
int photonState = PHOTON_ONLINE;
bool buttonPressed = false;

void setup() {
  pinMode(photoPin, INPUT);     // prepare the photoresistor
  pinMode(powerPin, OUTPUT);
  pinMode(D7, OUTPUT);       // the integrated LED signals readout errors
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
  // publish data: ttl = 60 for now, in the future it may be readoutTimeIntervalMins*60
  if(!Particle.publish("luxdata", payload, PRIVATE, WITH_ACK)) {
    return -1;
  }
  return 0;
}


void loop() {
  int pushButtonState = HIGH;
  if(pushButtonState == LOW) {
    // button was pressed, connect for an immediate readout
    Particle.connect();
    // turn off the LED in case it was on
    digitalWrite(D7, LOW);
    buttonPressed = true;
    photonState = PHOTON_CONNECTING;
    return;
  }
  else {
    // force a readout after the given time interval
    if(Time.now() - lastReadoutTime >= readoutTimeIntervalMins*60) {
      // for regular readouts, record the time of this readout: this way,
      // we make the readings as evenly spread in time as possible
      lastReadoutTime = Time.now();
      // connect to WiFi and the cloud (this is asynchronous)
      Particle.connect();
      // turn off the LED in case it was on
      digitalWrite(D7, LOW);
      buttonPressed = false;
      photonState = PHOTON_CONNECTING;
      return;
    }
  }
  if(photonState == PHOTON_CONNECTING) {
      // make sure we're online
      waitUntil(Particle.connected);
      // do the readout
      if(readout(buttonPressed)) {
        // something went wrong, signal the error by blinking the LED
        digitalWrite(D7, HIGH);
        // and don't change state, to try and read out right away
      }
      else {
        // all right, go in ONLINE mode to let the Particle cloud
        // effectively publish the data. Switching the WiFi off now
        // without returning control to the main firmware would make
        // the Publish ineffective!
        photonState = PHOTON_ONLINE;
      }
      return;
  }
  if(photonState == PHOTON_ONLINE && !buttonPressed) {
    // still wait a bit
    delay(1000);
    // and go offline
    WiFi.off();
    photonState = PHOTON_OFFLINE;
  }
  // loop at 5 Hz to catch button press events
  delay(200);
}
