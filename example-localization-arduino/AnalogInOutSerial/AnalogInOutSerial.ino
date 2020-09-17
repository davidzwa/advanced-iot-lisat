
const int analogInPin = A0;
int sensorValue = 0;

void setup() {
  Serial.begin(2000000);
}

void loop() {
  sensorValue = analogRead(analogInPin);
  Serial.println(map(sensorValue, 0, 1023, 0, 255));
}
