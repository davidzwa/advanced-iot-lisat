String buffert;

void setup() {
  Serial.begin(115200);
  Serial.swap(); // Swap FROM TX/GPIO1, RX/GPIO3 to D8/TX/GPIO15 and D7/RX/GPIO13
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  Serial.println("infah-esp");
  delay(20);
  Serial.println("info-esp");
//  if (Serial.available()) {
//    digitalWrite(LED_BUILTIN, LOW);
//    buffert = Serial.readString();
//    Serial.println(buffert);
//  }
}
