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
float lux, lmin, lmax;
int day = 0;


void setup() {
  Time.zone(+2);     // set your time zone if you wish a local time display
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


int doReadout(bool button) {
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
  if(!Particle.publish("Lux", payload, PRIVATE, WITH_ACK)) {
    return -1;
  }
  return 0;
}


void loop() {
  doReadout(true);
  delay(2000);
}
