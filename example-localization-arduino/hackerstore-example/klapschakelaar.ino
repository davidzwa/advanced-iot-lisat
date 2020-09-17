// Klapschakelaar
// (C) 2015 Hackerstore

const int PEN_LED =  13;        // LED-pen (of pen met relais)
const int PEN_MICROFOON = A0;   // Analoge pen waar digitale uitgang van microfoonmodule aan zit

const int SAMPLE_LENGTE = 50; // Duur in ms van sample-tijd
unsigned int geluid;

void setup() {
 pinMode(PEN_LED, OUTPUT); 
 pinMode(PEN_MICROFOON, INPUT); 
 Serial.begin(9600);
}

void loop() {
 unsigned long startMillis= millis(); // Begin van de sample-tijd
 unsigned int peakToPeak = 0; // top-top-niveau

 unsigned int Max = 0;
 unsigned int Min = 1024;

 // Verzamel gegevens voor gedurende SAMPLE_LENGTE (50) mS
 while (millis() - startMillis < SAMPLE_LENGTE)
 {
  geluid = analogRead(0);
  if (geluid < 1024) 
  {
   if (geluid > Max)
    Max = geluid;
   else if (geluid < Min)
    Min = geluid;
  }
 }
 peakToPeak = (Max - Min); // max - min = peak-peak amplitude

 if(peakToPeak>2) {
  // Klap-signaal ontvangen: zet de LED even aan
  digitalWrite(PEN_LED,HIGH);
  delay(300);  
 }
 
 digitalWrite(PEN_LED,LOW);
}

