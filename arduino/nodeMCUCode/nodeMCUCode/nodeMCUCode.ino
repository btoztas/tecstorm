#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>

#define RST_PIN 22 // Reset pin
#define SS_PIN 21 // Slave select pin
#define RELAY_PIN 4// Relay pin to control the plug

char* ssid = "OnePlus2";
char* password =  "candeias";

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

/*We have 7 sates: 0 iddle wait for input card
                   1 send data to server
                   2 wait server response
                   3 unlock relay and save card number
                   4 send data of consumption to server
                   5 verify card state
                   6 end- clear variables*/
int stateNode = 0;

//variables used
String content;


//in state 0 the machine is in iddle and waits for the card
void iddle(){
  while(true){
    Serial.println("Waiting input");
    if (mfrc522.PICC_IsNewCardPresent()){

      Serial.println("detetado cartão");
      if (mfrc522.PICC_ReadCardSerial()){
        Serial.println("li cartão");
        content= "";
        for (byte i = 0; i < mfrc522.uid.size; i++){

          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        stateNode = 1;//go to the new state
        return;
      }
    }
  }
}

void sendServerData(){

}

void end(){
  Serial.println("END STATE");
}

//RELAY CODE
void lockRelay(){
  digitalWrite(RELAY_PIN,LOW);
}
void unlockRelay(){
  digitalWrite(RELAY_PIN,HIGH);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial); // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  //connect to Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //config relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(stateNode) {

    case 0://idle
      iddle();
      break; /* optional */
	
    case 1:
      Serial.println(content);
      stateNode = 0;
      break; /* optional */
  
    default : /* by default an error occurred so it ends and clear all */
      end();
  }
  
  delay(500);
}
