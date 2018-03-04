#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RST_PIN 22 // Reset pin
#define SS_PIN 21 // Slave select pin
#define RELAY_PIN 4// Relay pin to control the plug

//constants
char* ssid = "MIBAR";
char* password =  "mibar2018";
const String pluckerID = "pluckers01";
const String apiEndpoint = "http://193.136.128.109:5006/api/";

String content;
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

/*We have 7 sates: 0 iddle wait for input card
                   1 send data to server of card and validate it
                   2 unlock relay and save card number
                   3 send data of consumption to server
                   4 verify card state
                   5 end- clear variables*/
int stateNode = 0;
String tagId;//variable that saves the current pluged device ID
unsigned long int powerConsumed = 0;
int actualCurrent = 1;

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

//function handles the authentication of validation in the server and also closes the connection in the second time it is called
void authenticationPost(String ID){
  if(WiFi.status() == WL_CONNECTED){
    //HTTP client
    HTTPClient http;
    http.begin(apiEndpoint+"session/"+pluckerID+"/"); //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); //Specify content-type header
    int httpResponseCode = http.POST("{\"tag\":\"" + ID + "\"}"); //Send the actual POST request

    if(httpResponseCode > 0){
      String response = http.getString();                       //Get the response to the request

      //Serial.println("POST Response:");   //Print return code
      //Serial.println(response);           //Print request answer

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
      stateNode = 0;//Return to IDLE
 
    }    
   http.end();  //Free resources
  }else{
    Serial.println("NO WIFI");
    stateNode = 0;//Return to IDLE
  }
}

void measureCurrent(){
  return;
}

void calculatePower(){
  powerConsumed = powerConsumed + (230 * actualCurrent);
}

void sendData(){
  if(WiFi.status() == WL_CONNECTED){
    //HTTP client
    HTTPClient http;
    http.begin(apiEndpoint + "consume/"+pluckerID+"/"); //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); //Specify content-type header
    int httpResponseCode = http.POST("{\"value\":\"" + String(powerConsumed) + "\"}"); //Send the actual POST request

    if(httpResponseCode > 0){
      String response = http.getString();                       //Get the response to the request

      //Serial.println("POST Response:");   //Print return code
      //Serial.println(response);           //Print request answeer
    }else{
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
   http.end();  //Free resources
  }else{
    Serial.println("NO WIFI");
  }
}

//function handles end state
void end(){
  Serial.println("END STATE");
  tagId = "";
  stateNode = 0;
  content = "";
  powerConsumed = 0;
  lockRelay();
  authenticationPost(tagId);//first time called it opens session second time it closes it
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

int i = 0;
unsigned lastTime;//variable for check the lastTime we seee the card - this detects if plug not present
void loop() {
  // put your main code here, to run repeatedly:
  switch(stateNode) {

    case 0://idle
      iddle();
      break; /* optional */
	
    case 1://send data to autentication
      authenticationPost(content);
      break; /* optional */
    
    case 2://unlock relay and save the id of the tag 
      Serial.println("unlockRelay");
      unlockRelay();
      tagId = content;
      stateNode = 3;
      break;
    case 3://send data and goes to the verify that tag is pluged
      Serial.println("measureAndPrintData");
      measureCurrent();
      calculatePower();
      sendData();
      stateNode = 4;
      break;
    case 4://verify if it dont pass 10 seconds since last time the card has been detected
      if(i == 0){
              Serial.println("First time in state 4");
              i = 1; //signals that we have passed the atuthentication
              lastTime = micros();
      }else{

        if( (micros()-lastTime) > 5000000){
          Serial.println("More than sec has passed...we need to end");
          //this means that more than 10sec has passed since last time we see a valid rfid so we go to state end
          stateNode = 5;
        }else{
          //this means that we are good so we measure more
          stateNode = 4;
          //check if the rfid is present if yes update lastTime saw
          if (mfrc522.PICC_IsNewCardPresent()){
            if (mfrc522.PICC_ReadCardSerial()){
              content= "";
              for (byte i = 0; i < mfrc522.uid.size; i++){

                content.concat(String(mfrc522.uid.uidByte[i], HEX));
              }
              if(content == tagId){//if tagg is the same
                lastTime = micros();
              }
            }
          }
        }
      }
      break;
    case 5://end comunitcations
      end();
      break;
    default : /* by default an error occurred so it ends and clear all */
      end();
  }
}
