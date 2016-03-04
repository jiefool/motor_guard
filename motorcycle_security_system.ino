#include <GSM.h>
#define PINNUMBER ""
#define irSensor A0
#define NB_SAMPLE 25

GSM gsmAccess;
GSM_SMS sms;
String defaultPin = "3421";
String promoNumber;
String adminNumber = "+639472837607";
char senderNumber[20];
int lockUnlockPin = 4;
int pressSensorPin = 5;
int buzzerOutputPin = A1;
int tiltInputPin = A2;
int irDetectCount = 0;

void setup() {  
  pinMode(tiltInputPin, INPUT);
  pinMode(pressSensorPin, INPUT);  
  pinMode(irSensor, INPUT);    
  pinMode(lockUnlockPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("initializing GSM...");
  checkGSM();
  Serial.println("GSM initialized.");
  Serial.println("Waiting for messages...");
  
}

int tiltValue = 0;
int tiltFinalValue = 0;
void loop() {  
  
  //sms detection  
  if (sms.available()) {    
    getSenderNumber();
    disposeOldMessage();
    String textMessage = readMessage();    
    Serial.println(textMessage);
    parseMessage(textMessage);
    sms.flush();   
  }
  
  //serial input detection
  if (Serial.available()>0){
    String serialInput = Serial.readString();
    parseMessage(serialInput);
  }
  
  //IR sensor detection
  if (checkIrSensor()){
    irDetectCount++;
  }else{
    irDetectCount = 0;
  }
  
  if (irDetectCount == 5){
    Serial.println("IR sensor triggered.");
    sendMessage("info", "IR sensor triggered.", adminNumber);
    ringBuzzer(); 
    irDetectCount = 0; 
  }
  
  Serial.println(irDetectCount);
  
  //press sensor detection
  if (digitalRead(pressSensorPin) == 1){
    Serial.println("Press sensor triggered.");
    sendMessage("info", "Press sensor is triggered.", adminNumber);    
    ringBuzzer();  
  } 
  
  //tilt sensor detection
  tiltValue = analogRead(tiltInputPin);
  tiltFinalValue = map(tiltValue, 0, 10, 0, 255);

  if (tiltFinalValue > 26000 ){
    Serial.println("Tilt sensor triggered.");    
    sendMessage("info", "Tilt sensor triggered.", adminNumber);
    ringBuzzer();    
  }      
}

void ringBuzzer(){
  for(int i=0;i<=10;i++){
    analogWrite(buzzerOutputPin, 255);
    delay(500);
    analogWrite(buzzerOutputPin, 0);
    delay(500);
  }
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
  if (textMessage.length()<=22){ 
    command = textMessage.substring(0,5);
    pin = textMessage.substring(5,9);
    Serial.println("Command: "+command);
    Serial.println("PIN: "+pin);
    if (pin == defaultPin){
      if (command == "olock"){      
        lockMotorcycle();
      }else if (command == "ulock"){
        unlockMotorcycle();
      }else if (command == "alarm"){
        sendMessage("info", "Buzzer ringed.", adminNumber);
        ringBuzzer();
      }else if (command == "chpin"){
        String senderNumberVal(senderNumber);               
        if (senderNumberVal == adminNumber){
          defaultPin = textMessage.substring(9,13);
          sendMessage("info", "PIN changed.", senderNumber);
        }else{
          sendMessage("info", "You're not authorized.", senderNumber);
        }
      }else if (command == "adnch"){
        String senderNumberVal(senderNumber);        
        if (senderNumberVal == adminNumber){       
          adminNumber = textMessage.substring(9);   
          sendMessage("info", "Admin number changed.", senderNumber);
          sendMessage("info", "You're the new Admin.", adminNumber);
        }else{
          sendMessage("info", "You're not authorized.", senderNumber);
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

int checkIrSensor(){
 int sum=0;
 for(int i=0; i<10;i++){
   int sensor=analogRead(irSensor);
   int dist= 3027.4/sensor;
   int distance= pow(dist,1.2134);
   sum=sum+distance;
 }
 
 int distance_cm=sum/10;  
 if(distance_cm >=45){
   return true;
 }else{
   return false;
 }  
}
