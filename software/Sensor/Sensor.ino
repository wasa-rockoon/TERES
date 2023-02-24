#define LED_STATUS 0
#define LED_ERROR 25


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  
  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);

  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_ERROR, HIGH);
  
  delay(1000); 

  /*Wire1.beginTransmission(0x29);
  error1 = Wire1.endTransmission();
  Wire1.beginTransmission(0x76);
  error2 = Wire1.endTransmission();*/
  

  digitalWrite(LED_ERROR, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_STATUS, HIGH);
  delay(500);
  digitalWrite(LED_STATUS, LOW);
  delay(500);

  //i2c_scanner(Wire1);
}
