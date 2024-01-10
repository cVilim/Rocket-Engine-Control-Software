const int pressurePin = A0;
const int baudRate = 19200;
const int sensorReadDelay = 3;
const int pressureZero = 211;
const int pressureMax = 1023;
const int pressureBar= 350;
float pressureValue = 0;

void setup(){
  Serial.begin(baudRate);
}

void loop(){
  pressureValue = analogRead(pressurePin);
  pressureValue = ((pressureValue-pressureZero)*pressureBar)/(pressureMax-pressureZero);
  Serial.println(pressureValue, 2);
  delay(sensorReadDelay);
}
