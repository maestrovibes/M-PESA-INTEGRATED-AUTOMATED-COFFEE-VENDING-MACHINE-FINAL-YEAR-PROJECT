#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Key.h>
#include <Keypad.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#define myipaddress 192.168.0.105

const char *ssid = "FAiba4G-1083"; //Enter your WIFI ssid
const char *WIFIpassword = "RY524EB1B98"; //Enter your WIFI password
const byte ROWS = 4; 
const byte COLS = 3; 
char customKey;
char keypressed;
String number = "";
String amount;
String choice;
String p;
String m;
int c;
int Amt;
int amnt;
//initialize button pin 13
int buttonPin = 13;
int buttonState = 0;
int pumpState = 0;
const int pumpPin =  33;   // initialize digital pin 33 for pump control
unsigned long dispenseTime = 0;
unsigned long dispenseStart = 0;
unsigned long dispenseEnd = 0;
unsigned long dispenseDelay20 = 10000;
unsigned long dispenseDelay10 = 5000;
int delayOver;
String password = "1234"; // change your password here
String input_password;
String new_password1;
String new_password2;


char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};


byte rowPins[ROWS] = {19, 18, 5, 17}; 
byte colPins[COLS] = {16, 4, 15}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

int cursorColumn = 0;
int passColumn = 7;

WiFiClient client;
HTTPClient http;
void setup() {

  //Initiating the serial connection
  Serial.begin(115200);
  delay(3000);
  // initialize button pin  
  pinMode(buttonPin, INPUT);

  // initialize the pump pin as an output:
  pinMode(pumpPin, OUTPUT);

  // initialize the lcd
  lcd.init(); 
  lcd.backlight();
  

  WiFi.begin(ssid, WIFIpassword);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("...");
  }
  lcd.setCursor(0,1);
  lcd.print("WiFi connected");
  delay(1000);
}
void loop() {

  lcd.clear();
  lcd.setCursor(0,0);  
  lcd.print("Welcome!!!");
  lcd.setCursor(0,1);
  lcd.print("Press green butn");  
  
  keypressed = customKeypad.getKey();   //Constantly waiting for a key to be pressed
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH){
    //enter amount and payment details(phone number)  
    enterPaymentDetails();   

    if(amnt >= 10 && amnt <= 30){

      //create json document
      StaticJsonDocument<200> doc;

      //create object
      doc["amount"] = amnt;
      doc["phone"] = p;



      // Send STK push request
      http.begin(client, "http://192.168.8.110:5000/stkpush");
      http.addHeader("Content-Type", "application/json");
      int postCode = http.POST(json);
      
      delay(4000);
        
      if(postCode > 0){
          if(postCode == HTTP_CODE_OK || postCode == HTTP_CODE_MOVED_PERMANENTLY) {
              // Read response
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("STK push sent!!");
              lcd.setCursor(0,1);
              String response = http.getString();
              lcd.print("Check your phone");
              delay(15000);

              //confirm transaction status
              confirm();

            }
      }else{

          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(postCode).c_str());

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Failed to send STK"); 
          delay(2000);       
      }

      http.end();
    }

    yield();

  }
  else if(keypressed == '*'){         // * to change password       
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Enter password"); 
    
    //function to change password
    changeCode();
  }
  else if(keypressed == '#'){        //# to initiate override system
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Enter password");     

    //function to override system
    overrideSys();
  }
  delay(200);

}

void dispense(){
    switch(amnt){
    case 10:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Dispensing 10...");
      digitalWrite(pumpPin, HIGH);   // send signal from pin 9 to circuit
      delay(3600);                   // wait for 3.6 seconds
      digitalWrite(pumpPin, LOW);    // end signal
      delay(100); 
     
      break;

    case 20:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Dispensing 20...");
      digitalWrite(pumpPin, HIGH);   // send signal from pin 9 to circuit
      delay(7500);                   // wait for 7.5 seconds
      digitalWrite(pumpPin, LOW);    // end signal
      delay(100);
      break;
    
    case 30:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Dispensing 30...");
      digitalWrite(pumpPin, HIGH);   // send signal from pin 9 to circuit
      delay(11000);                   // wait for 11 seconds
      digitalWrite(pumpPin, LOW);    // end signal
      delay(100); 

      break;     
    
    default:
      if(Amt > 30 && Amt <= 90){
        delayOver = Amt*360;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Dispensing   ...");
        lcd.setCursor(11,0);
        lcd.print(Amt);
        digitalWrite(pumpPin, HIGH);   // send signal from pin 9 to circuit
        delay(delayOver);                   // wait for 12 seconds
        digitalWrite(pumpPin, LOW);    // end signal
        delay(100); 

        break;
      }else {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ASK FOR HELP!!");
        delay(2000);
        yield();
        break;
      }
  }
}

void confirm(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Confirming...");
  delay(6000);
  
  
  http.begin("http://192.168.8.110:5000/confirm"); 

  int getCode = http.GET();

  delay(1000);

  // getCode will be negative on error
  if(getCode > 0) {
      // file found at server
      if(getCode == HTTP_CODE_OK) {
          String payload = http.getString();
          int resultCode = payload.toInt(); 
          delay(2000);

          if(resultCode == 0){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("PAYMENT RECEIVED");
              delay(1000);           
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("PRESS # ");
              lcd.setCursor(0,1);
              lcd.print("TO DISPENSE");

              for( ; ; ){
                customKey = customKeypad.getKey();
                if (customKey) {
                  if (customKey == '#') {
                    
                    //dispense order
                    dispense();

                    break;
                  } else if(customKey == '*'){                     
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("Press # ");
                    lcd.setCursor(0,1);
                    lcd.print("to dispense");
                    }else {
                      lcd.clear();
                      lcd.setCursor(0,0);
                      lcd.print("Press # ");
                      lcd.setCursor(0,1);
                      lcd.print("to dispense");
                      }
                }
                yield();
              }

          } else{
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("TRANSACTION FAILED!!");
              delay(3000);                   
          }
                
        }
  } else {
      Serial.println("Error on HTTP GET request");
  }
}


void enterPaymentDetails(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Choose 1:Small");
  lcd.setCursor(0,1);
  lcd.print("2:Medium 3:Large");  
   
  delay(300);
  for( ; ; ){
   customKey = customKeypad.getKey();
   if (customKey) {
    if (customKey == '#') {
          m = amount;
          Amt = amount.toInt();
          amnt = Amt*10;
          amount ="";
          cursorColumn = 0;

          if(Amt > 3 || Amt < 1){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Invalid input!!");
            delay(2000);
            cursorColumn = 0;
            amount ="";
            break;
          } else {
            //function to enter phone number
            enterPhoneNumber();
            break;
          }                   
    } else if(customKey == '*'){
        amount = "";
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Choose 1:Small");
        lcd.setCursor(0,1);
        lcd.print("2:Medium 3:Large"); 
        cursorColumn = 0;
      }else {
        lcd.clear();
        lcd.setCursor(cursorColumn,0);
        lcd.print(customKey);
        amount += customKey;
        cursorColumn++;
    }
  }
  yield();
 }
}

void enterPhoneNumber(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Mpesa number");
  
  delay(300);  
    for( ; ; ){
    customKey = customKeypad.getKey();
        if (customKey) {
          if (customKey == '#') {
            p = number;
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Processing...");
            cursorColumn = 0;                  
            number = "";
            break;
          } else if(customKey == '*'){
                  number = "";
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Mpesa Number");
                  cursorColumn = 0;
            }else {
                  lcd.setCursor(cursorColumn,1);
                  lcd.print(customKey);
                  number += customKey;
                  cursorColumn++;
              }
      }
      yield();
  }
}


void changeCode(){
  
  for( ; ;){  
    char key = customKeypad.getKey();

    if (key) {
      Serial.println(key);

      if (key == '*') {
        input_password = ""; // reset the input password
        passColumn = 7;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter password"); 
      } else if (key == '#') {
          if (input_password == password) {
            Serial.println("Enter new password");
            input_password = ""; // reset the input password
            passColumn = 7;
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Enter new passwd");

            //new password
            for( ; ;){
              char key = customKeypad.getKey();
              if (key) {
                Serial.println(key);

                if (key == '*') {
                  new_password1 = ""; // reset the new password
                  passColumn = 7;
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Enter new passwd");
                } else if (key == '#') {
                    password = new_password1;
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("New password is:");
                    lcd.setCursor(7,1);
                    lcd.print(new_password1);
                    new_password1 = ""; // reset the new password
                    passColumn = 7;
                    delay(2000);
                    break;
                } else {
                  new_password1 += key; // append new character to new password string
                  lcd.setCursor(passColumn,1);
                  lcd.print(key);
                  passColumn++;
                }
              }
              yield();
            } 
            break;        
          } else {
              Serial.println("Invalid Password => Try again");
              input_password = ""; // reset the input password
              passColumn = 7;
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Invalid password!");
              delay(1000);
              break; 
            }
        } else {
            input_password += key; // append new character to input password string
            lcd.setCursor(passColumn,1);
            lcd.print("*");  
            passColumn++;
        }
    }
    yield();    
  }  
}


void overrideSys(){

  for( ; ;){
    char key = customKeypad.getKey();
    
    if (key) {
      Serial.println(key);
      
      if (key == '*') {
        input_password = ""; // reset the input password
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter password");
        passColumn = 7;
      } else if (key == '#') {
        if (input_password == password) {
          Serial.println("Valid Password => Enter amount");
          passColumn = 7;
          input_password = ""; // reset the input password

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Enter amount");
          delay(500);
          
          for( ; ; ){
            customKey = customKeypad.getKey();
            if (customKey) {
              if (customKey == '#') {
                    m = amount;
                    Amt = amount.toInt();
                    amnt = Amt;
                    cursorColumn = 0;
                    amount ="";
                    break;
              } else if(customKey == '*'){
                  amount = "";
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Enter amount");
                  cursorColumn = 0;
                }else {
                  lcd.setCursor(cursorColumn,1);
                  lcd.print(customKey);
                  amount += customKey;
                  cursorColumn++;
              }
            }
            yield();
          }   
          //dispense order
          dispense(); 
          break;   
          
          delay(100);
        } else {
          Serial.println("Invalid Password => Try again");
          input_password = ""; // reset the input password
          passColumn = 7;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Invalid password!");
          delay(1000);
          break;         
        }

        input_password = ""; // reset the input password
      } else {
        input_password += key; // append new character to input password string
        lcd.setCursor(passColumn,1);
        lcd.print("*");  
        passColumn++;
      }
    }
    yield();
  }
}
