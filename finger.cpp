// Inside your Arduino code where attendance data is ready
void sendData() {
  String url = "https://stud-attendance.onrender.com/uploadData";
  url += "?matric=" + matric + "&classHeld=" + nClass + "&attend=" + nAttend;
  url += "&present=" + nPresent + "&absent=" + nAbsent;
  
  // Make HTTP POST request to the server
  // Use the appropriate library for HTTP requests on Arduino
}


#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Software serial for when you dont have a hardware serial port
// pin #2 is IN from sensor (YELLOW wire)  //for R301 fingerprint module
// pin #3 is OUT from arduino  (BLUE wire)  //for R301 fingerprint module

SoftwareSerial mySerial (A8, A9);//(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
//                              {'1','2','3','A'},
//                              {'4','5','6','B'},
//                              {'7','8','9','C'},
//                              {'*','0','#','D'}
                               {'1','4','7','*'},
                               {'2','5','8','0'},
                               {'3','6','9','#'},
                               {'A','B','C','D'}
                            };
byte rowPins[ROWS] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad myKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

char strPass[] = "1111", strKey[6], matric[] = "    /    ";
char onoff, key, point=0; 
int record, col=0, knt=0, i=0, authorized=0, nClass, nAttend, classStop;  
unsigned long nTime;//=millis()+3000;
boolean halt;
float nAbsent, nPresent;
int buzzer=4;

const int dTime = 5000;

uint8_t id;
uint8_t getFingerprintEnroll();
uint8_t getFingerprintEnroll(uint8_t id);
int getFingerprintIDez();

void EnterCode();
void buzz(char onoff);
void ChangeCode();
void clearResult();

void setup(){ 
  // set the data rate for the sensor serial port  
  Serial.begin(9600);
  lcd.begin();  lcd.backlight();  lcd.clear();
  lcd.print("  ...Loading... ");  delay(8000);  lcd.clear();
  
  pinMode(buzzer, 1);  digitalWrite(buzzer, 0);
  
  finger.begin(57600);
  
  if(!finger.verifyPassword()){
    lcd.setCursor(0,0);   lcd.print("  Fingerprint   ");
    lcd.setCursor(0,1);   lcd.print("   Not Found!   ");
    while(1);  //delay(2000);  //
  }

  nAttend = EEPROM.read(0);  //No. of Attendee
  if(nAttend>100)  nAttend = 0;
  
  classStop = EEPROM.read(1);  //check if class has been stopped
  if(classStop==255)  classStop = 0;
  
  if(!classStop){
    nClass = EEPROM.read(2);  //number of classes held
    if(nClass==255)  nClass = 1;
  }
  
  char str1[4];
  for(int l=0; l<4;  l++)  str1[l] = EEPROM.read(l+4);
  if(isdigit(EEPROM.read(4)))  strncpy(strPass, str1, 4);  
  
  record = EEPROM.read(3);  //No. of registered users
  if(record>100)  record = 0;
    
  lcd.clear();
  
//  for(int y=0;  y<1000;  y++)  EEPROM.update(y, 255);
}

void saveMatric(char str1[], int Id){
  int index = (Id + 49) * 9;
  
  for(int y=0;  y<9;  y++)  EEPROM.update(index+y, str1[y]);
}

int searchMatric(char str1[]){
  char str2[9];  int x=441, num=0;
  do{
    x += 9;  num++;  //num = (x / 9) - 49;
    if(!isdigit(EEPROM.read(x)))  continue;
    for(int y=0;  y<9;  y++)  str2[y] = EEPROM.read(x+y);
    if(!strncmp(str1, str2, 9) && EEPROM.read(300+num)==num)  return num;
  }while(x<1000);
  
  return 0;
}

int getId(){
  for(int a=301; a<=400; a++){
    if(EEPROM.read(a)==255)  return a-300;
  }
  return 0;
}

void searchId(int Id){
  int x=441, num;
  char str1[9];
  do{
    x += 9;  num = (x / 9) - 49;
    if(num==Id && isdigit(EEPROM.read(x))){
      for(int y=0;  y<9;  y++){
        matric[y] = EEPROM.read(x+y);
      }
      return; //(num);
    }
  }while(x<1000);
  //return 0;
}

int getMatric(){
  uint8_t num = 0, n=0, brk=0;
  unsigned long tim = millis() + dTime;
  char mat[] = {"         "};
  
  while(millis()<tim && !brk){
    key = myKeypad.getKey();  lcd.setCursor(2, 1);  lcd.print(mat);
    if(key){
      tim = millis() + dTime;
      if(isdigit(key)){
        if(n<9)  mat[n] = key;
        if(n==3)  n++;
        else if(n==5)  mat[4] = '/';
        
        n++;
      }
      else if(key=='#'){
        if(n>=9){
          strncpy(matric, mat, 9);  return 1;
        }
      }
      else if(key == '*' && n){    
        lcd.setCursor(col, 0);  strncpy(mat, "         ", 9);  n=0;
      }
      else if(key == '*' && !n){    
        num=0;   lcd.clear();  brk=1;  halt = true;
      }      
    }
  }

  return 0;
}

void menu() {
  if(!classStop){
    lcd.setCursor(0,0);  lcd.print("   Class: ");  lcd.print(nClass);
    lcd.setCursor(0,1);  lcd.print("Attendee: ");  lcd.print(nAttend);  
  }
  else{
    lcd.setCursor(0,0);  lcd.print("     Class      ");
    lcd.setCursor(0,1);  lcd.print("  Terminated!   ");
  }
  i=0;
}

void trigMsg() {
  lcd.clear();
  lcd.setCursor(1,0);  lcd.print(matric);
  lcd.setCursor(0,1);  lcd.print("P: ");
  lcd.print(nPresent, 0);  lcd.print("%");
//  lcd.setCursor(13,1);
  lcd.print("    A: ");
  lcd.print(nAbsent, 0);  lcd.print("%");
}

void sendData(){
  String url = "";
  url = String(matric) + "&classheld=" + String(nClass) + "&attend=" + String(nAttend);
  url += "&present=" + String(nPresent, 0) + "&absent=" + String(nAbsent, 0) + "\"\r\n";

//  Serial.print(url);

  Serial.println("AT+CGATT=1\r\n");  delay(100);
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");  delay(2000);

  // Serial.println("AT+SAPBR=3,1,\"APN\",\"internet.ng.airtel.com\"\r\n");
  Serial.println("AT+SAPBR=3,1,\"APN\",\"web.gprs.mtnnigeria.net\"\r\n");
  // Serial.println("AT+SAPBR=3,1,\"APN\",\"glosecure\"\r\n");
  delay(4000);  lcd.clear();
  
  Serial.println("AT+SAPBR=1,1\r\n");  delay(2000);

  Serial.print("AT+HTTPINIT\r\n");  delay(2000);
  
  Serial.print("AT+HTTPPARA=\"CID\",1\r\n");  delay(2000);
  
  Serial.print("AT+HTTPPARA=\"URL\",\"http://aypteksolutions.com.ng/iot/");
  Serial.print("classattendance/ClassSend.php?studentid=");
  Serial.print(url);  delay(2000);

  Serial.println("AT+HTTPACTION=0\r\n");  //submit the request
  delay(10000);

  //  Serial.println("AT+HTTPREAD");  delay(100);  //toSerial()
  Serial.print("AT+HTTPTERM\r\n");  delay(300);
  
  lcd.print(" Data Uploading ");  lcd.clear();
}

void loop(){                     // run over and over again
  key = myKeypad.getKey();  authorized=0;  clockIn();
  
  if(!i || millis()>=nTime+dTime)  menu(); 
  
  if(key){
    if(!i){
      lcd.clear();  strncpy(strKey, "", 6);  point=0;
    }
        
    if(isdigit(key)){  
      i++;  lcd.setCursor(5+i,1);  lcd.print('*');  lcd.setCursor(5,0);
      lcd.print(" Admin ");  strKey[i-1] = key;  nTime = millis();
    }
    
    else if(key=='#'){
      delay(200);  i=0;
      if(!strncmp(strKey, strPass, 4) && point && strKey[5]=='1')  ChangeCode();
      else if(!strncmp(strKey, strPass, 4) && point && strKey[5]=='2'){
        lcd.clear();  lcd.print("REGISTERED: ");  lcd.print(record);  delay(2000);
        lcd.clear();
      }      
      else if(!strncmp(strKey, strPass, 4) && point && strKey[5]=='3'){
        if(!classStop){
          key = 0;  lcd.clear();  lcd.print("  New Class?   ");  i=0;
          while(key != '*' && key != '#')  key = myKeypad.getKey();
          if(key == '#'){  nAttend=0;  nClass++;  lcd.clear();  }
        }
      }
      else if(!strncmp(strKey, "102030", 6)){           
        for(int l=0; l<4;  l++)  EEPROM.write(l+4, '1');
        strncpy(strPass, "1111", 4);  lcd.clear();  lcd.print("Password Reset!");  buzz(1);
      }
      else{
        lcd.clear();  lcd.print("Unknown Command!");  buzz(0);  i=0;
      }
      lcd.clear();
    }
   
    else if(key=='*'){
      i++;  lcd.setCursor(5+i,1);  lcd.print('*');  lcd.setCursor(5,0);  
      lcd.print(" Admin ");  strKey[i-1] = key;  nTime = millis();
      if(i!=5 || point){  lcd.clear();  i=0;  }
      else if(!point)  point++;
    }

    else if(key == 'C' && !i && classStop!=1){   
      buzz(0);  lcd.clear();  lcd.print("  Collate Data  ");  key=0;      
      lcd.setCursor(0,1);    lcd.print("  Stop Record?  ");  nTime = millis();
      while(key!='*' && key!='#' && millis() < nTime + dTime)  key = myKeypad.getKey();
      if(key=='#'){  
        EnterCode();
        if(authorized){
          classStop=1;  EEPROM.write(1, classStop);    
        }
      }
      i=0;  lcd.clear();
    }
  
    else if(key == 'D' && !i){
      buzz(0);  key=0;  lcd.clear();  lcd.print("Reset Device?  ");  nTime = millis();
      while(key!='*' && key!='#' && millis() < nTime + dTime)  key = myKeypad.getKey();
      if(key=='#'){
        EnterCode();  i=0; 
        if(authorized)  clearResult();                          
      }
      i=0;  lcd.clear();
    }
  
    else if(key == 'A' && !i){
      delay(100); 
      if(record>=100){
        lcd.clear();  lcd.print(" No More Space  ");  buzz(0);
      }
      else if(!classStop){
        EnterCode();  i=0;  halt = false;
        if(authorized)  getFingerprintEnroll();        
        while(authorized && !halt && record<100)  getFingerprintEnroll();        
      }
      i=0;  lcd.clear();
    }

    else if(key == 'B' && !i  && !classStop){
      delay(200);  EnterCode();  i=0; 
      if(authorized){
        lcd.clear();  lcd.print("Delete Matric: ");
        col=7;  id = getMatric();
        if(id)  removeID(id);
      }  
      i=0;  lcd.clear();  
    }
    
    if(i>6){  i=0;  lcd.clear();  }
  }
}

void clockIn(){
  boolean chk=false;  int fpID=0, present=0;
  uint8_t p = finger.getImage();
    
  if (p == FINGERPRINT_OK){
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK){
      p = finger.fingerFastSearch();  i=0;
      if (p == FINGERPRINT_OK){
        fpID = finger.fingerID;  //
//        lcd.clear();  lcd.print("fpID: ");  lcd.print(fpID);  delay(3000);
/////////////////////////////////////////check if user is registered/////////////////////////////////////////////
//        if(searchId(fpID))  chk = true;
        for(int a=301; a<=400; a++){
          if (fpID==EEPROM.read(a)){
            searchId(fpID);  chk=true;  break;
          } 
        }        
///////////////////////////////////////check if user clocked in before////////////////////////////////////////////
      }
      if(chk){
        present = EEPROM.read(fpID+100);
        if(present==255)  present = 0;
        
        if(!classStop){
          if(nClass != EEPROM.read(2))  EEPROM.write(2, nClass);

          if(nClass != EEPROM.read(fpID+200)){
            //INCREMENT NUMBER OF ATTENDEE
            nAttend++;  delay(20);  EEPROM.write(0, nAttend);
            //INCREMENT NUMBER OF PRESENT            
            present++;  EEPROM.write(fpID+100, present);
            //LOG IN FOR TODAY'S CLASS          
            EEPROM.write(fpID+200, nClass);
          }
//           else{
//             lcd.clear();  lcd.print("Already Taken!");  //delay(2000);
// //            lcd.setCursor(0,1);  lcd.print("nCl: ");  lcd.print(nClass);  
// //            lcd.print(" fp: ");  lcd.print(fpID);  delay(3000);
//           }
        }
        else{
           nClass = EEPROM.read(2);  
           if(nClass==255)  nClass = 0;
        }
        
        nPresent = float(present) / float(nClass) * 100.0;
        nAbsent =  (float(nClass) - float(present)) / float(nClass) * 100.0;

        trigMsg();  /*buzz(1);*/  sendData();
      }
      else{
        lcd.clear();  lcd.print("Not Registered !");  buzz(0);
//        lcd.setCursor(0,1);  lcd.print("p: ");  lcd.print(p);  
//        lcd.print(" fp: ");  lcd.print(fpID);  buzz(0);        
      }
    }
  }    
}

void removeID(uint8_t urID){  
  uint8_t p = -1, Id, index;  boolean chk=0;

  Id = searchMatric(matric);  //index = (Id + 49) * 9;
  
  if(Id){
    p = finger.deleteModel(Id);  chk=true;
    EEPROM.update(Id+300, 255);  //Delete ID 
    EEPROM.update(Id+100, 255);  //Delete No. of Present
    EEPROM.update(Id+200, 255);  //Delete Attend Status
    if(record>0)  record--;  EEPROM.update(3, record);
    lcd.clear();  lcd.setCursor(4, 0);  lcd.print(matric);
    lcd.setCursor(0, 1);  lcd.print("Record Removed!");  buzz(1);
  }
  
  if(!chk){
    lcd.clear();  lcd.setCursor(4, 0);  lcd.print(matric);  
    lcd.setCursor(0, 1);  lcd.print("Record Not Found!");  buzz(0);
  }
}

void clearResult(){  
  uint8_t p = -1;   char idd; 
 
  for(int a=10;  a<1000;  a++)  EEPROM.update(a, 255);
  
  record = 0;  EEPROM.update(3, record);  finger.emptyDatabase();
  classStop=0;  EEPROM.update(1, classStop);  //Restart Class
  nClass=1;  EEPROM.update(2, nClass);
  nAttend=0;  EEPROM.update(0, nAttend);
  lcd.clear();  lcd.print(" Device Reset!");  buzz(1);
}

uint8_t getFingerprintEnroll() {
  int p = -1, Id;//, pp = -1;

  halt = false;  lcd.clear();  lcd.print("Register Matric:   ");  
  col=4;  id = getMatric();

  if(id){
    Id = searchMatric(matric);
    if(Id){
      lcd.clear();  lcd.print(matric);  lcd.print(" Exists!");
      buzz(0);  id=0;   return -1;
    }

    //generate new ID
    Id = getId();  //lcd.clear();  lcd.print("getId");  lcd.print(Id);
    if(!Id){
      lcd.clear();  lcd.print("Error getting ID");  buzz(0);  return -1;
    }
  
    lcd.setCursor(0,1);  lcd.print("   Scan Finger   ");  //p = finger.getImage();   
    
    while(p != FINGERPRINT_OK && key!='*'){
      p = finger.getImage();  key = myKeypad.getKey(); 
    }
  
    if(key == '*'){
      lcd.clear();  lcd.print("    Cancelled");  buzz(0);
      halt = true;  return -1;
    }
    
    p = finger.image2Tz(); 
    if (p != FINGERPRINT_OK) {
      lcd.clear();  lcd.print(" Not Converted ");  buzz(0);   
      return -1;
    }

////////////////////////////////////////////////////////////////////////////////////
    p = finger.fingerFastSearch();                                                //
    if (p == FINGERPRINT_OK) {                                                    //
      lcd.setCursor(0,1);  lcd.print("  Finger Used ! ");  buzz(0);               //
      return -1;                                                                  //
    }                                                                             //
////////////////////////////////////////////////////////////////////////////////////
    
    p = 0;  lcd.setCursor(0,1);   lcd.print("  Remove Finger ");
    while (p != FINGERPRINT_NOFINGER)  p = finger.getImage();
    
    lcd.setCursor(0,1);  lcd.print("Scan Same Finger");
    p = finger.getImage();
    
    while (p != FINGERPRINT_OK && key!='*') {
      p = finger.getImage();  key = myKeypad.getKey();   
    }            
  
    if(key == '*'){
      lcd.clear();  lcd.print("    Cancelled    ");  buzz(0);
      halt = true;  return -1;
    }
  
    p = finger.image2Tz(1);
    if (p != FINGERPRINT_OK) {
      lcd.clear();  lcd.print(" Not Converted ");  buzz(0);  
      return p;
    }
    
////////////////////////////////////////////////////////////////////////////////////
    p = finger.fingerFastSearch();                                               //
    if (p == FINGERPRINT_OK) {                                                    //
      lcd.setCursor(0,1);  lcd.print("  Finger Used ! ");  buzz(0);               //
      return -1;                                                                  //
    }                                                                             //
////////////////////////////////////////////////////////////////////////////////////
    
    p = 0;  lcd.setCursor(0,1);   lcd.print("  Remove Finger ");
    while (p != FINGERPRINT_NOFINGER)  p = finger.getImage();
    
    lcd.setCursor(0,1);   lcd.print("Scan Same Finger");
    p = finger.getImage();
    
    while (p != FINGERPRINT_OK && key!='*') {
      p = finger.getImage();    key = myKeypad.getKey();   
    }  

    if(key == '*'){
      lcd.clear();  lcd.print("    Cancelled    ");  buzz(0);
      halt = true;  return -1;
    }
    
    p = finger.image2Tz(2);
    if (p != FINGERPRINT_OK){  
      lcd.clear();  lcd.print(" Not Converted ");  buzz(0);  return p;
    }   

////////////////////////////////////////////////////////////////////////////////////
    p = finger.fingerFastSearch();                                                //
    if (p == FINGERPRINT_OK){                                                     //
      lcd.setCursor(0,1);  lcd.print("  Finger Used ! ");  buzz(0);               //
      return -1;                                                                  //
    }                                                                             //
////////////////////////////////////////////////////////////////////////////////////
    
    p = finger.createModel();   lcd.setCursor(0,1);
    if (p != FINGERPRINT_OK){    
      lcd.clear();  lcd.print("  Not Created  ");  buzz(0);  return p;
    }
    
    p = finger.storeModel(Id);  lcd.setCursor(0,1);
    if (p == FINGERPRINT_OK){
      saveMatric(matric, Id);  EEPROM.write(300+Id,Id);  record++;
      EEPROM.write(3,record);  lcd.print("  Registered !  ");  buzz(1);
    }
    else {
      lcd.clear();  lcd.print("   Not Stored !  ");  buzz(0);  return p;
    }  
  }  
}

void EnterCode(){
  char myCode[4];  byte y=0, cancelOp=0, brk=0;
  unsigned long tim = millis() + dTime;

  lcd.clear();  lcd.print(" Enter Password ");  //lcd.setCursor(6, 1);  cancelOp=0;
  authorized=0;  delay(200);
  while(millis()<tim && y<5 && !cancelOp && !brk){
    key = myKeypad.getKey();
    if(key){
      tim = millis() + dTime;   
      if(isdigit(key)){
        myCode[y] = key;  y++;  lcd.setCursor(y+5, 1);  lcd.print('*');
      }
      else if (key == '*' && y){    
        lcd.clear();  lcd.print(" Enter Password ");  y=0;  strncpy(myCode, "", 4);
      }
      else if (key == '*' && !y){  cancelOp = 1;  return;  }
      else if(key=='#')  brk = 1;  
    }    
  }

  if(brk==1){
    if(!strncmp(strPass, myCode, 4))  authorized=1;
    else{  
      lcd.clear();  lcd.print("Wrong Password !");  buzz(0);
    }
  }
}

void ChangeCode(){
  char code1[4], code2[4], code3[4];
  byte y=0, cancelOp=0, brk=0;  boolean goNext=false;
  unsigned long tim = millis() + dTime;

  lcd.clear();  lcd.print("  Old Password  ");  cancelOp=0;

  while(millis()<tim && y<5 && !cancelOp && !brk){
    key = myKeypad.getKey();
    if(key){      
      tim = millis() + dTime;
      if(isdigit(key)){
        code1[y] = key;  y++;  lcd.setCursor(y+5, 1);  lcd.print('*');            
      }
      else if(key == '*' && y){
        lcd.clear();  lcd.print("  Old Password  ");
        y=0;  strncpy(code1, "", 4);
      }
      else if(key == '*' && !y){  cancelOp = 1;  return;  }
      else if(key=='#')  brk = 1;  
    }    
  }

  if(brk==1){
    if(!strncmp(strPass, code1, 4))  goNext = true;
    else{
      lcd.clear();  lcd.print("Wrong Password !");  buzz(0);
    }
  }

  if(goNext){
    lcd.clear();  lcd.print("  New Password  ");  y=0;  goNext=false;  brk = 0;
  
    while(millis()<tim && y<5 && !cancelOp && !brk){
      key = myKeypad.getKey();
      if(key){
        tim = millis() + dTime;
        if(isdigit(key)){
          code2[y] = key;  y++;  lcd.setCursor(y+5, 1);  lcd.print('*'); 
        }
        else if(key == '*' && y){
          lcd.clear();  lcd.print("  New Password  ");  y=0;  strncpy(code2, "", 4);
        }
        else if(key == '*' && !y){  cancelOp = 1;  return;  }
        else if(key=='#'){  brk = 1;  goNext=true;  }
      }    
    }
  }

  if(goNext){
    lcd.clear();  lcd.print("Confirm Password");  y=0;  brk = 0;  
  
    while(millis()<tim && y<5 && !cancelOp && !brk){
      key = myKeypad.getKey();
      if(key){
        tim = millis() + dTime;
        if(isdigit(key)){
          code3[y] = key;  y++;  lcd.setCursor(y+5, 1);  lcd.print('*');
        }
        else if(key == '*' && y){
          lcd.clear();  lcd.print("Confirm Password");  y=0;  strncpy(code3, "", 4);
        }
        else if (key == '*' && !y){  cancelOp = 1;  return;  }
        else if(key=='#')  brk = 1;  
      }    
    }
  
    if(brk==1){
      if(!strncmp(code2, code3, 4)){
        lcd.clear();  lcd.print("    Password"); 
        lcd.setCursor(4,1);  lcd.print("Changed");  buzz(1);
        for(int l=0; l<4;  l++)  EEPROM.update(l+4, code2[l]);    
        strncpy(strPass, code2, 4);         
      }
      else{  
        lcd.clear();  lcd.print("   Not Match ! ");  buzz(0); 
      }
    }
  }
}

void buzz(char onoff){
  char state=0;
  
  if(!onoff){
    for(int v=1; v<=6; v++){  state=~state;  digitalWrite(buzzer, state);  delay(100);  }
    delay(1000);  
  }
  else if(onoff){
    digitalWrite(buzzer, 1);  delay(1000);  digitalWrite(buzzer, 0);  delay(1000); 
  }
  lcd.clear();
}
