#include <Wire.h>                   //for lcd
#include <RTClib.h>                 //for RTC DS1307    
#include <SoftwareSerial.h>         //for GSM Module
#include <MFRC522.h>                //for RFID module
#include <LiquidCrystal_I2C.h>      //for LCD
#include <Keypad.h>         
#include <SPI.h>

SoftwareSerial GSMSerial(14, 15);
int indoorDoorOpenButtonPin   = 29;
int outdoorDoorCloseButtonPin  = 28;
int alternateModeButtonPin    = 30;
MFRC522 mfrc522(53, 5);            
int microwaveSensor     = 36;
int pir1Pin         =   37;
int pir2Pin         =   39;
int pir3Pin         =   38;
float smokeThrValue1    =   400;	//two different kinds of MQ2 sensors
float smokeThrValue2    =   400;	//so slightly different values
float LPGThrValue       =   400;
int smoke1Pin          =   A8;
int smoke2Pin          =   A9;
int smoke3Pin          =   A10;
int LPG1Pin           =   A11;
int roomNumber0LightingPin  =   24;
int roomNumber1LightingPin  =   25;
int roomNumber2LightingPin  =   26;
int roomNumber3LightingPin  =   27;
int buzzerSmallPin      =   8;
int solenoidDoorLockPin   = 9;
int securityModeLEDPin    = 10;
int nightTimeLEDPin     =   12;
int dayTimeLEDPin       =   11;
int buzzerBigPin = 47;
String destinationNumber = "09443063353";  
String doorUnlockMethod, userName, roomName, roomName1, roomName2, roomName3, intruderAlertSMS, doorUnlockedSMS, triedDoorUnlockSMS, fireAlertSMS, gasAlarmSMS, lastDoorUnlockedUserName, lastDoorUnlockedMethod, lastDoorUnlockedHour, lastDoorUnlockedMinute, lastDoorUnlockedSecond, doorUnlockedMethod  ;
int roomNumber, currentSec, currentHour, currentMin;
int user0ID=00;
String user0Name="U Hla";
char user0password[4] = {'1', '2', '3', '4'}; 
String user0tagID = "19 61 BE 2B";
int user1ID=01;
String user1Name="Daw Mya";
char user1password[4] = {'1', '3', '4', '6'};
String user1tagID = "F2 64 50 2E";
int user2ID=02;
String user2Name="Mg Mg";
char user2password[4] = {'4', '6', '7', '9'};
String user2tagID = "43 B2 24 2E";
int user3ID=03;
String user3Name="Su Su";
char user3password[4] = {'3', '4', '5', '6'};
String user3tagID = "43 68 21 2E";
char alarmDisableCode[4] = {'0', '0', '0', '0'};
char doorLockCode[4] = {'1','1','1','1'};
int buttonState = 0;
int prevButtonState = HIGH;
int prevButtonState2 = HIGH;
int prevButtonState3 = HIGH;
long lastDebounceTime = 0;
long debounceDelay = 50;
int calibrationTime = 180;  
LiquidCrystal_I2C lcd(0x27, 20, 4);
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char nameOfTheMonth[13][12] = {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
RTC_DS1307 rtc;
char password[4]; 
char key_pressed = 0; 
uint8_t i = 0;  
const byte rows = 4;
const byte columns = 4;
char hexaKeys[rows][columns] = {
  {'D', 'C', 'A', 'B'},
  {'#', '9', '3', '6'},
  {'0', '8', '2', '5'},
  {'*', '7', '1', '4'}};
byte row_pins[rows] = {A4, A5, A6, A7};
byte column_pins[columns] = {A0, A1, A2, A3};
Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);
int lastDoorUnlocked = 0;
volatile bool nightTime     =  false;
volatile bool securityMode  =  false;
volatile bool displayMode   =   true;
volatile bool doorUnlockState = false;
volatile bool alternateMode       =     false;
volatile bool fireAlarmMode       =     false;
volatile bool fireAlarmMode1       =     false;
volatile bool fireAlarmMode2       =     false;
volatile bool fireAlarmMode3       =     false;
volatile bool gasAlarmMode        =     false;
volatile bool intruderAlarmMode   =     false;
const unsigned long interval05    =   500; 
const unsigned long interval1s    =   1000; 
const unsigned long interval5s    =   5000; 
const unsigned long interval10s   =   10000; 

void setup(){
  Wire.begin();     		
  Serial.begin(9600);  	
  rtc.begin();
  lcd.init();  
  lcd.backlight();
  SPI.begin(); 
  mfrc522.PCD_Init(); 
  GSMSerial.begin(9600);
  pinMode(indoorDoorOpenButtonPin, INPUT_PULLUP);
  digitalWrite(indoorDoorOpenButtonPin, HIGH);
  pinMode(outdoorDoorCloseButtonPin, INPUT_PULLUP);
  digitalWrite(outdoorDoorCloseButtonPin, HIGH);
  pinMode(alternateModeButtonPin, INPUT_PULLUP);
  digitalWrite(alternateModeButtonPin, HIGH);
  pinMode(solenoidDoorLockPin, OUTPUT);
  pinMode(pir1Pin, INPUT);
  pinMode(pir2Pin, INPUT);
  pinMode(pir3Pin, INPUT);
  pinMode(microwaveSensor, INPUT);
  pinMode(roomNumber0LightingPin, OUTPUT);
  pinMode(roomNumber1LightingPin, OUTPUT);
  pinMode(roomNumber2LightingPin, OUTPUT);
  pinMode(buzzerSmallPin, OUTPUT);
  pinMode(buzzerBigPin, OUTPUT);
  pinMode(nightTimeLEDPin, OUTPUT);
  pinMode(securityModeLEDPin, OUTPUT);
  pinMode(dayTimeLEDPin, OUTPUT);
  pinMode(smoke1Pin, INPUT);
  pinMode(smoke2Pin, INPUT);
  pinMode(smoke3Pin, INPUT);
  pinMode(LPG1Pin, INPUT);
  lcd.setCursor(0,0);
  lcd.print("Calibrating Sensor ");
  for(int i = 0; i < calibrationTime; i++){
    delay(1000);  }
  lcd.clear();
  checkNightTime();
  checkMotion();
  digitalWrite(securityModeLEDPin, LOW);}

void loop(){
  checkRFIDtag();
  checkPassword();
  checkButtons();
  checkNightTime();  
  checkPIRSensors();
  checkFireAndGasSensors(); 
  if (displayMode == true){ 
    displayIntroOnLCD();  }}

void checkRFIDtag(){
  if ( ! mfrc522.PICC_IsNewCardPresent()){	// Look for new cards  
    return;}
  if ( ! mfrc522.PICC_ReadCardSerial()) {	// Select one of the cards
    return;  }
  String tag = "";				 //Reading from the card
  for (byte j = 0; j < mfrc522.uid.size; j++) {
    tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
    tag.concat(String(mfrc522.uid.uidByte[j], HEX));  }
  tag.toUpperCase();
  if (tag.substring(1) == user0tagID)	{	//Checking the card
    unlockTheDoor();
    doorUnlockState=true;
    securityMode=false;
    digitalWrite(securityModeLEDPin, LOW);
    displayInfoAfterUnlock("RFID", user0Name);
    sendDoorUnlockedSMS("RFID",user0Name);}
  else if (tag.substring(1) == user1tagID){
    unlockTheDoor();
    doorUnlockState=true;
    securityMode=false;
    digitalWrite(securityModeLEDPin, LOW);
    displayInfoAfterUnlock("RFID", user1Name);
    sendDoorUnlockedSMS("RFID",user1Name); }
  else if (tag.substring(1) == user2tagID){
    unlockTheDoor();
    doorUnlockState=true;
    securityMode=false;
    digitalWrite(securityModeLEDPin, LOW);
    displayInfoAfterUnlock("RFID", user2Name);
    sendDoorUnlockedSMS("RFID",user2Name);  }
  else if (tag.substring(1) == user3tagID) {
    unlockTheDoor();
    doorUnlockState=true;
    securityMode=false;
    digitalWrite(securityModeLEDPin, LOW);
    displayInfoAfterUnlock("RFID", user3Name);
    sendDoorUnlockedSMS("RFID",user3Name);        }
  else   {   // If password is not matched
    digitalWrite(buzzerSmallPin, HIGH);
    displayMode=false;
    lcd.clear();
    unsigned long wrongRFIDMillis = millis();
    while(millis() - wrongRFIDMillis < interval5s){ 
      lcd.setCursor(0,0);
      lcd.print("Wrong RFID tag");
      lcd.setCursor(0,1);
      lcd.print("Alert SMS sent.");
      lcd.setCursor(0,2);
      lcd.print("Alarm ringing.");
      soundTriedDoorUnlockAlarm(); }
    i = 0;
    sendTriedDoorUnlockSMS("RFID");
    digitalWrite(buzzerSmallPin, LOW);
    lcd.clear();
    displayMode=true;
    displayIntroOnLCD(); }}
void checkPassword(){
  key_pressed = keypad_key.getKey(); // Storing keys
  if (key_pressed)  {       
    password[i++] = key_pressed; // Storing in password variable
    displayMode = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Enter Password:");
    lcd.setCursor(0,1);
    lcd.print("");
    lcd.setCursor(0,2);
    lcd.print("");
    lcd.setCursor(0,3);
    lcd.print("");  }
  if (i == 4)   {
    if (!(strncmp(password, user0password, 4))) {
      unlockTheDoor();
      doorUnlockState=true;
      securityMode=false;
      digitalWrite(securityModeLEDPin, LOW);
      displayInfoAfterUnlock("Password", user0Name);
      sendDoorUnlockedSMS("Password",user0Name);    
      i=0;  }
    else if (!(strncmp(password, user1password, 4)))  {
      unlockTheDoor();
      doorUnlockState=true;
      securityMode=false;
      digitalWrite(securityModeLEDPin, LOW);
      displayInfoAfterUnlock("Password", user1Name);
      sendDoorUnlockedSMS("Password",user1Name);    
      i=0;   }
    else if (!(strncmp(password, user2password, 4)))   {
      unlockTheDoor();
      doorUnlockState=true;
      securityMode=false;
      digitalWrite(securityModeLEDPin, LOW);
      displayInfoAfterUnlock("Password", user2Name);
      sendDoorUnlockedSMS("Password",user2Name);    
      i=0; }
    else if (!(strncmp(password, user3password, 4)))  {
      unlockTheDoor();
      doorUnlockState=true;
      securityMode=false;
      digitalWrite(securityModeLEDPin, LOW);
      displayInfoAfterUnlock("Password", user3Name);
      sendDoorUnlockedSMS("Password",user3Name);    
      i=0; }
    else if (!(strncmp(password, alarmDisableCode, 4)))     {
      digitalWrite(buzzerSmallPin, LOW);
      noTone(buzzerSmallPin);
      fireAlarmMode1 = false;
      fireAlarmMode2 = false;
      fireAlarmMode3 = false;
      gasAlarmMode  = false;
      intruderAlarmMode = false;
      lcd.setCursor(0, 0);
      lcd.print("Alarm disabled");
      lcd.setCursor(0, 1);
      lcd.print("by the user.");
      delay (2000);
      displayMode =true;
      displayIntroOnLCD();
      i=0; }
    else if (!(strncmp(password, doorLockCode, 4))){
      if (securityMode == false && doorUnlockState == true)   { 
        lockTheDoor();
        displayMode = false;
        lcd.clear();
        unsigned long displayInfoAfterLockMillis = millis();
        while(millis() - displayInfoAfterLockMillis < interval5s) { 
          lcd.setCursor(0, 0);
          lcd.print("Door is locked.");
          lcd.setCursor(0, 1);
          lcd.print("Security Mode is OFF");    }
      i=0;
      lcd.clear();
      displayMode=true;  
      displayIntroOnLCD();}
      else{
        displayMode = false;
        lcd.clear();
        unsigned long displayInfoAfterLockMillis = millis();
        while(millis() - displayInfoAfterLockMillis < interval5s)    { 
          lcd.setCursor(0, 0);
          lcd.print("The code works only");
          lcd.setCursor(0, 1);
          lcd.print("when door is OPEN");   }
      i=0;
      lcd.clear();
      displayMode=true;  
      displayIntroOnLCD();   }  }
    else       { 
      digitalWrite(buzzerSmallPin, HIGH);
      displayMode = false;
      lcd.clear();
      unsigned long wrongPasswordmillis = millis();
      while(millis() - wrongPasswordmillis < interval5s)  { 
        lcd.setCursor(0,0);
        lcd.print("Incorrect Password Entry");
        lcd.setCursor(0,1);
        lcd.print("Alert SMS sent.");
        lcd.setCursor(0,2);
        lcd.print("Alarm ringing.");
        soundTriedDoorUnlockAlarm();   }
      digitalWrite(buzzerSmallPin, LOW);
      sendTriedDoorUnlockSMS("Password");
      i = 0;
      lcd.clear();
      displayMode=true;
      displayIntroOnLCD(); }  }  }      
void displayIntroOnLCD(){
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);   
  lcd.setCursor(0, 0);
  lcd.print("Home Security System");
  lcd.setCursor(0, 1);
  lcd.print("by Mg Nay Myo Htet");
  lcd.setCursor(0,2);
  if (currentHour.toInt() <10) {
    lcd.print("0");
    lcd.print(now.hour(), DEC);    }
  else {
    lcd.print(now.hour(), DEC);    }  
  lcd.print(":");
  if (currentMin.toInt() <10) {
    lcd.print("0");
    lcd.print(now.minute(), DEC);    }
  else {
    lcd.print(now.minute(), DEC);    }
  lcd.print(":");
  if (currentSec.toInt() <10)  {
    lcd.print("0");
    lcd.print(now.second(), DEC);     }
  else  {
    lcd.print(now.second(), DEC);    }
  lcd.print("  ");
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" ");
  lcd.setCursor(0,3);
  lcd.print(nameOfTheMonth[now.month()]);
  lcd.print(" ");
  lcd.print(now.day(), DEC);
  lcd.print(", ");
  lcd.print(now.year(), DEC);}
void sendTriedDoorUnlockSMS(String doorUnlockedMethod){
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);  
  String timeDivider=":";
  String strPart1="Unauthorized Attempt to Unlock at ";
  String strPart2=strPart1+currentHour;
  String strPart3=strPart2+timeDivider;
  String strPart4=strPart3+currentMin;
  String strPart5=strPart4+timeDivider;
  String strPart6=strPart5+currentSec;
  String strPart7=strPart6+" with ";
  String strPart8=strPart7+doorUnlockedMethod;
  String triedDoorUnlockSMS=strPart8+".";
  GSMSerial.println("AT+CMGF=1");                             
  delay(1000);                                                 
  GSMSerial.println("AT+CMGS=\"" + destinationNumber +"\"");  
  delay(1000);
  GSMSerial.println(triedDoorUnlockSMS);                      
  delay(1000);
  GSMSerial.println((char)26);                                
  }
void sendDoorUnlockedSMS(String doorUnlockedMethod, String userName){ 
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);   
  String timeDivider=":";
  String strPart1="Door Unlocked at ";
  String strPart2=strPart1+currentHour;
  String strPart3=strPart2+timeDivider;
  String strPart4=strPart3+currentMin;
  String strPart5=strPart4+timeDivider;
  String strPart6=strPart5+currentSec;
  String strPart7=strPart6+" with ";
  String strPart8=strPart7+doorUnlockedMethod;
  String strPart9=strPart8+" by ";
  String strPart10=strPart9+userName;
  String doorUnlockedSMS=strPart10+".";
  GSMSerial.println("AT+CMGF=1");                              //Sets the GSM Module in Text Mode
  delay(1000);                                                 // Delay of 1000 milli seconds or 1 second
  GSMSerial.println("AT+CMGS=\"" + destinationNumber +"\"");    // Replace x with mobile number
  delay(1000);
  GSMSerial.println(doorUnlockedSMS);                           // The SMS text you want to send
  delay(1000);
  GSMSerial.println((char)26);                                  // ASCII code of CTRL+Z
void soundTriedDoorUnlockAlarm(){ 
  unsigned long soundTriedDoorUnlockAlarmMillis = millis();
  while(millis() - soundTriedDoorUnlockAlarmMillis < interval5s)  { 
	digitalWrite(buzzerSmallPin, HIGH);   }
  digitalWrite(buzzerSmallPin, LOW);}
void soundIntruderAlarm(String roomName){  
  if (intruderAlarmMode == true)  {   
  unsigned long soundIntruderAlarmMillis = millis();
  while(millis() - soundIntruderAlarmMillis < interval5s)  { 
    tone(buzzerSmallPin,440,200);
    checkPassword();    }   }
  else  (intruderAlarmMode == false && fireAlarmMode == false && gasAlarmMode == false)  {
    noTone(buzzerSmallPin);  }}
void checkButtons(){
  buttonState = digitalRead(indoorDoorOpenButtonPin);     //indoor dooropen button
  if ((buttonState != prevButtonState) && (buttonState == HIGH))   {
    if (securityMode == false && doorUnlockState==false)    {
      doorUnlockState=true;
      unlockTheDoor();    }
    else if (securityMode == false && doorUnlockState == true)    {
      doorUnlockState=false;
      lockTheDoor();    }
    else (securityMode == true)    {
      sendIntruderAlertSMS("Living Room");
      soundIntruderAlarm("Living Room");
      intruderAlarmMode = true;
      displayIntruderAlert("Living Room");    }}
  prevButtonState = buttonState;
  buttonState = digitalRead(outdoorDoorCloseButtonPin);    
  if ((buttonState != prevButtonState2) && (buttonState == HIGH))   {   
    if (doorUnlockState == true && securityMode ==  false)    {
      lockTheDoor();
      checkMotion();
      displayInfoAfterLock();    }  
    else  (doorUnlockState == false)    {
      soundDoorBell();    }  }
  prevButtonState2 = buttonState;
  buttonState = digitalRead(alternateModeButtonPin);    //indoor doorclose button
  if ((buttonState != prevButtonState3) && (buttonState == HIGH))   {
    if (alternateMode == false)     {
      alternateMode = true;      }
    else    {
      alternateMode = false;    }}
  prevButtonState3 = buttonState;}
void turnOnLights(int roomNumber){
  if (roomNumber==0){
    digitalWrite(roomNumber0LightingPin, HIGH);  	}
  if (roomNumber==1)  {
    digitalWrite(roomNumber1LightingPin, HIGH);   }
  if (roomNumber==2)  {
    digitalWrite(roomNumber2LightingPin, HIGH);   }
  if (roomNumber==3)  {
    digitalWrite(roomNumber3LightingPin, HIGH);   }}
void turnOffLights(int roomNumber){
  if (roomNumber==0)  {
    digitalWrite(roomNumber0LightingPin, LOW);    }
  if (roomNumber==1)  {
    digitalWrite(roomNumber1LightingPin, LOW);    }
  if (roomNumber==2)  {
    digitalWrite(roomNumber2LightingPin, LOW);   }
  if (roomNumber==3)  {
    digitalWrite(roomNumber3LightingPin, LOW);   }}
void checkMotion(){ 
  if (digitalRead(pir1Pin) == HIGH && digitalRead(microwaveSensor) == HIGH)  {
    securityMode = false;
    digitalWrite(securityModeLEDPin, LOW);
  else if (digitalRead(pir2Pin) == HIGH && digitalRead(microwaveSensor) == HIGH)  {
    securityMode = false;
    digitalWrite(securityModeLEDPin, LOW);
  else if (digitalRead(pir3Pin) == HIGH && digitalRead(microwaveSensor) == HIGH)  {
    securityMode = false;
    digitalWrite(securityModeLEDPin, LOW);
  else  (digitalRead(pir1Pin) == LOW && digitalRead(pir2Pin) == LOW && digitalRead(pir3Pin)  == LOW)  {
    securityMode = true;
    digitalWrite(securityModeLEDPin, HIGH);}
void unlockTheDoor(){ 
  digitalWrite(buzzerSmallPin, HIGH);
  delay(10);
  digitalWrite(buzzerSmallPin, LOW);
  doorUnlockState = true;
  digitalWrite(solenoidDoorLockPin, HIGH);   }
void lockTheDoor(){ 
  doorUnlockState = false;
  digitalWrite(solenoidDoorLockPin, LOW);    }
void sendIntruderAlertSMS(String roomName){ 
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);  
  String timeDivider=":";
  String strPart1="Motion detected at ";
  String strPart2=strPart1+currentHour;
  String strPart3=strPart2+timeDivider;
  String strPart4=strPart3+currentMin;
  String strPart5=strPart4+timeDivider;
  String strPart6=strPart5+currentSec;
  String strPart7=strPart6+" at ";
  String strPart8=strPart7+roomName;
  String intruderAlertSMS=strPart8+".";  
  GSMSerial.println("AT+CMGF=1");                             
  delay(1000);                                                 
  GSMSerial.println("AT+CMGS=\"" + destinationNumber +"\"");  
  delay(1000);
  GSMSerial.println(intruderAlertSMS);                        
  delay(1000);
  GSMSerial.println((char)26);                                
void checkNightTime(){
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);  
  if (currentHour.toInt() >= 18 || currentHour.toInt() <= 5 || alternateMode  ==  true)  {
    nightTime = true;     
    digitalWrite(nightTimeLEDPin, HIGH);
    digitalWrite(dayTimeLEDPin, LOW);  }
  else   {
    nightTime = false;        
    digitalWrite(nightTimeLEDPin, LOW);
    digitalWrite(dayTimeLEDPin, HIGH);  }}
void displayInfoAfterUnlock(String doorUnlockedMethod, String userName)
{ DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);  
  digitalWrite(securityModeLEDPin, LOW);
  displayMode=false;
  lcd.clear();
  if (lastDoorUnlocked == 1) { 
    if (securityMode=true)    {
      unsigned long x = millis();
    while(millis() - x < interval5s)    { 
      lcd.setCursor(0, 0);
      lcd.print("Door Unlocked with");
      lcd.setCursor(0, 1);
      lcd.print(doorUnlockedMethod);
      lcd.print(" by ");
      lcd.print(userName);
      lcd.print(".");
      lcd.setCursor(0, 2);
      lcd.print("Security ==> OFF");
      lcd.setCursor(0,3);
      lcd.print("Door Unlocked.");
      checkButtons();    }
    lcd.clear();
    unsigned long y = millis();
    while(millis() - y < interval5s)   { 
      lcd.setCursor(0,0);
      lcd.print("Last Unlocked with");
      lcd.setCursor(0,1);
      lcd.print(lastDoorUnlockedMethod);
      lcd.print(" by ");
      lcd.print(lastDoorUnlockedUserName);
      lcd.setCursor(0,2);
      lcd.print("at ");
      lcd.print(lastDoorUnlockedHour);
      lcd.print(":");
      lcd.print(lastDoorUnlockedMinute);
      lcd.print(":");
      lcd.print(lastDoorUnlockedSecond);
      lcd.print(".");
      checkButtons();    }
    lastDoorUnlocked = 1;
    lastDoorUnlockedUserName =userName;
    lastDoorUnlockedMethod =doorUnlockedMethod;
    lastDoorUnlockedHour=currentHour;
    lastDoorUnlockedMinute=currentMin;
    lastDoorUnlockedSecond=currentSec;
    lcd.clear();
    displayMode=true;
    securityMode=false;
    digitalWrite(securityModeLEDPin, LOW);
    displayIntroOnLCD();    }
    else    {
      unsigned long k = millis();
    while(millis() - k < interval5s)    { 
      lcd.setCursor(0, 0);
      lcd.print("Door Unlocked with");
      lcd.setCursor(0, 1);
      lcd.print(doorUnlockedMethod);
      lcd.print(" by ");
      lcd.print(userName);
      lcd.print(".");
      lcd.setCursor(0, 2);
      lcd.print("Security ==> OFF");
      lcd.setCursor(0,3);
      lcd.print("Door Unlocked.");
      checkButtons();    }
    lcd.clear();
    unsigned long h = millis();
    while(millis() - h < interval5s)    { 
      lcd.setCursor(0,0);
      lcd.print("Last Unlocked with");
      lcd.setCursor(0,1);
      lcd.print(lastDoorUnlockedMethod);
      lcd.print(" by ");
      lcd.print(lastDoorUnlockedUserName);
      lcd.setCursor(0,2);
      lcd.print("at ");
      lcd.print(lastDoorUnlockedHour);
      lcd.print(":");
      lcd.print(lastDoorUnlockedMinute);
      lcd.print(":");
      lcd.print(lastDoorUnlockedSecond);
      lcd.print(".");
      checkButtons();  }
    lastDoorUnlocked = 1;
    lastDoorUnlockedUserName =userName;
    lastDoorUnlockedMethod =doorUnlockedMethod;
    lastDoorUnlockedHour=currentHour;
    lastDoorUnlockedMinute=currentMin;
    lastDoorUnlockedSecond=currentSec;
    lcd.clear();
    displayMode=true;
    securityMode=false;
    digitalWrite(securityModeLEDPin, LOW);
    displayIntroOnLCD(); }}
  else { 
    displayMode=false;
    lcd.clear();
    unsigned long x = millis();
    while(millis() - x < interval5s)
    {    lcd.setCursor(0, 0);
      lcd.print("Door Unlocked with");
      lcd.setCursor(0, 1);
      lcd.print(doorUnlockedMethod);
      lcd.print(" by ");
      lcd.print(userName);
      lcd.print(".");
      lcd.setCursor(0, 2);
      lcd.print("Security was OFF");
      lcd.setCursor(0,3);
      lcd.print("Door Unlocked.");
      checkButtons();    }
    lastDoorUnlocked          =   1;
    lastDoorUnlockedUserName  =   userName;
    lastDoorUnlockedMethod    =   doorUnlockedMethod;
    lastDoorUnlockedHour    =   currentHour;
    lastDoorUnlockedMinute  =   currentMin;
    lastDoorUnlockedSecond  =   currentSec;
    lcd.clear();
    displayMode   =   true;
    displayIntroOnLCD();  }}
void displayIntruderAlert(String roomName){ { 
    lcd.setCursor(0, 0);
    lcd.print("Motion detected at ");    
    lcd.setCursor(0, 1);
    lcd.print(roomName);
    lcd.print(".");
    lcd.setCursor(0, 2);
    lcd.println("Alert SMS is sent.");
    lcd.setCursor(0,3);
    lcd.print("Alarm ringing.");
    checkPassword();
    checkRFIDtag(); } }
void checkPIRSensors(){
  if (securityMode == true && nightTime == false)  {   
    turnOffLights(0);
    turnOffLights(1);
    turnOffLights(2);
    turnOffLights(3);
    if (digitalRead(pir1Pin) == HIGH)    {
      turnOnLights(1);
      soundIntruderAlarm("Living Room");
      intruderAlarmMode = true;
      sendIntruderAlertSMS("Living Room");
      displayIntruderAlert("Living Room");    }
    if (digitalRead(pir2Pin) == HIGH)    {
      turnOnLights(2);
      soundIntruderAlarm("Bedroom");
      intruderAlarmMode = true;
      sendIntruderAlertSMS("Bedroom");
      displayIntruderAlert("Bedroom");    }
    if (digitalRead(pir3Pin) == HIGH)    {
      turnOnLights(3);
      soundIntruderAlarm("Kitchen");
      intruderAlarmMode = true;
      sendIntruderAlertSMS("Kitchen");
      displayIntruderAlert("Kitchen");    }  }
  else if (securityMode ==  true && nightTime == true)  {
    turnOnLights(0);
    turnOffLights(1);
    turnOffLights(2);
    turnOffLights(3);
    if (digitalRead(pir1Pin) == HIGH)    {
      turnOnLights(1);
      displayIntruderAlert("Living Room");
      soundIntruderAlarm("Living Room");
      intruderAlarmMode = true;
      sendIntruderAlertSMS("Living Room");
      lcd.clear();
      displayMode=1;
      displayIntroOnLCD();    }
    if (digitalRead(pir2Pin) == HIGH)    {
      turnOnLights(2);
      displayIntruderAlert("Bedroom");
      soundIntruderAlarm("Bedroom");
      intruderAlarmMode = true;
      sendIntruderAlertSMS("Bedroom");
      lcd.clear();
      displayMode=1;
      displayIntroOnLCD();    }
    if (digitalRead(pir3Pin) == HIGH)    {
      turnOnLights(3);
      displayIntruderAlert("Kitchen");
      soundIntruderAlarm("Kitchen");
      intruderAlarmMode = true;
      sendIntruderAlertSMS("Kitchen");
      lcd.clear();
      displayMode=1;
      displayIntroOnLCD();    }  }
  else if (securityMode == false && nightTime== true)    {
    turnOnLights(0);
    if (digitalRead(pir1Pin) == HIGH && digitalRead(microwaveSensor)==HIGH)    {
      turnOnLights(1);    }
    else     {
      turnOffLights(1);    }
    if (digitalRead(pir2Pin) == HIGH && digitalRead(microwaveSensor)==HIGH)    {
      turnOnLights(2);    }
    else     {
      turnOffLights(2);    }
    if (digitalRead(pir3Pin) == HIGH && digitalRead(microwaveSensor)==HIGH)    {
      turnOnLights(3);    }    
    else     {
      turnOffLights(3);    }     }
  else  (securityMode == false && nightTime == false)  { 
    turnOffLights(0);
    turnOffLights(1);
    turnOffLights(2);
    turnOffLights(3); }}
void checkFireAndGasSensors(){
  if(analogRead(smoke1Pin) > smokeThrValue1 && fireAlarmMode1==false)  {
    int smoke1Value=analogRead(smoke1Pin);;
    fireAlarmMode1=true;
    digitalWrite(buzzerSmallPin, HIGH);    
    displayFireAlert("Living Room");
    sendFireAlertSMS("Living Room");
    checkButtons();
    checkPassword();  }
  if(analogRead(smoke2Pin) > smokeThrValue1 && fireAlarmMode2==false)  {
    int smoke2Value=analogRead(smoke2Pin);;
    fireAlarmMode2=true;
    digitalWrite(buzzerSmallPin, HIGH);    
    displayFireAlert("Bedroom");  
    sendFireAlertSMS("Bedroom");
    checkButtons();
    checkPassword();  }
  if(analogRead(smoke3Pin) > smokeThrValue2 && fireAlarmMode3==false)  {
    int smoke3Value=analogRead(smoke3Pin);;
    fireAlarmMode3=true;
    digitalWrite(buzzerSmallPin, HIGH);
    displayFireAlert("Kitchen");
    sendFireAlertSMS("Kitchen");
    checkButtons();
    checkPassword(); }
  if(analogRead(LPG1Pin) > LPGThrValue && gasAlarmMode==false)  {
    int lpgValue=analogRead(LPG1Pin);;
    gasAlarmMode=true;
    digitalWrite(buzzerSmallPin, HIGH);
    displayLPGAlert("Kitchen");
    sendLPGAlertSMS("Kitchen");
    checkButtons();
    checkPassword();  }}
void sendFireAlertSMS(String roomName){
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);  
  String timeDivider=":";
  String strPart1="Smoke detected at ";
  String strPart2=strPart1+currentHour;
  String strPart3=strPart2+timeDivider;
  String strPart4=strPart3+currentMin;
  String strPart5=strPart4+timeDivider;
  String strPart6=strPart5+currentSec;
  String strPart7=strPart6+" at ";
  String strPart8=strPart7+roomName;
  String fireAlertSMS=strPart8+". Call 191, 192.";
  GSMSerial.println("AT+CMGF=1");                            
  delay(1000);                                               
  GSMSerial.println("AT+CMGS=\"" + destinationNumber +"\""); 
  delay(1000);
  GSMSerial.println(fireAlertSMS);                           
  delay(1000);
  GSMSerial.println((char)26); }
void sendLPGAlertSMS(String roomName){
  DateTime now = rtc.now();
  String currentHour=String(now.hour(),DEC);
  String currentMin=String(now.minute(),DEC);
  String currentSec=String(now.second(),DEC);  
  String timeDivider=":";
  String strPart1="Gas Leakage detected at ";
  String strPart2=strPart1+currentHour;
  String strPart3=strPart2+timeDivider;
  String strPart4=strPart3+currentMin;
  String strPart5=strPart4+timeDivider;
  String strPart6=strPart5+currentSec;
  String strPart7=strPart6+" at ";
  String strPart8=strPart7+roomName;
  String LPGAlertSMS=strPart8+". Call 191, 192.";
  GSMSerial.println("AT+CMGF=1");                           
  delay(1000);                                              
  GSMSerial.println("AT+CMGS=\"" + destinationNumber +"\"");
  delay(1000);
  GSMSerial.println(LPGAlertSMS);                      
  delay(1000);
  GSMSerial.println((char)26);                              
void displayFireAlert(String roomName){
  displayMode = false;
  lcd.clear();  
    lcd.setCursor(0, 0);
    lcd.print("Smoke detected at");
    lcd.setCursor(0, 1);
    lcd.print(roomName);
    lcd.print(" .");      
    lcd.setCursor(0, 2);
    lcd.print("Alarm ringing.");
    lcd.setCursor(0,3);
    lcd.print("Phone: 09964294838");
    checkButtons();
    checkPassword(); }
void displayLPGAlert(String roomName){
  displayMode = false;
  lcd.clear();
  if (gasAlarmMode == true)  {
    lcd.setCursor(0, 0);
    lcd.print("Gas leakage detected");
    lcd.setCursor(0, 1);
    lcd.print("at ");  
    lcd.print(roomName);
    lcd.print(" .");      
    lcd.setCursor(0, 2);
    lcd.print("Alarm ringing.");
    lcd.setCursor(0,3);
    lcd.print("Phone: 09964294838");
    checkButtons();  
    checkPassword();}
  else   {
    displayMode = true;
    displayIntroOnLCD();  }}
void soundFireAlarm(String roomName){
  if (fireAlarmMode == true)  {
    tone(buzzerSmallPin, 1200, 250);
    tone(buzzerSmallPin,  8
00, 250);    }
  else if (fireAlarmMode == false && gasAlarmMode == false && intruderAlarmMode == false)  {
    noTone(buzzerSmallPin);  }}
void soundLPGAlarm(String roomName){
  if (gasAlarmMode == true)  {
    tone(buzzerSmallPin, 660, 700);
    checkButtons();
    checkPassword(); }
  else if (fireAlarmMode == false && gasAlarmMode == false && intruderAlarmMode == false)  {
    noTone(buzzerSmallPin);  }}
void soundDoorBell(){
  tone(buzzerSmallPin, 660);
  delay(700);
  tone(buzzerSmallPin, 550);
  delay(700);
  tone(buzzerSmallPin, 440);
  delay(700);
  noTone(buzzerSmallPin);}
void displayInfoAfterLock(){
  if (securityMode == true)  {
    displayMode = false;
    lcd.clear();
    unsigned long aa = millis();
    while(millis() - aa < interval5s)    { 
      lcd.setCursor(0, 0);
      lcd.print("Door is locked.");
      lcd.setCursor(0, 1);
      lcd.print("No motion detected.");  
      lcd.setCursor(0, 2);
      lcd.print("Security Mode => ON");    }
    lcd.clear();
    displayMode=true; 
    displayIntroOnLCD();  }
  else (securityMode == false)  {
    displayMode=false;
    lcd.clear();
    unsigned long bb = millis();
    while(millis() - bb < interval10s)    { 
      lcd.setCursor(0, 0);
      lcd.print("Door is locked.");
      lcd.setCursor(0, 1);
      lcd.print("Motions detected.");  
      lcd.setCursor(0, 2);
      lcd.print("Security Mode was OFF.");    }
    lcd.clear();
    displayMode=true;  
    displayIntroOnLCD();  }}
