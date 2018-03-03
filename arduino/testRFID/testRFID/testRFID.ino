#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22 // Reset pin
#define SS_PIN 21 // Slave select pin
 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
int flag = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial); // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

}

void loop() {
  // put your main code here, to run repeatedly:
  if (mfrc522.PICC_IsNewCardPresent()){
    Serial.println("detetado cartão");
    flag=1;
  }
  if (mfrc522.PICC_ReadCardSerial()){
    Serial.println("li cartão");
    flag=1;
  }
  if(flag == 0){
    Serial.println("Estou à espera");
  }
  if(flag){
    String content= "";
    for (byte i = 0; i < mfrc522.uid.size; i++){
      //content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println(content);
    flag = 0;
  }

}
