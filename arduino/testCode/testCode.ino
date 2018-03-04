
#define currentMeter 36

int currentValue;

void setup() {
  // put your setup code here, to run once:
  pinMode(currentMeter, INPUT);
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup OK");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  currentValue = analogRead(currentMeter);
  Serial.println(currentValue);
}
