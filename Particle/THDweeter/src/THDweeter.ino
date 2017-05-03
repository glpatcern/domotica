/*
 * Project THDweeter
 * Description:
 * Author:
 * Date:
 */
 // This #include statement was automatically added by the Particle IDE.
 #include "HttpClient/HttpClient.h"

 // This #include statement was automatically added by the Particle IDE.
 #include "Adafruit_DHT/Adafruit_DHT.h"

 // Example testing sketch for various DHT humidity/temperature sensors
 // Written by ladyada, public domain

 #define DHTPIN 2     // what pin we're connected to

 // Uncomment whatever type you're using!
 //#define DHTTYPE DHT11		// DHT 11
 #define DHTTYPE DHT22		// DHT 22 (AM2302)
 //#define DHTTYPE DHT21		// DHT 21 (AM2301)

 // Connect pin 1 (on the left) of the sensor to +5V
 // Connect pin 2 of the sensor to whatever your DHTPIN is
 // Connect pin 4 (on the right) of the sensor to GROUND
 // Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

 DHT dht(DHTPIN, DHTTYPE);

 /**
 * Declaring the variables.
 */
 unsigned int nextTime = 0;    // Next time to contact the server
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
 }

 void loop() {
 // Wait a minute between measurements.
 	delay(60000);

 // Reading temperature or humidity takes about 250 milliseconds!
 // Sensor readings may also be up to 2 seconds 'old' (its a
 // very slow sensor)
 	float h = dht.getHumidity();
 // Read temperature as Celsius
 	float t = dht.getTempCelcius();
 // Read temperature as Farenheit
 	float f = dht.getTempFarenheit();

 // Check if any reads failed and exit early (to try again).
 	if (isnan(h) || isnan(t)) {
 		Particle.publish("DEBUG","Failed to read from DHT sensor!");
 		return;
 	}

 // Compute heat index
 // Must send in temp in Fahrenheit!
 	float hi = dht.getHeatIndex();
 	float dp = dht.getDewPoint();
 	float k = dht.getTempKelvin();

 	//Serial.print("Humid: ");
 	//Serial.print(h);
 	//Serial.print("% - ");
 	//Serial.print("Temp: ");
 	//Serial.print(t);
 	//Serial.print("*C ");
 	//Serial.print(f);
 	//Serial.print("*F ");
 	//Serial.print(k);
 	//Serial.print("*K - ");
 	//Serial.print("DewP: ");
 	//Serial.print(dp);
 	//Serial.print("*C - ");
 	//Serial.print("HeatI: ");
 	//Serial.print(hi);
 	//Serial.println("*C");
 	//Serial.println(Time.timeStr());

 	//cast floats to string
 	String st(t, 2);
 	String sh(h, 2);

 	//pblish tests
 	//Spark.publish("faranheight",f,60,PRIVATE);
 	Particle.publish("TEMP",st);
 	Particle.publish("HUMIDITY",sh);

 	//post to dweet
 	request.hostname = "dweet.io";
     request.port = 80;
     request.path = "/dweet/for/glp_photon_verandath?temp=" + st + "&humidity=" + sh;

 	//json body doesn't work
 	//request.body = "{\"humidity\":\"43%\",\"temp\":\"45f\"}"; //"{\"key\":\"value\"}";

 	Particle.publish("DEBUG",request.body);

     // Get request
     http.get(request, response, headers);
     //Spark.publish("DEBUG","Application Response status: ");
     //Particle.publish("DEBUG",response.status);

     //Spark.publish("DEBUG","Application HTTP Response Body: ");
     Particle.publish("DEBUG",response.body);
 }
