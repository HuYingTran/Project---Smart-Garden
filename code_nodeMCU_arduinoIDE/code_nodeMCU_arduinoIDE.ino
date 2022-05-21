#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//-------------------------------------------------------------
const char* ssid = "Huynh Tran";        //-->wifi name.
const char* password = "12345678";     //-->wifi password.
//-------------------------------------------------------------
FirebaseData firebaseData;
#define FIREBASE_HOST "https://qtfirebaseintegrationexa-942c6-default-rtdb.firebaseio.com" //--> URL address of Firebase Realtime Database.
#define FIREBASE_AUTH "VL316DNkgpysG9g46LgH7SjbBeBTdlGljufQDJ1A" //--> firebase database secret code.
//--------------------------------------------------------------
#define relayFan 5    // D1
#define relayLight 4      // D2
#define relayPump 0     // D3
#define lightSensor 14  // D5
//14 D5
#define soilSensor A0   // A0
//--------------------------------------------------------------
#define DHTTYPE DHT11 //--> khai bao loai cam bien DHT
const int DHTPin = 2; //--> Chan cam bien nhiet do DHT11 / D4
DHT dht(DHTPin, DHTTYPE); //--> Khoi tao cam bien
float t,h,w,l;
//------------------------------tinh toan thoi gian theo mui gio
// > UTC +07:00 -> 7 * 60 * 60 = 25200
const long utcOffsetInSeconds = 25200;
//-----------------------------------------xac dinh ung dung NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//--------------------------------------------------------------
unsigned long previousMillis = 0;        //--> bien luu tru lan cuoi duoc cap nhap
const long interval = 1000;           //--> khoang thoi gian

//================================================================================================================ void setup
void setup() {
  Serial.begin(9600); //---------------> khoi tao Serial Monitor
  delay(500);

  dht.begin();  //-------------------> bat dau chay cam bien DHT
  delay(500);
  
  WiFi.begin(ssid, password); //--------------> Ket noi toi Wifi
  Serial.println("");
    
  //-------------------------------------Khai bao chuc nang chan
  pinMode(relayLight,OUTPUT);
  pinMode(relayFan,OUTPUT);
  pinMode(relayPump,OUTPUT);
  pinMode(lightSensor,INPUT);
  pinMode(soilSensor,INPUT);

  //-------------------------------------------Thong bao ket noi
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //---------------------------------------- hien thi dia chi IP
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //-------------------------------------------cau hinh Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  //--------------------------khoi tao NTPClient de co thoi gian
  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
  delay(1000);
}

//========================================================================================================================void loop
void loop() {
  //------------------------------------------------------- doc gia tri cac cam bien
  h = dht.readHumidity();
  t = dht.readTemperature(); // read do C
  w = round((1024-analogRead(soilSensor))*(10000/812))/100;
  l = digitalRead(lightSensor);
  
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; //-->luu lai thoi gian cuoi
 
    //----------------------------------------Update du lieu 5s/lan
    timeClient.update();   
    int everyfive = timeClient.getSeconds();
    
    if (everyfive %5 == 0) { //--> gui va nhan du lieu 5s, 10s, 15s...
      unsigned long epochTime = timeClient.getEpochTime();
      struct tm *ptm = gmtime ((time_t *)&epochTime);
      //---------------------------------tinh toan ngay/thang/nam
      int monthDay = ptm->tm_mday; 
      int currentMonth = ptm->tm_mon+1;
      int currentYear = ptm->tm_year+1900;
      String strmonthDay;
      String strcurrentMonth;
      if (monthDay <10) {
        strmonthDay = "0" + String(monthDay);
      } else {
        strmonthDay = String(monthDay);
      }
      if (currentMonth <10) {
        strcurrentMonth = "0" + String(currentMonth);
      } else {
        strcurrentMonth = String(currentMonth);
      }
      String currentDate = strmonthDay + "-" + strcurrentMonth + "-" + String(currentYear); //--> ngay + thang + nam
  
      String currentTime = timeClient.getFormattedTime();  
      //----------------------------------------gui thoi gian len Firebase.
      Firebase.setString(firebaseData,"RealTimeData/Time",currentTime);
    
      //------------------------------------------------------------------------------------------------------------doc cac cam bien
      //------------------------------------------------------------------------ doc cam bien anh sang
      String strLight;
      if(l==1){
        strLight = "LOW"; //--> do sang
      }else if(l==0){
        strLight = "HIGH";
      }
      Firebase.setString(firebaseData,"RealTimeData/Value_Light",strLight); //--> do sang
      //------------------------------------------------------------------------------ doc cam bien dat
//      Serial.print("Wet: ");
//      Serial.print(w);
//      Serial.print(" % ");
      String strWet = String(w);
      Firebase.setString(firebaseData,"RealTimeData/Value_Wet",strWet); //--> do am dat
      //------------------------------------------------------------------- code doc nhiet do cua DHT11
      // doc toc do 250mili giay
      
      // float tf = dht.readTemperature(true); // read do F
      //-----hien thi du lieu DHT11 len Serial Monitor
      //Serial.print(F("Humidity: "));
      //Serial.print(h);
      //Serial.print(F("%  Temperature: "));
      //Serial.print(t);
      //Serial.println(F("°C "));
      //-------------------Convert number sang string
      String strHum = String(h);
      String strTem = String(t);
      //------------------------------------------gui du lieu DHT11 len SmartHome Database.
      Firebase.setString(firebaseData,"RealTimeData/Value_Humidity",strHum); //--> do am
      Firebase.setString(firebaseData,"RealTimeData/Value_Temperature",strTem); //--> nhiet do
      //----------------------------------------

      //------------------------------------cap nhap du lieu vao DHT11Database cua Firebase
      String DateAndTime = currentDate + "_" + currentTime; //--> d/y/y + time
//      Firebase.setString(firebaseData,"DHT11Database/" + DateAndTime + "/Date",currentDate); //--> ngay
//      Firebase.setString(firebaseData,"DHT11Database/" + DateAndTime + "/Time",currentTime); //--> thoi gian
      Firebase.setString(firebaseData,"DHT11Database/" + DateAndTime + "/Humidity",strHum); //--> do am
      Firebase.setString(firebaseData,"DHT11Database/" + DateAndTime + "/Temperature",strTem); //--> nhiet do
      Firebase.setString(firebaseData,"DHT11Database/" + DateAndTime + "/Light",strLight); //--> anh sang
      Firebase.setString(firebaseData,"DHT11Database/" + DateAndTime + "/Wet",strWet); //--> do am dat
      //-----------------------------------------------------------------------------------
    }
    //------------------------------------------------------------------------------------------------------------ <truy xuat data nut an tu Firebase>
    //------------------------------------------------------------------------------------------------------------
    if(Firebase.getString(firebaseData,"AutoData/Status_Auto")){
      if(firebaseData.stringData()=="\"ON\""){ //----------------------------------------------- Auto
        //if(Firebase.getString(firebaseData,"AutoData/Value_Light"))
        //int value = digitalRead(lightSensor);
        
          //Serial.print(firebaseData.stringData().toInt());
        if(Firebase.getString(firebaseData,"AutoData/Value_Temperature"))
          if(firebaseData.stringData().toInt() < t){
            digitalWrite(relayFan,LOW);
            if(l == 1){
              digitalWrite(relayLight,LOW);
            }else{
              digitalWrite(relayLight,HIGH);
            }
          }else{
            digitalWrite(relayLight,LOW);
          }
        if(Firebase.getString(firebaseData,"AutoData/Value_Humidity"))
          if(firebaseData.stringData().toInt() < h){
            digitalWrite(relayFan,LOW);
          
        if(Firebase.getString(firebaseData,"AutoData/Value_Wet"))
          //Serial.print(String(w));
          Serial.print("\n");
          Serial.print(String(firebaseData.stringData().toInt()));
          if(firebaseData.stringData().toInt() > w){
            Serial.print(String(w));
            digitalWrite(relayPump,LOW);
          }else{
            digitalWrite(relayPump,HIGH);
          }
      }else{//----------------------------------------------------------------------------------- Manual
        //------------------------------------------------------------------ trang thai den
        if(Firebase.getString(firebaseData,"RealTimeData/Relay_Light")){
          if(firebaseData.stringData()=="\"ON\""){
            digitalWrite(relayLight,LOW);
          }else{
            digitalWrite(relayLight,HIGH);
          }
        }
        //------------------------------------------------------------------ trang thai quat
        if(Firebase.getString(firebaseData,"RealTimeData/Relay_Fan")){
          if(firebaseData.stringData()=="\"ON\""){
            digitalWrite(relayFan,LOW);
          }else{
            digitalWrite(relayFan,HIGH);
          }
        }
        //------------------------------------------------------------------ trang thai bom
        if(Firebase.getString(firebaseData,"RealTimeData/Relay_Pump")){
          if(firebaseData.stringData()=="\"ON\""){
            digitalWrite(relayPump,LOW);
          }else{
            digitalWrite(relayPump,HIGH);
          }
        }
      }
    }
  }    
}
