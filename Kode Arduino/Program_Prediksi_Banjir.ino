#include <UltraDistSensor.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include "CTBot.h"
#include "FirebaseESP8266.h"  // Install Firebase ESP8266 library

FirebaseData firebaseData;
FirebaseJson json;
CTBot myBot;
TBMessage msg;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Ultrasonik
#define trigPin 2 //D4
#define echoPin 0 //D3
long durasi, jarak, tinggi; //inisial input tinggi
//Waterflow
int x;
int y;
int state=0;
float waktu = 0;
float frekuensi = 0;
float air = 0; //inisial input debit air
float total = 0;
float liter = 0;
float wkt = 0; //inisial input waktu kenaikan air
const int flow = 13; //D7
//Buzzer
const int pinBuzzer = 15; //D8
//LED
const int ledMerah = 16; //D0 merah
const int ledKuning = 14; //D5 kuning
const int ledHijau = 12; //D6 hijau
//firebase
#define firebase_host "sistemprediksibanjir-91724-default-rtdb.firebaseio.com/"
#define firebase_auth "WfryA1QgGVyO492wAGeqBC9zI0QsjOxoqMR9vaks"
//chatbot Tele
String ssid = "vivo 1919"; //"Sudarto";     // Sesuikan dengan nama wifi anda
String pass = "123123123"; //"sudarto123"; // sesuaikan password wifi
String token = "1474868946:AAG1VrQ-M9fTkYoLxKVw0vMBfYQ7kdM7QBo"; // token bot telegram yang telah dibuat
const int id = 1314548670;
String pesan = msg.text;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledKuning, OUTPUT);
  pinMode(ledHijau, OUTPUT);
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Si_Pre_Banjir");
  lcd.setCursor(0,1);
  lcd.print("****************");
  delay(2000);
  pinMode(flow,INPUT);
  //koneksi ke WIFI
  WiFi.begin(ssid, pass);
  Serial.print("Menghubungkan...");
  //selama tidak terkoneksi
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  //apabila terkoneksi
  Serial.println();
  Serial.print("Terkoneksi ");
  Serial.println(ssid);
  Serial.println(WiFi.localIP());
  Firebase.begin(firebase_host, firebase_auth);
  Firebase.reconnectWiFi(true);
  
  // Chatbot Telegram
  Serial.println("Starting TelegramBot...");
  //koneksi ke wifi
  myBot.wifiConnect(ssid, pass);
  //set token telegram
  myBot.setTelegramToken(token);
  // check if all things are ok
  if (myBot.testConnection()){
    Serial.println("\nKoneksi OK");
  }
  else{
    Serial.println("\nKoneksi Tidak OK");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  durasi = pulseIn(echoPin, HIGH);
  jarak = (durasi/2) / 29.1;
  tinggi = 50-jarak; //tinggi air menggunakan wadah setinggi 50cm
  //waterflow
  x = pulseIn(flow, HIGH);
  y = pulseIn(flow, LOW);
  waktu = x + y;
  frekuensi = 1000000/waktu;
  air = frekuensi/7.5; //menghitung debit air(Liter/Menit)
  liter = air/60; //menghitung total debit air(Liter)
  
  //baca ketinggian & debit air
  if((jarak>30)&&(frekuensi >= 0)){
    Serial.print("***Sistem Prediksi Banjir***");
    Serial.println(" ");
    if(isinf(frekuensi)){
      serialMonitor1();
      Serial.print("Kondisi Sungai AMAN!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Siaga dalam waktu(menit) = ");
      wkt = (25*25*(20-tinggi))/air/1000; 
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:0.0 | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: AMAN");
    }else{
      serialMonitor2();
      Serial.print("Kondisi Sungai AMAN!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Siaga dalam waktu(menit) = ");
      wkt = (25*25*(20-tinggi))/air/1000; 
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:");
      lcd.print(air);
      lcd.print(" | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: AMAN");
    }
    delay(0);
    //led
    setColor(0, 255, 0); // setting warna hijau
    sensorUpdate();
    sensorUpdate2();
    sensorUpdate3();
    chatTeleAman();
  }
  else if((jarak>20) & (jarak<=30)){
    Serial.print("***Sistem Prediksi Banjir***");
    Serial.println(" ");
    
    if(isinf(frekuensi)){
      serialMonitor1();
      Serial.print("Kondisi Sungai SIAGA!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Bahaya dalam waktu(menit) = ");
      wkt = (25*25*(30-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:0.0 | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: SIAGA");
    }else{
      serialMonitor2();
      Serial.print("Kondisi Sungai SIAGA!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Bahaya dalam waktu(menit) = ");
      wkt = (25*25*(30-tinggi))/air/1000; 
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:");
      lcd.print(air);
      lcd.print(" | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: SIAGA");
    }
    delay(0);
    //led
    setColor(255, 255, 0); // setting warna kuning
    sensorUpdate();
    sensorUpdate2();
    sensorUpdate3();
    //chat Telegram
    chatTeleSiaga();
  }
  else if((jarak>10) & (jarak<=20)){
    Serial.print("***Sistem Prediksi Banjir***");
    Serial.println(" ");
    
    if(isinf(frekuensi)){
      serialMonitor1();
      Serial.print("Kondisi Sungai BAHAYA!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Waspada dalam waktu(menit) = ");
      wkt = (25*25*(40-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:0.0 | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: BAHAYA");
    }else{
      serialMonitor2();
      Serial.print("Kondisi Sungai BAHAYA!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Waspada dalam waktu(menit) = ");
      wkt = (25*25*(40-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:");
      lcd.print(air);
      lcd.print(" | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: BAHAYA");
    }
    delay(0);
    //led
    setColor(255, 0, 0); // setting warna merah
    sensorUpdate();
    sensorUpdate2();
    sensorUpdate3();
    //nada panjang
    alarmPanjang();
    //chat Telegram 
    chatTeleBahaya();
  }
 
  else if((jarak>5) & (jarak<=10)){
    Serial.print("***Sistem Prediksi Banjir***");
    Serial.println(" ");
    
    if(isinf(frekuensi)){
      serialMonitor1();
      Serial.print("Kondisi Sungai WASPADA!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Banjir dalam waktu(menit) = ");
      wkt = (25*25*(45-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:0.0 | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: WASPADA");
    }else{
      serialMonitor2();
      Serial.print("Kondisi Sungai WASPADA!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi status Banjir dalam waktu(menit) = ");
      wkt = (25*25*(45-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:");
      lcd.print(air);
      lcd.print(" | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: WASPADA");
    }
    delay(0);
    sensorUpdate();
    sensorUpdate2();
    sensorUpdate3();
    //nada sedang
    alarmSedang();
    //chat Telegram
    chatTeleWaspada();
  }
  
  else if((jarak>=0) & (jarak<=5)){
    Serial.print("***Sistem Prediksi Banjir***");
    Serial.println(" ");
    
    if(isinf(frekuensi)){
      serialMonitor1();
      Serial.print("Kondisi Sungai BANJIR!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi meningkat dalam waktu(menit) = "); 
      wkt = (25*25*(50-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:0.0 | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: BANJIR!!");
    }else{
      serialMonitor2();
      Serial.print("Kondisi Sungai BANJIR!!! ");
      Serial.println(" ");
      Serial.print("Air berpotensi meningkat dalam waktu(menit) = ");
      wkt = (25*25*(50-tinggi))/air/1000;
      Serial.println(wkt);
      Serial.println(" ");
      Serial.println(" ");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("D:");
      lcd.print(air);
      lcd.print(" | T:");
      lcd.print(tinggi);
      lcd.setCursor(0,1);
      lcd.print("STATUS: BANJIR!!");
    }
    delay(0);
    sensorUpdate();
    sensorUpdate2();
    sensorUpdate3();
    //nada pendek
    alarmPendek();
    //chat telegram
    chatTeleBanjir();
  }
}

void chatTeleAman(){
  //menghitung volume wadah uji(menit) selang 5cm: volume(25*25*(20-tinggi))/debit/1000(cm^3-->dm^3 = Liter)
  wkt = (25*25*(20-tinggi))/air/1000;  
  if(jarak<35 & jarak>=30){ //mengirim chatbot pada saat ketinggian 15-20
    String t = String (tinggi);
    String a = String (air);
    String w = String (wkt);
    myBot.sendMessage(id, "Kondisi Sungai AMAN!!!");
    myBot.sendMessage(id, "Ketinggian Air : " + t + " Centimeters\n"
                         +"Debit Air : " + a + " Liter/Menit\n"
                         +"Air berpotensi status Siaga dalam waktu : " + w + " Menit");
  }
}
void chatTeleSiaga(){
  //menghitung volume wadah uji(menit) selang 5cm: volume(25*25*(30-tinggi))/debit/1000(cm^3-->dm^3 = Liter) 
  wkt = (25*25*(30-tinggi))/air/1000; 
  if(jarak<25 & jarak>=20){ //mengirim chatbot pada saat ketinggian 25-30
    String t = String (tinggi);
    String a = String (air);
    String w = String (wkt);
    myBot.sendMessage(id, "Kondisi Sungai SIAGA!!! Harap bersiaga!");
    myBot.sendMessage(id, "Ketinggian Air : " + t + " Centimeters\n"
                         +"Debit Air : " + a + " Liter/Menit\n"
                         +"Air berpotensi status Bahaya dalam waktu : " + w + " Menit");
  }
}
void chatTeleBahaya(){
  //menghitung volume wadah uji(menit) selang 5cm: volume(25*25*(40-tinggi))/debit/1000(cm^3-->dm^3 = Liter) 
  wkt = (25*25*(40-tinggi))/air/1000; 
  if(jarak<15 & jarak>=10){ //mengirim chatbot pada saat ketinggian 35-40
    String t = String (tinggi);
    String a = String (air);
    String w = String (wkt);
    myBot.sendMessage(id, "Kondisi Sungai BAHAYA!!! Harap berwaspada!");
    myBot.sendMessage(id, "Ketinggian Air : " + t + " Centimeters\n"
                         +"Debit Air : " + a + " Liter/Menit\n"
                         +"Air berpotensi status Waspada dalam waktu : " + w + " Menit");
  }
}
void chatTeleWaspada(){
  //menghitung volume wadah uji(menit) selang 5cm: volume(25*25*(45-tinggi))/debit/1000(cm^3-->dm^3 = Liter) 
  wkt = (25*25*(45-tinggi))/air/1000; 
  if(jarak<10){ //mengirim chatbot pada saat ketinggian 40-45
    String t = String (tinggi);
    String a = String (air);
    String w = String (wkt);
    myBot.sendMessage(id, "Kondisi Sungai WASPADA!!! Harap bersiap-siap mengungsi ke tempat yang lebih AMAN!!!");
    myBot.sendMessage(id, "Ketinggian Air : " + t + " Centimeters\n"
                         +"Debit Air : " + a + " Liter/Menit\n"
                         +"Air berpotensi Banjir dalam waktu : " + w + " Menit");
  }
}
void chatTeleBanjir(){
  //menghitung volume wadah uji(menit) selang 5cm: volume(25*25*(50-tinggi))/debit/1000(cm^3-->dm^3 = Liter) 
  wkt = (25*25*(50-tinggi))/air/1000; 
  //mengirim chatbot pada saat ketinggian 45-50
  String t = String (tinggi);
  String a = String (air);
  String w = String (wkt);
  myBot.sendMessage(id, "Kondisi Sungai BANJIR!!! Hati-hati saat mengungsi!!!");
  myBot.sendMessage(id, "Ketinggian Air : " + t + " Centimeters\n"
                       +"Debit Air : " + a + " Liter/Menit\n"
                       +"Air berpotensi meningkat 5cm lagi dalam waktu : " + w + " Menit");
}
void serialMonitor1(){
  Serial.print("Jarak Air (cm) = ");
  Serial.print(jarak);
  Serial.println(" ");
  Serial.print("Tinggi Air (cm) = ");
  Serial.print(tinggi);
  Serial.println(" ");
  Serial.print("Debit Air (Liter/Menit) = 0.0");
  Serial.println(" ");
  Serial.print("Total Debit Air (Liter/Menit) = ");
  Serial.print(total);
  Serial.println(" ");
}
void serialMonitor2(){
  total = total + liter; 
  Serial.print("Frekuensi (Hz) = ");
  Serial.print(frekuensi);
  Serial.println(" ");
  Serial.print("Jarak Air (cm) = ");
  Serial.print(jarak);
  Serial.println(" ");
  Serial.print("Tinggi Air (cm) = ");
  Serial.print(tinggi);
  Serial.println(" ");
  Serial.print("Debit Air (Liter/Menit) = ");
  Serial.print(air);
  Serial.println(" ");
  Serial.print("Total Debit Air (Liter/Menit) = ");
  Serial.print(total);
  Serial.println(" ");
}
void sensorUpdate(){ //Kirim data Tinggi air ke firebase
  String jrk = String(tinggi);
  Serial.println(jrk);   // print out the value you read
  delay(1000);
  if (Firebase.setString(firebaseData, "/Ultrasonik/Tinggi air", jrk))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
void sensorUpdate2(){ //Kirim data Debit air ke firebase
  String dbt = String(air);
  Serial.println(dbt);   // print out the value you read
  delay(1000);
  if (Firebase.setString(firebaseData, "/Water Flow/Debit air", dbt))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
void sensorUpdate3(){ //Kirim data Prediksi waktu ke firebase
  String w = String(wkt);
  Serial.println(w);   // print out the value you read
  delay(1000);
  if (Firebase.setString(firebaseData, "/Prediksi/Waktu air naik ke level berikutnya", w))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
void alarmPendek(){ //Banjir
  for(int i=0;i<10;i++){
    //led
    setColor(255, 0, 0); // setting warna merah
    digitalWrite(pinBuzzer, HIGH);
    delay(200);
    setColor(0, 0, 0);
    digitalWrite(pinBuzzer, LOW);
    delay(200);
  }
}
void alarmSedang(){ //Waspada
  for(int i=0;i<10;i++){
    setColor(255, 0, 0); // setting warna merah
    digitalWrite(pinBuzzer, HIGH);
    delay(500);
    setColor(0, 0, 0);
    digitalWrite(pinBuzzer, LOW);
    delay(500);
  }
}
void alarmPanjang(){ //Bahaya
  for(int i=0;i<10;i++){
    digitalWrite(pinBuzzer, HIGH);
    delay(1000);
    digitalWrite(pinBuzzer, LOW);
    delay(1000);
  }
}
void setColor(int merah, int kuning, int hijau){
  analogWrite(ledMerah, merah);      //menulis data analog ke pin LED merah
  analogWrite(ledKuning, kuning);  //menulis data analog ke pin LED kuning
  analogWrite(ledHijau, hijau);    //menulis data analog ke pin LED hijau
}
