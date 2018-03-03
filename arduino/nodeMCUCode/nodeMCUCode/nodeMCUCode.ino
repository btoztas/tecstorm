#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RST_PIN 22 // Reset pin
#define SS_PIN 21 // Slave select pin
#define RELAY_PIN 4// Relay pin to control the plug

char* ssid = "Redmi";
char* password =  "9ecd440f1d8f";
String pluckerID = "pluckers01";
String TagID;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

/*We have 7 sates: 0 iddle wait for input card
                   1 send data to server of card and validate it
                   2 unlock relay and save card number
                   3 send data of consumption to server
                   4 verify card state
                   5 end- clear variables*/
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

bool authenticationPost(){
  if(WiFi.status() == WL_CONNECTED){
    //HTTP client
    HTTPClient http;
    http.begin("http://193.136.128.103:5005/api/session/"+pluckerID+"/"); //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); //Specify content-type header
    int httpResponseCode = http.POST("{\"tag\":\"" + content + "\"}"); //Send the actual POST request

    if(httpResponseCode > 0){
      String response = http.getString();                       //Get the response to the request

      Serial.println("POST Response:");   //Print return code
      Serial.println(response);           //Print request answer

      if(httpResponseCode == 200){
        //This means it's a valid ID
        Serial.println("Valid ID");
        stateNode = 2;
      }else{
        Serial.println("Invalid ID");
        stateNode = 0;//Return to IDLE
      }

    }else{
 
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
 
    }    
   http.end();  //Free resources
  }else{
    Serial.println("NO WIFI");
  }
}



void end(){
  Serial.println("END STATE");
  tagId = "";
  stateNode = 0;
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
	
    case 1://send data to autentication
      authenticationPost();
      break; /* optional */
    
    case 2://unlock relay and save the id of the tag 
      unlockRelay();
      tagId = content;
      stateNode = 3;
      break;
    case 3://send data and goes to the verify data


    default : /* by default an error occurred so it ends and clear all */
      end();
  }
  
  delay(500);
}
