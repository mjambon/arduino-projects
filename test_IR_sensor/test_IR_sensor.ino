/*
  IR sensor test
*/

#define BLUE_LED 7
#define SIGNAL_PIN 2

void setup() {
  Serial.begin(9600);
  pinMode(SIGNAL_PIN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  if (digitalRead(SIGNAL_PIN) == HIGH) {
    digitalWrite(BLUE_LED, HIGH);
    Serial.println("high");
  }
  else {
    digitalWrite(BLUE_LED, LOW);
    Serial.println("low");
  }
  delay(1000);
}
