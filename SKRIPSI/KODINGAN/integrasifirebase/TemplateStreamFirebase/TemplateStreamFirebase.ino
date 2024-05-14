#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <Wire.h> //buat RTC & LCD kalo pake
#include <RtcDS3231.h>  //RTC library makunalib
#include <EEPROM.h>
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define WIFI_SSID "ICHA" 
#define WIFI_PASSWORD "jakarta682" 

#define API_KEY "AIzaSyAx7gQ7QVpawuz9FgpqT6bU0Wsz2hmhNsg" 
#define DATABASE_URL "crudmedlist-8b416-default-rtdb.firebaseio.com"  

// //ini buat led 
// #define MD1 D3 
// #define MD2 D4 
// #define MD3 D5 
// #define MD4 D6 

#define buzzer  26
#define button  34
#define I2C_SDA 21
#define I2C_SCL 22
#define buzzertime 3000

RtcDS3231<TwoWire> Rtc(Wire); //Uncomment for version 2.0.0 of the rtc library

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


//_______________________________SETUP ALARM_____________________________________
FirebaseData firebaseData1;
FirebaseData firebaseData1;

String parentPath = "/schedules";
String childPath[3] = {"/MS1", "/MS2", "/MS3"};

size_t childPathSize = 3;

String parentPathTime = "/schedules/T";
String childPathTime[6] = {"/AH1", "/AH2", "/AH3", "/AM1", "AM2", "AM3"};

size_t childPathTimeSize = 6; 

void streamCallback(MultiPathStreamData stream)
{
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.printf("path: %s, event: %s, type: %s, value: %s%s", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");
      
      handlePin(stream.dataPath.c_str(), stream.value.c_str());
 
    }
  }

  Serial.println();


  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());
}


void streamCallbackTime(MultiPathStreamData stream)
{
  size_t numChild = sizeof(childPathTime) / sizeof(childPathTime[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPathTime[i]))
    {
      Serial.printf("path: %s, event: %s, type: %s, value: %s%s", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");
      
      handlePinTime(stream.dataPath.c_str(), stream.value.c_str());

    }

  }

  Serial.println();


  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());
}



void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

//__________________________________VARIABEL SETUP________________________________________
int AV1, AV2, AV3, pDay; //menandakan alarm sudah berbunyi dg nilai 0 dan 1 (sudah menyala) jika ada network wifii realtime firebase 
int AVR1, AVR2, AVR3;//menandakan alarm sudah berbunyi yg berasal dr eeprom
int Years, Months, Days, Hours, Min, Sec,timeStamp, date;

String k, m1, m2, m3, iv; 
// k utk simpan nilai string dr firebase sblm di convert ke tipe data array , letak obat
//iv utk nilai string firebase jam alarm ke int 
//m1,m2,m3 utk mengekstrak nilai indeks ke-n dari array yg ada di K

int MS1[] = {1,0,3};
int MS2[] = {1,2,3};
int MS3[] = {0,2,3};

/* inisialisasi awal variabel dengan nilai default 
sebagai contoh atau sebagai tanda untuk menjelaskan 
format data yang akan diterima dari Firebase.*/


int M1, M2, M3; //simpan hasil konversi string ke array trs diambil int kompartemen letak obat 
int ind1, ind2, ind3; //mencari indeks ke 1, 2, 3 dari karakter string firebase yg ada di K


int AH1, AM1, AH2, AM2, AH3, AM3; // waktu ketika alarm harus diatur untuk berbunyi.
int W; //memanggil M1,M2,M3 sbg nomor obat yang akan diberikan di fungsi lokal
int AHR1, AMR1, AHR2, AMR2, AHR3, AMR3; //utk pengulangan alarm di eeprom
int onv=1, ofv=0;  // untuk mengontrol output pada pin untuk mengaktifkan atau menonaktifkan perangkat keras.

// value to be adjust according to your choice
int rt=1;// repeat time gap in menit, 1 = 1 menit
unsigned long lm, HT = 5000; // time is in milisecond//// **It is the time how much the led will glow after alarm

void setup() 
{
 
 Serial.begin(115200);
 EEPROM.begin(4096);

  stream.keepAlive(5, 5, 1);
 
// /*  led didefinisikan sbg output dg inisiasi awal off*/
//  pinMode(MD1,OUTPUT); pinMode(MD2,OUTPUT); pinMode(MD3,OUTPUT); pinMode(MD4,OUTPUT); 
//  digitalWrite(MD1,ofv); digitalWrite(MD2,ofv); digitalWrite(MD3,ofv); digitalWrite(MD4,ofv); Serial.println("All off");

 //_____________________________________LCD SETUP_________________________________________________
  lcd.init(); // initialize the LCD
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Hi, I'm MedStance!");
  lcd.setCursor(3,1);
  lcd.print("Your Personal");
  lcd.setCursor(0,2);
  lcd.print("Medicine Assistance");
  delay(3000);
//_____________________________________BUZZER SETUP_________________________________________________
  pinMode(buzzer, OUTPUT);
  

//_____________________________________BTN SETUP_________________________________________________
  int btn = 14;
  bool alarmStopped = false;   // Flag to track if the alarm is stopped
  unsigned long alarmStartTime; // Variable to store the start time of the alarm
  unsigned long buttonPressTime; // Variable to store the time when the button was pressed

//_____________________________________RTC begin_________________________________________________
   
  Wire.begin(I2C_SDA, I2C_SCL);
  Rtc.Begin();
  /*RtcDateTime currentTime = RtcDateTime(2022, 01, 29, 20,35, 0); //define date and time object
  rtcObject.SetDateTime(currentTime); //configure the RTC with object*/

  //______________________________________Checking the date once____________________________________________
  RtcDateTime currentTime = Rtc.GetDateTime(); //baca waktu dan disimpan dlm object variabel
  pDay = currentTime.Day();

  Serial.print(currentTime.Year());Serial.print("/");Serial.print(currentTime.Month());Serial.print("/");Serial.print(currentTime.Day());Serial.print("-----");
  Serial.print(currentTime.Hour());Serial.print(":");Serial.print(currentTime.Minute());Serial.print(":");Serial.print(currentTime.Second());Serial.println();
  
  if (currentTime.Year() == 2000) {
    Serial.println("RTC not set");
  } else { 
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("TIME CONFIGURATION ");
    lcd.setCursor(6,2);
    lcd.print("SUCCESS ");
    successtone(); 
    lcd.clear();
  } 


//_______________________________________EEPROM set______________________________________________
 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  delay(5000);
  
  if (WiFi.status() == WL_CONNECTED)
 {
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;

  config.signer.test_mode = true;
  Firebase.reconnectNetwork(true);

  firebaseData1.setBSSLBufferSize(4096, 1024);
  firebaseData2.setBSSLBufferSize(4096, 1024);
  firebaseData2.setResponseSize(1024);
  firebaseData2.setResponseSize(1024);

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  delay(2000);
  
  if (!Firebase.beginMultiPathStream(firebaseData1, parentPath, childPath, childPathSize))
  {
    Serial.println("-------------------------------------------------------");
    Serial.println("Can't begin stream connection..");
    Serial.println("Reason, %s\n\n" + firebaseData1.errorReason().c_str()); 
    Serial.println("-------------------------------------------------------");
    Serial.println();
  }

  Firebase.setMultiPathStreamCallback(firebaseData1, streamCallback, streamTimeoutCallback);

  if (!Firebase.beginMultiPathStream(firebaseData2, parentPathTime, childPathTime, childPathTimeSize))
  {
    Serial.println("-------------------------------------------------------");
    Serial.println("Can't begin stream connection..");
    Serial.println("Reason, %s\n\n" + firebaseData2.errorReason().c_str()); 
    Serial.println("-------------------------------------------------------");
    Serial.println();
  }

  Firebase.setMultiPathStreamCallback(firebaseData2, streamCallback, streamTimeoutCallback);



 if (Firebase.ready()) 
  { 
//_________________For Medicine set 1_____________________
    Firebase.getString(fbdo, "schedules/MS1");
    k = fbdo.to<String>();
    
    ind1 = k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS1[0]=M1; EEPROM.write(40,MS1[0]);
    ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS1[1]=M2; EEPROM.write(41,MS1[1]);
    ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS1[2]=M3; EEPROM.write(42,MS1[2]);
    Serial.print("MS1 - "); Serial.print(MS1[0]); Serial.print(" - "); Serial.print(MS1[1]); Serial.print(" - "); Serial.println(MS1[2]);
     
//_________________For Medicine set 2_____________________
   Firebase.getString(fbdo, "schedules/MS2");
    k = fbdo.to<String>();
    ind1 = k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS2[0]=M1; EEPROM.write(20,MS2[0]);
    ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS2[1]=M2; EEPROM.write(21,MS2[1]);
    ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS2[2]=M3; EEPROM.write(22,MS2[2]); 
    Serial.print("MS2 - "); Serial.print(MS2[0]); Serial.print(" - "); Serial.print(MS2[1]); Serial.print(" - "); Serial.println(MS2[2]);
     

//_________________For Medicine set 3_____________________
   Firebase.getString(fbdo, "schedules/MS3");
    k = fbdo.to<String>();
    ind1= k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS3[0]=M1; EEPROM.write(30,MS3[0]);
    ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS3[1]=M2; EEPROM.write(31,MS3[1]);
    ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS3[2]=M3; EEPROM.write(32,MS3[2]);
    Serial.print("MS3 - "); Serial.print(MS3[0]); Serial.print(" - "); Serial.print(MS3[1]); Serial.print(" - "); Serial.println(MS3[2]);
     
    //_________________For Alarm 1____________________
    Firebase.getString(fbdo, "schedules/T/AH1"); iv = fbdo.to<String>();
    AH1 = iv.toInt(); EEPROM.write(10,AH1);
    Firebase.getString(fbdo, "schedules/T/AM1"); iv = fbdo.to<String>();
    AM1 = iv.toInt(); EEPROM.write(11,AM1);
    if((AM1+rt) == 60) { AHR1=AH1+1; AMR1=0; }
    else               { AHR1=AH1; AMR1=AM1+rt; }
    
//_________________For Alarm 2____________________
    Firebase.getString(fbdo, "schedules/T/AH2"); iv = fbdo.to<String>();
    AH2 = iv.toInt(); EEPROM.write(12,AH2);
    Firebase.getString(fbdo, "schedules/T/AM2"); iv = fbdo.to<String>();
    AM2 = iv.toInt(); EEPROM.write(13,AM2);
    if((AM2+rt) == 60) { AHR2=AH2+1; AMR2=0; }
    else               { AHR2=AH2; AMR2=AM2+rt; }

//_________________For Alarm 3____________________
    Firebase.getString(fbdo, "schedules/T/AH3"); iv = fbdo.to<String>();
    AH3 = iv.toInt(); EEPROM.write(14,AH3);
    Firebase.getString(fbdo, "schedules/T/AM3"); iv = fbdo.to<String>();
    AM3 = iv.toInt(); EEPROM.write(15,AM3);
    if((AM3+rt) == 60) { AHR3=AH3+1; AMR3=0; }
    else               { AHR3=AH3; AMR3=AM3+rt; }
    
  }//fb ready
 }// have network

//_______________________________________no network_________________________________________
else{
  // LED KEDIP KEDIP
  // digitalWrite(MD3,1); delay(200);digitalWrite(MD3,0); delay(200); digitalWrite(MD3,1); delay(200);digitalWrite(MD3,0);
  Serial.println("No network detected");
  if(EEPROM.read(10) == 77 && EEPROM.read(11) == 77)   
   {
  //A1  
    AH1=00; AM1=00;  // sokal 8 ta
    if((AM1+rt) == 60) { AHR1=AH1+1; AMR1=0; }
    else               { AHR1=AH1; AMR1=AM1+rt; }
   }
   else 
   { 
    Serial.println("Reading from EEPROM 1");
    AH1 = EEPROM.read(10); AM1 = EEPROM.read(11); 
    if((AM1+rt) == 60) { AHR1=AH1+1; AMR1=0; }
    else               { AHR1=AH1; AMR1=AM1+rt; }
   }
   
  //A2
   if(EEPROM.read(12) == 77 && EEPROM.read(13) == 77)   
   {
    AH2=00; AM2=05; // dupur 1 ta
    if((AM2+rt) == 60) { AHR2=AH2+1; AMR2=0; }
    else               { AHR2=AH2; AMR2=AM2+rt; }
   }
   else 
   { 
    Serial.println("Reading from EEPROM 2");
    AH2 = EEPROM.read(12); AM2 = EEPROM.read(13); 
    if((AM2+rt) == 60) { AHR2=AH2+1; AMR2=0; }
    else               { AHR2=AH2; AMR2=AM2+rt; }
   }
  
  //A3
   if(EEPROM.read(14) == 77 && EEPROM.read(15) == 77)   
   { 
    AH3=00; AM3=10; // rat 9 ta
    if((AM3+rt) == 60) { AHR3=AH3+1; AMR3=0; }
    else               { AHR3=AH3; AMR3=AM3+rt; }
   }
   else 
   { 
    Serial.println("Reading from EEPROM 3");
    AH3 = EEPROM.read(14); AM3 = EEPROM.read(15); 
    if((AM3+rt) == 60) { AHR3=AH3+1; AMR3=0; }
    else               { AHR3=AH3; AMR3=AM3+rt; }
   }
   
  //________________________________Medicine set______________________________________ 
  
  //MS1
   if(EEPROM.read(40) == 77 && EEPROM.read(41) == 77 && EEPROM.read(42) == 77)  {}
   else { Serial.println("Reading from EEPROM 4"); MS1[0]= EEPROM.read(40); MS1[1]= EEPROM.read(41); MS1[2]= EEPROM.read(42); }
  //MS2
   if(EEPROM.read(20) == 77 && EEPROM.read(21) == 77 && EEPROM.read(22) == 77)  {}
   else { Serial.println("Reading from EEPROM 5"); MS2[0]= EEPROM.read(20); MS2[1]= EEPROM.read(21); MS2[2]= EEPROM.read(22); } 
  //MS3
   if(EEPROM.read(30) == 77 && EEPROM.read(31) == 77 && EEPROM.read(32) == 77)  {}
   else { Serial.println("Reading from EEPROM 6"); MS3[0]= EEPROM.read(30); MS3[1]= EEPROM.read(31); MS3[2]= EEPROM.read(32); } 
  }// no net end
EEPROM.commit();

  Serial.print("Alarm1-  "); Serial.print(AH1); Serial.print(":"); Serial.println(AM1);
  Serial.print("Alarm2-  "); Serial.print(AH2); Serial.print(":"); Serial.println(AM2);
  Serial.print("Alarm3-  "); Serial.print(AH3); Serial.print(":"); Serial.println(AM3);
  

  
  Serial.print("MS1 - "); Serial.print(MS1[0]); Serial.print(" - "); Serial.print(MS1[1]); Serial.print(" - "); Serial.println(MS1[2]);
  Serial.print("MS2 - "); Serial.print(MS2[0]); Serial.print(" - "); Serial.print(MS2[1]); Serial.print(" - "); Serial.println(MS2[2]);
  Serial.print("MS3 - "); Serial.print(MS3[0]); Serial.print(" - "); Serial.print(MS3[1]); Serial.print(" - "); Serial.println(MS3[2]);

Serial.print(EEPROM.read(10)); Serial.print(":"); Serial.println(EEPROM.read(11));
Serial.print(EEPROM.read(12)); Serial.print(":"); Serial.println(EEPROM.read(13));
Serial.print(EEPROM.read(14)); Serial.print(":"); Serial.println(EEPROM.read(15));

Serial.print(EEPROM.read(40)); Serial.print(" - "); Serial.print(EEPROM.read(41)); Serial.print(" - "); Serial.println(EEPROM.read(42));
Serial.print(EEPROM.read(20)); Serial.print(" - "); Serial.print(EEPROM.read(21)); Serial.print(" - "); Serial.println(EEPROM.read(22));
Serial.print(EEPROM.read(30)); Serial.print(" - "); Serial.print(EEPROM.read(31)); Serial.print(" - "); Serial.println(EEPROM.read(32));

   Serial.print("Repeat Alarm1-  "); Serial.print(AHR1); Serial.print(":"); Serial.println(AMR1);
   Serial.print("Repeat Alarm2-  "); Serial.print(AHR2); Serial.print(":"); Serial.println(AMR2);
   Serial.print("Repeat Alarm3-  "); Serial.print(AHR3); Serial.print(":"); Serial.println(AMR3);
 
//  Serial.println("wait for some moment...");
//  delay(5000);
 
}//setup

void loop(){}

// void loop() 
// {
//      RtcDateTime currentTime = Rtc.GetDateTime();    //get the time from the RTC
 
//    Years= currentTime.Year(); Months= currentTime.Month(); Days= currentTime.Day();
//    Hours= currentTime.Hour(); Min= currentTime.Minute(); Sec= currentTime.Second();

//     if (Hours == AH1 && Min == AM1 && AV1 == 0) // Morning alarm1
//     {
//       playBuzzer();
//       AV1 = 1;
//     }
    
//    else if (Hours == AH2 && Min == AM2 && AV2 == 0) //Day Alarm
//     {
//         playBuzzer();
//         AV2 = 1;
//     }

//    else if (Hours == AH3 && Min == AM3 && AV3 == 0) //Night Alarm 
//     {
//       playBuzzer();
//       AV3 = 1;

//     }
// //_________________________________________________Repeat alarm____________________________________________________

//  else if (Hours == AHR1 && Min == AMR1 && AVR1 == 0) // Repeat Alarm Morning Alarm
//     {
//       playBuzzer();
//       AVR1 = 1;
//     }
    
//    else if (Hours == AHR2 && Min == AMR2 && AVR2 == 0) // Repeat Alarm Day Alarm
//     {
//       playBuzzer();
//       AVR2 = 1;
//     }

//    else if (Hours == AHR3 && Min == AMR3 && AVR3 == 0) // Repeat Alarm Night Alarm
//     {
//       playBuzzer();
//       AVR3 = 1;
//     }

    
// }
    //repeating alarm disini terjadi apabila tidak ada tanggapan atau action dalam melakukan konfirmasi konsumsi obat, jika pengguna sudah klik btn 
    //maka pengguna tidak akan mendengan buzzer lagi 

    //tapi di kode ini masih dalam bentuk delay aja blm ada konfirmasi dr pengguna. 

//_____________________________________________________day reset & light off_______________________________________
    
//    if(Days != pDay)
//    {
//      AV1=0; AV2=0; AV3=0;
//      pDay=Days; 
//    }
//    else{}
//    if ( millis()-lm>= HT)
//    {
        //lcd.setCursor(0,0); lcd.print("RTC ERROR!")
         //lcd.setCursor(0,1); lcd.print("Please check RTC and current dates!")
//    }
//    else{}

// }// loop

//__________________________________________void functions________________________________
// void work()
// {
  
//   if (W == 0){}
//   else if (W == 01)
//   { digitalWrite(MD1,onv); /*myDFPlayer.play(6) ganti jadi buzzer*/; 
//       lcd.setCursor(0,2);
//       lcd.print("Medicine no. 1");
//       delay(1500); 
//       Serial.println(" Medicine1");
//       lcd.clear();
//    }
//   else if(W == 02)
//   {   lcd.setCursor(0,2);
//       lcd.print("Medicine no. 2");
//       delay(1500); 
//       Serial.println(" Medicine2");
//       lcd.clear();
//   }
//   else if(W == 03)
//   { 
//       lcd.setCursor(0,2);
//       lcd.print("Medicine no. 3");
//       delay(1500); 
//       Serial.println(" Medicine3");
//       lcd.clear();
//     }
// }


//HandlingPin Update Data from firebase 
void handlePinTime(stream.dataPath.c_str(), stream.value.c_str()){
  if(path == "/schedules/MS1"){
    if()
  }
}

void handlePin(stream.dataPath.c_str(), stream.value.c_str()){
  
}

void successtone() { // bunyi yg nandain kalo konfigurasi RTC OK
  tone(buzzer, 400, 100); // Play first "teng"
  delay(200); // Pause between "teng" sounds
  tone(buzzer, 600, 100); // Play second "teng"
  delay(200); // Pause after second "teng"
  noTone(buzzer); // Turn off buzzer
}

void playBuzzer() {
  tone(buzzer, 1000); // Nada dengan frekuensi 1000 Hz
  delay(100); // Waktu delay antara nada
  noTone(buzzer); // Matikan nada
  delay(100); // Waktu delay antara nada
}

// void playAlarm(){

// }

// void checkstatusbtn(){

// }



// void triggerAlarm(){

// }


// void buZing()
// {
//   Serial.println("Medicine Alert");
//   digitalWrite(buzzer, HIGH);
//   delay(2050);
//   myDFPlayer.play(1); // Tring Medicine Alert It's the time for
//   delay(4050);
// }

// void lastKaj()
// {
//   myDFPlayer.play(2); //don't forget to take your medicine 
//   delay(3200);
//   Serial.println();
//   Serial.println("-------------");
//   lm=millis();
// }

// void lastKaj1()
// {
//   myDFPlayer.play(3); // This is the last reminder for the medicine don't forget to take your medicine 
//   delay(6050);
//   Serial.println();
//   Serial.println("F-------------");
//   lm=millis();
// }
