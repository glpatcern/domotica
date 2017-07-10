int led = D7; // LED is connected to D0
int pushButton = D4; // Push button is connected to D2
int ledstate = LOW;

// This routine runs only once upon reset
void setup() 
{
  pinMode(led, OUTPUT); // Initialize D0 pin as output
  pinMode(pushButton, INPUT_PULLUP); 
  // Initialize D2 pin as input with an internal pull-up resistor
}

// This routine loops forever
void loop() 
{
  int pushButtonState; 

  pushButtonState = digitalRead(pushButton);

  if(pushButtonState == LOW)
  { // If we push down on the push button
      if (ledstate == LOW) {
        digitalWrite(led, HIGH);
        ledstate = HIGH;
      } else {
        digitalWrite(led, LOW);
        ledstate = LOW;
      }
  }  
}
