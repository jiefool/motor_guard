#include <GSM.h>
#define PINNUMBER ""


GSM gsmAccess;
GSM_SMS sms;
String defaultPin = "3421";
String promoNumber;
String adminNumber = "+639472837607";
char senderNumber[20];
int lockUnlockPin = 4;

void setup() {  
  pinMode(lockUnlockPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("initializing GSM...");
  checkGSM();
  Serial.println("GSM initialized.");
  Serial.println("Waiting for messages...");
  
}

void loop() {  
  if (sms.available()) {    
    getSenderNumber();
    disposeOldMessage();
    String textMessage = readMessage();    
    Serial.println(textMessage);
    parseMessage(textMessage);
    sms.flush();   
  }
  
  if (Serial.available()>0){
    String serialInput = Serial.readString();
    parseMessage(serialInput);
  }
  delay(1000);
}

void disposeOldMessage(){
  if (sms.peek() == '#') {
      Serial.println("Discarded SMS");
      sms.flush();
  }
}

void checkGSM(){   
  boolean notConnected = true;
  
  while (notConnected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
}

String readMessage(){
    char c; 
    String textMessage; 
    while (c = sms.read()) {
      textMessage.concat(c);
    }
    return textMessage;
}

void getSenderNumber(){
  sms.remoteNumber(senderNumber, 20);
}

void parseMessage(String textMessage){
  String command;
  String pin; 
  if (textMessage.length()<=20){ 
    command = textMessage.substring(0,5);
    pin = textMessage.substring(5,9);
    Serial.println("Command: "+command);
    Serial.println("PIN: "+pin);
    if (pin == defaultPin){
      if (command == "olock"){      
        lockMotorcycle();
      }else if (command == "ulock"){
        unlockMotorcycle();
      }else if (command == "chpin"){
        char adminNumberVal[20];
        adminNumber.toCharArray(adminNumberVal, 20);
        if (senderNumber == adminNumberVal){
          defaultPin = textMessage.substring(9,13);
          sendMessage("info", "PIN changed.", senderNumber);
        }else{
          sendMessage("info", "You're not authorize.", senderNumber);
        }
      }else if (command == "adnch"){
        char adminNumberVal[20];
        adminNumber.toCharArray(adminNumberVal, 20);
        if (senderNumber == adminNumberVal){          
          sendMessage("info", "Admin number changed.", senderNumber);
          sendMessage("info", "You're the new Admin.", adminNumber);
        }else{
          sendMessage("info", "You're not authorize.", senderNumber);
        }
      }else if (command == "prreg"){
        String promoNumber = textMessage.substring(9,13);
        String promoRegText = textMessage.substring(13);      
        sendMessage("action", promoRegText, promoNumber);
      }else{
        sendMessage("info","Command not found.", senderNumber);
      }    
    }else{     
       sendMessage("info", "Invalid PIN.", senderNumber);
    }
  }
}

void lockMotorcycle(){
  sendMessage("info", "Motorcycle is now locked.", senderNumber);
  digitalWrite(lockUnlockPin, HIGH);
}

void unlockMotorcycle(){
  sendMessage("info", "Motorcycle is now unlocked.", senderNumber);
  digitalWrite(lockUnlockPin, LOW);
}

void sendMessage(String type, String message, String recipient){
  String textToSend;
  if (type == "info"){
    textToSend = "Motor Guard\nInfo: ";  
    textToSend.concat(message);      
  }else if (type == "action"){    
    textToSend = message;
  }else{
    Serial.println("Invalid type.");
  }
  
  Serial.println("Message: "+textToSend);
  Serial.println("Sent to: "+recipient);
  
  char recipientChar[20];
  recipient.toCharArray(recipientChar, 20);
  sms.beginSMS(recipientChar);
  sms.print(textToSend);
  sms.endSMS(); 
  
}
