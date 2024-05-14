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
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define WIFI_SSID "sira" 
#define WIFI_PASSWORD "otsira116" 
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
FirebaseData stream, stream_M1, stream_M2, stream_M3, stream_AH1, stream_AH2, stream_AH3, stream_AM1, stream_AM2, stream_AM3, stream_qty1, stream_qty2, stream_qty3, stream_med1, stream_med2, stream_med3;
FirebaseAuth auth;
FirebaseConfig config;

int AV1, AV2, AV3, pDay; //menandakan alarm sudah berbunyi dg nilai 0 dan 1 (sudah menyala) jika ada network wifii realtime firebase 
int AVR1, AVR2, AVR3;
int Years, Months, Days, Hours, Min, Sec,timeStamp, date;

String k, m1, m2, m3, iv; 
// k utk simpan nilai string dr firebase sblm di convert ke tipe data array , letak obat
//iv utk nilai string firebase jam alarm ke int 
//m1,m2,m3 utk mengekstrak nilai indeks ke-n dari array yg ada di K

int MS1[] = {1,2,3};
int MS2[] = {1,2,3};
int MS3[] = {1,2,3};

/* inisialisasi awal variabel dengan nilai default 
sebagai contoh atau sebagai tanda untuk menjelaskan 
format data yang akan diterima dari Firebase.*/
String MED1, MED2, MED3;
int Q1, Q2, Q3;
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
  delay(10000);
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
    delay(5000);
    successtone(); 
    lcd.clear();
  } 

 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  lcd.setCursor(1,1);
  lcd.print("Connecting to Wi-Fi");
  delay(8000);
  lcd.clear();
  delay(5000);
  
  if (WiFi.status() == WL_CONNECTED){

  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  lcd.setCursor(3,1);
  lcd.print("WiFi Connected ");
  delay(8000);
  lcd.clear();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024 );
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  delay(2000);

  stream.keepAlive(5, 5, 1);

  //stream 

    //STREAM NAMA OBAT 
      if (!Firebase.beginStream(stream_med1, "/meds/1/medicine")) {
        Serial.printf("stream namaobat1 begin error, %s\n\n", stream_med1.errorReason().c_str());}

      if (!Firebase.beginStream(stream_med2, "/meds/2/medicine")) {
        Serial.printf("stream namaobat2 begin error, %s\n\n", stream_med2.errorReason().c_str());}

      if (!Firebase.beginStream(stream_med3, "/meds/3/medicine")) {
        Serial.printf("stream namaobat3 begin error, %s\n\n", stream_med3.errorReason().c_str());}

   //STREAM QUANTITAS OBAT
      if (!Firebase.beginStream(stream_qty1, "/meds/1/quantity")) {
      Serial.printf("stream jumlahobat1 begin error, %s\n\n", stream_qty1.errorReason().c_str());}

      if (!Firebase.beginStream(stream_qty2, "/meds/2/quantity")) {
      Serial.printf("stream jumlahobat2 begin error, %s\n\n", stream_qty2.errorReason().c_str());}

      if (!Firebase.beginStream(stream_qty3, "/meds/3/quantity")) {
      Serial.printf("stream jumlahobat3 begin error, %s\n\n", stream_qty3.errorReason().c_str());}


  //stream set obat
  if (!Firebase.beginStream(stream_M1, "/schedules/MS1")) {
    Serial.printf("stream M1 begin error, %s\n\n", stream_M1.errorReason().c_str());}
  //stream M2
  if (!Firebase.beginStream(stream_M2, "/schedules/MS2")) {
      Serial.printf("stream M2 begin error, %s\n\n", stream_M2.errorReason().c_str());}
  //stream M3
  if (!Firebase.beginStream(stream_M3, "/schedules/MS3")) {
      Serial.printf("stream M3 begin error, %s\n\n", stream_M3.errorReason().c_str());}
  //stream AH1
  if (!Firebase.beginStream(stream_AH1, "/schedules/T/AH1")) {
      Serial.printf("stream AH1 begin error, %s\n\n", stream_AH1.errorReason().c_str());}
  //stream AH2
  if (!Firebase.beginStream(stream_AH2, "/schedules/T/AH2")) {
      Serial.printf("stream AH2 begin error, %s\n\n", stream_AH2.errorReason().c_str());}
  //stream AH3
  if (!Firebase.beginStream(stream_AH3, "/schedules/T/AH3")) {
      Serial.printf("stream AH3 begin error, %s\n\n", stream_AH3.errorReason().c_str());}
  //stream AM1
  if (!Firebase.beginStream(stream_AM1, "/schedules/T/AM1")) {
      Serial.printf("stream AM1 begin error, %s\n\n", stream_AM1.errorReason().c_str());}
  //stream AM2
  if (!Firebase.beginStream(stream_AM2, "/schedules/T/AM2")) {
      Serial.printf("stream AM2 begin error, %s\n\n", stream_AM2.errorReason().c_str());}
  //stream AM3
  if (!Firebase.beginStream(stream_AM3, "/schedules/T/AM3")) {
      Serial.printf("stream AM3 begin error, %s\n\n", stream_AM3.errorReason().c_str());}

  //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
  config.timeout.socketConnection = 30 * 1000;
  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;
  

 }// have network
}//setup

void loop() 
{
        if (Firebase.ready() && WiFi.status() == WL_CONNECTED) 
          { 

                        //__MEDICINE NAME ________________________________
             if(!Firebase.readStream(stream_med1)){
                Serial.printf("stream  namaobat1 read error, %s\n\n", stream_med1.errorReason().c_str());
              } 
              if(stream_med1.streamAvailable()){
                if(stream_med1.dataType() == "string"){
                  MED1 = stream_med1.to<String>();
                }
              }

              if(!Firebase.readStream(stream_med2)){
                Serial.printf("stream  namaobat2 read error, %s\n\n", stream_med2.errorReason().c_str());
              } 
              if(stream_med2.streamAvailable()){
                if(stream_med2.dataType() == "string"){
                  MED2 = stream_med2.to<String>();
                }
              }

             if(!Firebase.readStream(stream_med3)){
                Serial.printf("stream  namaobat3 read error, %s\n\n", stream_med3.errorReason().c_str());
              } 
              if(stream_med3.streamAvailable()){
                if(stream_med3.dataType() == "string"){
                  MED3 = stream_med3.to<String>();
                }
              }

             //_MEDICINE QUANTITY____________________________

              if(!Firebase.readStream(stream_qty1)){
                Serial.printf("stream  jumlahobat1 read error, %s\n\n", stream_qty1.errorReason().c_str());
              } 
              if(stream_qty1.streamAvailable()){
                if(stream_qty1.dataType() == "int"){
                  Q1 = stream_qty1.to<int>(); 
                }
              }

              if(!Firebase.readStream(stream_qty2)){
                Serial.printf("stream  jumlahobat2 read error, %s\n\n", stream_qty2.errorReason().c_str());
              } 
              if(stream_qty2.streamAvailable()){
                if(stream_qty2.dataType() == "int"){
                  Q2 = stream_qty2.to<int>();
                }
              }

             if(!Firebase.readStream(stream_qty3)){
                Serial.printf("stream  jumlahobat3 read error, %s\n\n", stream_qty3.errorReason().c_str());
              } 
              if(stream_qty3.streamAvailable()){
                if(stream_qty3.dataType() == "int"){
                  Q3 = stream_qty3.to<int>();
                }
              }

//_______________________________________________________MEDICINE SET 1__________________________________________________________________________________________
              if(!Firebase.readStream(stream_M1)){
                Serial.printf("stream  M1 read error, %s\n\n", stream.errorReason().c_str());
              } 
              if(stream_M1.streamAvailable()){
                if(stream_M1.dataType() == "string"){
                  k = stream_M1.to<String>();
                  ind1 = k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS1[0]=M1; 
                  ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS1[1]=M2; 
                  ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS1[2]=M3; 
                }
              }

//_______________________________________________________MEDICINE SET 2__________________________________________________________________________________________
              if(!Firebase.readStream(stream_M2)){
                Serial.printf("stream  M2 read error, %s\n\n", stream.errorReason().c_str());
              } 
              if(stream_M2.streamAvailable()){
                if(stream_M2.dataType() == "string"){
                  k = stream_M2.to<String>();
                  ind1 = k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS2[0]=M1; 
                  ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS2[1]=M2; 
                  ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS2[2]=M3; 
                }
              }

//_______________________________________________________MEDICINE SET 3__________________________________________________________________________________________
                if(!Firebase.readStream(stream_M3)){
                  Serial.printf("stream  M3 read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_M3.streamAvailable()){
                  if(stream_M3.dataType() == "string"){
                   k = stream_M3.to<String>();
                  ind1= k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS3[0]=M1; 
                  ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS3[1]=M2; 
                  ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS3[2]=M3; 
                  }
                }

//_________________________________________________ALARM1_____________________________________________________________
//_______________________________________________AH1 DAN AM 1_________________________________________________________
                if(!Firebase.readStream(stream_AH1)){
                  Serial.printf("stream  AH1 read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AH1.streamAvailable()){
                  if(stream_AH1.dataType() == "string"){
                    iv = stream_AH1.to<String>();
                    AH1 = iv.toInt(); 
                  }
                }

                if(!Firebase.readStream(stream_AM1)){
                  Serial.printf("stream  AM1 read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AM1.streamAvailable()){
                  if(stream_AM1.dataType() == "string"){
                    iv = stream_AM1.to<String>();
                    AM1 = iv.toInt(); 
                  }
                }

//_________________________________________________ALARM2_____________________________________________________________
//_______________________________________________AH2 DAN AM 2_________________________________________________________
                if(!Firebase.readStream(stream_AH2)){
                  Serial.printf("stream  AH2   read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AH2.streamAvailable()){
                  if(stream_AH2.dataType() == "string"){
                    iv = stream_AH2.to<String>();
                    AH2 = iv.toInt();  
                  }
                }

                if(!Firebase.readStream(stream_AM2)){
                  Serial.printf("stream  AM2 read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AM2.streamAvailable()){
                  if(stream_AM2.dataType() == "string"){
                    iv = stream_AM2.to<String>();
                    AM2 = iv.toInt();  
                  }
                }

//_________________________________________________ALARM3_____________________________________________________________
//_______________________________________________AH3 DAN AM 3_________________________________________________________
                if(!Firebase.readStream(stream_AH3)){
                  Serial.printf("stream  AH3   read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AH3.streamAvailable()){
                  if(stream_AH3.dataType() == "string"){
                    iv = stream_AH3.to<String>();
                    AH3 = iv.toInt(); 
                  }
                }

                if(!Firebase.readStream(stream_AM3)){
                  Serial.printf("stream  AM3 read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AM3.streamAvailable()){
                  if(stream_AM3.dataType() == "string"){
                    iv = stream_AM3.to<String>();
                    AM3 = iv.toInt(); 
                  }
                }
          }//fb read
//_______________________________________no network_________________________________________
        else{
          // LED KEDIP KEDIP
          // LCD PRINT 
            lcd.setCursor(1,0);
            lcd.print("No Network Detected ");
            lcd.setCursor(0,1);
            lcd.print("Please Restart ESP32");
            delay(8000);
            lcd.clear();
            Serial.println("No network detected");
            }
        

        Serial.print("Alarm1-  "); Serial.print(AH1); Serial.print(":"); Serial.println(AM1);
        Serial.print("Alarm2-  "); Serial.print(AH2); Serial.print(":"); Serial.println(AM2);
        Serial.print("Alarm3-  "); Serial.print(AH3); Serial.print(":"); Serial.println(AM3);   
            
        Serial.print("MS1 - "); Serial.print(MS1[0]); Serial.print(" - "); Serial.print(MS1[1]); Serial.print(" - "); Serial.println(MS1[2]);
        Serial.print("MS2 - "); Serial.print(MS2[0]); Serial.print(" - "); Serial.print(MS2[1]); Serial.print(" - "); Serial.println(MS2[2]);
        Serial.print("MS3 - "); Serial.print(MS3[0]); Serial.print(" - "); Serial.print(MS3[1]); Serial.print(" - "); Serial.println(MS3[2]);

        Serial.print("Q1 = "); Serial.println(Q1); 
        Serial.print("Q2 = "); Serial.println(Q2); 
        Serial.print("Q3 = "); Serial.println(Q3);

        Serial.print("MS[0] = "); Serial.println(MED1); 
        Serial.print("MS[1] = "); Serial.println(MED2); 
        Serial.print("MS[2] = "); Serial.println(MED3);
 

        Serial.println("wait for some moment...");

        delay(5000);


        RtcDateTime currentTime = Rtc.GetDateTime();    //get the time from the RTC
      
        Years= currentTime.Year(); Months= currentTime.Month(); Days= currentTime.Day();
        Hours= currentTime.Hour(); Min= currentTime.Minute(); Sec= currentTime.Second();

          if (Hours == AH1 && Min == AM1 && AV1 == 0) // Morning alarm1
          {
            playBuzzer(); //BUAT FUNGSI BUZZER DGN PUSH BTN NYA DISINI
            AV1 = 1;
          }
          
        else if (Hours == AH2 && Min == AM2 && AV2 == 0) //Day Alarm
          {
              playBuzzer(); //BUAT FUNGSI BUZZER DGN PUSH BTN NYA DISINI
              AV2 = 1;
          }

        else if (Hours == AH3 && Min == AM3 && AV3 == 0) //Night Alarm 
          {
            playBuzzer(); //BUAT FUNGSI BUZZER DGN PUSH BTN NYA DISINI
            AV3 = 1;

          }
// //_________________________________________________Repeat alarm____________________________________________________

    //repeating alarm disini terjadi apabila tidak ada tanggapan atau action dalam melakukan konfirmasi konsumsi obat, jika pengguna sudah klik btn 
    //maka pengguna tidak akan mendengan buzzer lagi 

    //tapi di kode ini masih dalam bentuk delay aja blm ada konfirmasi dr pengguna. 

//_____________________________________________________day reset & light off_______________________________________
    
   if(Days != pDay)
   {
     AV1=0; AV2=0; AV3=0;
     pDay=Days; 
   }
   else{}
  //  if ( millis()-lm>= HT) times for LED GLOW AFTER ALARM 
  //  {
  //       lcd.setCursor(0,0); lcd.print("RESET DATES ERROR!");
  //        lcd.setCursor(0,1); lcd.print("Please check RTC and current dates!");
  //  }
  //  else{}

} //loop


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

void successtone() { // bunyi yg nandain kalo konfigurasi RTC OK
  tone(buzzer, 400, 100); // Play first "teng"
  delay(200); // Pause between "teng" sounds
  tone(buzzer, 600, 100); // Play second "teng"
  delay(200); // Pause after second "teng"
  noTone(buzzer); // Turn off buzzer
}

void playBuzzer() {
  tone(buzzer, 1000); // Nada dengan frekuensi 1000 Hz
  delay(1000); // Waktu delay antara nada
  tone(buzzer, 1000);
  delay(1000); // Waktu delay antara nada
  tone(buzzer, 1000);
  delay(1000); // Waktu delay antara nada
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