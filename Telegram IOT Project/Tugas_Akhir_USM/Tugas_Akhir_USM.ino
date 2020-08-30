#include <LiquidCrystal.h>
#include <MsTimer2.h>
#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
Servo servo1, servo2;
DHT dht(2, DHT11);
RTC_DS3231 rtc;

#define sw5 3 
#define sw4 A3 
#define sw3 A2
#define sw2 A1
#define sw1 A0

#define selenoid1 10
#define selenoid2 11

#define pinServo1 13
#define pinServo2 12

#define addKondisi 1

char  buf1[32], buf2[32], buf3[32];
char  data;
long  count;
int   m, mm, tt, kondisi, intruksi = 0;
int   sudutServoA, sudutServoB, sudutServoC;
int   tahun, bulan, hari, jam, menit, detik;
int   countBuka, countAuto, countManual, countAutoManual, detikBuka, detikTutup;
byte  enableMakan = 'N', enableMinum = 'N', cekStart = 'N', cekWaktu = 'N', cekSuhu = 'N';
byte  runIntruksiA = 'N', runIntruksiB = 'N';
float suhu, kelembapan;

const char *tampilStart[3] = {
  "Auto       ",
  "Manual     ",
  "Auto Manual",
};
const char *tampilTest[4] = {
  "Buka Makan  ",
  "Buka Minum  ",
  "Tgl Waktu   ",
  "Suhu Klmbpan",
  };
const char tampilHari[7][12] = {
  "Min",
  "Sen",
  "Sel",
  "Rab",
  "Kam",
  "Jum",
  "Sab",
  };

void setup() {
  rtc.begin();
  dht.begin();
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  lcd.begin(20,4);
  Serial.begin(9600);
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }  

  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);
  pinMode(sw3, INPUT_PULLUP);
  pinMode(sw4, INPUT_PULLUP);
  pinMode(sw5, INPUT_PULLUP);

  pinMode(selenoid1, OUTPUT);
  pinMode(selenoid2, OUTPUT);
  digitalWrite(selenoid1, HIGH);
  digitalWrite(selenoid2, HIGH);

  MsTimer2::set(100, waktuTunda);
  MsTimer2::start();

  servo1.write(180);
  servo2.write(180);

  kondisi = EEPROM.read(addKondisi);
}

byte baruReset = 'Y';

void loop() {
  if (baruReset == 'Y') {
    intruksi = 0;
    listenTelegram();
    tanggalWaktu(); suhuKelembapan();
    Serial.print(String("#,") + (int)intruksi + String(",") + (int)jam + String(",") + (int)menit + String(",") + (int)detik + String(",") + (int)suhu + String(",") + (int)kelembapan + String(",\n"));
    
    if (kondisi == 1) { runAuto(); }
    else if (kondisi == 2) { runManual(); }
    else if (kondisi == 3) { runAutoManual(); }
  }
  baruReset = 'N';
  
  DateTime now = rtc.now(); tanggalWaktu();
  
  lcd.setCursor(0, m); lcd.print(">");  
  lcd.setCursor(1, 0); lcd.print("Start: "); lcd.print(tampilStart[mm]);
  lcd.setCursor(1, 1); lcd.print("Test : "); lcd.print(tampilTest[tt]);
  lcd.setCursor(3, 2);
  lcd.print(tampilHari[now.dayOfTheWeek()]);
  lcd.print(","); lcd.print((int)hari);
  lcd.print("-"); lcd.print((int)bulan);
  lcd.print("-"); lcd.print((int)tahun); lcd.print("   ");
  lcd.setCursor(6, 3);
  lcd.print((int)jam); lcd.print(":");
  lcd.print((int)menit); lcd.print(":");
  lcd.print((int)detik); lcd.print(" ");
  
  if (U()) {lcd.setCursor(0, m); lcd.print(" "); m--; delay(150); }
  if (D()) {lcd.setCursor(0, m); lcd.print(" "); m++; delay(150); }
  if (P() && m == 0) { mm--; delay(150); }
  if (M() && m == 0) { mm++; delay(150); }
  if (P() && m == 1) { tt--; delay(150); }
  if (M() && m == 1) { tt++; delay(150); }

  if (m < 0) { m = 1; } 
  if (m > 1) { m = 0; }
  if (mm < 0) { mm = 2; } 
  if (mm > 2) { mm = 0; }
  if (tt < 0) { tt = 3; } 
  if (tt > 3) { tt = 0; }

  if (O() && m == 0 && mm == 0) { kondisi = 1; EEPROM.write(addKondisi, kondisi); runAuto(); }
  if (O() && m == 0 && mm == 1) { kondisi = 2; EEPROM.write(addKondisi, kondisi); runManual(); }
  if (O() && m == 0 && mm == 2) { kondisi = 3; EEPROM.write(addKondisi, kondisi); runAutoManual(); }

  if (O() && m == 1 && tt == 0) { openServo(); delay(2000); closeServo(); }
  if (O() && m == 1 && tt == 1) { openSelenoid(); delay(2000); closeSelenoid(); }
  if (O() && m == 1 && tt == 2) { tampilWaktu(); }
  if (O() && m == 1 && tt == 3) { tampilSuhu(); }

  count++;
  if (count > 1000) {
    intruksi = 0;
    tanggalWaktu(); suhuKelembapan();
    Serial.print(String("#,") + (int)intruksi + String(",") + (int)jam + String(",") + (int)menit + String(",") + (int)detik + String(",") + (int)suhu + String(",") + (int)kelembapan + String(",\n"));
    count = 0;
  }
  
  listenTelegram();
}

void waktuTunda() {  
  
  countBuka++;
  if (countBuka > 10) {
    detikBuka++;
    detikTutup++;
    countBuka = 0;
  }
  
  countAuto++;
  countManual++;
  countAutoManual++;
  }
bool O() {
  if (digitalRead(sw1) == 0) {
    return true;
  }
  else {
    return false;
  }
}
bool D() {
  if (digitalRead(sw2) == 0) {
    return true;
  }
  else {
    return false;
  }
}
bool U() {
  if (digitalRead(sw3) == 0) {
    return true;
  }
  else {
    return false;
  }
}
bool P() {
  if (digitalRead(sw4) == 0) {
    return true;
  }
  else {
    return false;
  }
}
bool M() {
  if (digitalRead(sw5) == 0) {
    return true;
  }
  else {
    return false;
  }
}

void suhuKelembapan() {
  suhu = dht.readTemperature();
  kelembapan = dht.readHumidity();

//  lcd.setCursor(5,1); 
//  lcd.print("Suhu: "); lcd.print((int)suhu); lcd.print("C"); lcd.print(" "); 
//  lcd.setCursor(2,2); 
//  lcd.print("Kelembapan: "); lcd.print((int)kelembapan); lcd.print("RH"); lcd.print(" ");
}

void tanggalWaktu() {
  DateTime now = rtc.now();
  tahun = now.year();
  bulan = now.month();
  hari = now.day();
  jam = now.hour();
  menit = now.minute();
  detik = now.second();

//  lcd.setCursor(3,2);
//  lcd.print(tampilHari[now.dayOfTheWeek()]);
//  lcd.print(","); lcd.print((int)hari);
//  lcd.print("-"); lcd.print((int)bulan);
//  lcd.print("-"); lcd.print((int)tahun); lcd.print(" ");
//  
//  lcd.setCursor(6,3);
//  lcd.print((int)jam); lcd.print(":");
//  lcd.print((int)menit); lcd.print(":");
//  lcd.print((int)detik); lcd.print(" ");
}

void tampilSuhu() {
  delay(300); lcd.clear();
  while (!O()) {
    suhuKelembapan();
    lcd.setCursor(5,1); 
    lcd.print("Suhu: "); lcd.print((int)suhu); lcd.print("C"); lcd.print(" "); 
    lcd.setCursor(2,2); 
    lcd.print("Kelembapan: "); lcd.print((int)kelembapan); lcd.print("RH"); lcd.print(" ");
  }
  delay(300); lcd.clear();
}

void tampilWaktu() {
  delay(300); lcd.clear();
  while (!O()) {
    DateTime now = rtc.now(); tanggalWaktu();
    lcd.setCursor(3,1);
    lcd.print(tampilHari[now.dayOfTheWeek()]);
    lcd.print(","); lcd.print((int)hari);
    lcd.print("-"); lcd.print((int)bulan);
    lcd.print("-"); lcd.print((int)tahun); lcd.print(" ");
    
    lcd.setCursor(6,2);
    lcd.print((int)jam); lcd.print(":");
    lcd.print((int)menit); lcd.print(":");
    lcd.print((int)detik); lcd.print(" ");
  }
  delay(300); lcd.clear();
}

void openServo() {
  servo1.write(100);
  servo2.write(100);
}

void closeServo() {
  servo1.write(180);
  servo2.write(180);
}

void openSelenoid(){
  digitalWrite(selenoid1, LOW);
}

void closeSelenoid(){
  digitalWrite(selenoid1, HIGH);
}

void bukaMakan() {
  delay(300); lcd.clear();
  while (!(P() || M())) {
    DateTime now = rtc.now(); 
    tanggalWaktu(); 
    suhuKelembapan();

    openServo();
    lcd.setCursor(0,0); lcd.print("Memberi Makan...");
    lcd.setCursor(0,1);
    lcd.print(tampilHari[now.dayOfTheWeek()]);
    lcd.print(","); lcd.print((int)hari);
    lcd.print("-"); lcd.print((int)bulan);
    lcd.print("-"); lcd.print((int)tahun); lcd.print(" ");
    
    lcd.setCursor(0,2);
    lcd.print((int)jam); lcd.print(":");
    lcd.print((int)menit); lcd.print(":");
    lcd.print((int)detik); lcd.print(" ");
    
    lcd.setCursor(0,3); 
    lcd.print("Suhu:"); lcd.print((int)suhu); lcd.print("C"); 
    lcd.print(" Klbpn:"); lcd.print((int)kelembapan); lcd.print(" ");
  }
  delay(300); lcd.clear(); return(runManual());
}

void bukaMinum() {
  delay(300); lcd.clear();
  while (!(P() || M())) {
    DateTime now = rtc.now(); 
    tanggalWaktu(); 
    suhuKelembapan();

    openSelenoid();
    lcd.setCursor(0,0); lcd.print("Memberi Minum...");
    lcd.setCursor(0,1);
    lcd.print(tampilHari[now.dayOfTheWeek()]);
    lcd.print(","); lcd.print((int)hari);
    lcd.print("-"); lcd.print((int)bulan);
    lcd.print("-"); lcd.print((int)tahun); lcd.print(" ");
    
    lcd.setCursor(0,2);
    lcd.print((int)jam); lcd.print(":");
    lcd.print((int)menit); lcd.print(":");
    lcd.print((int)detik); lcd.print(" ");
    
    lcd.setCursor(0,3); 
    lcd.print("Suhu:"); lcd.print((int)suhu); lcd.print("C"); 
    lcd.print(" Klbpn:"); lcd.print((int)kelembapan); lcd.print(" ");
  }
  delay(300); lcd.clear(); return(runManual());
}

void bukaMakanMinum() {
  delay(300); lcd.clear();
  while (!(P() || M())) {
    DateTime now = rtc.now(); 
    tanggalWaktu(); 
    suhuKelembapan();

    openServo();
    openSelenoid();
    lcd.setCursor(0,0); lcd.print("Memberi Mkn/Mnm...");
    lcd.setCursor(0,1);
    lcd.print(tampilHari[now.dayOfTheWeek()]);
    lcd.print(","); lcd.print((int)hari);
    lcd.print("-"); lcd.print((int)bulan);
    lcd.print("-"); lcd.print((int)tahun); lcd.print(" ");
    
    lcd.setCursor(0,2);
    lcd.print((int)jam); lcd.print(":");
    lcd.print((int)menit); lcd.print(":");
    lcd.print((int)detik); lcd.print(" ");
    
    lcd.setCursor(0,3); 
    lcd.print("Suhu:"); lcd.print((int)suhu); lcd.print("C"); 
    lcd.print(" Klbpn:"); lcd.print((int)kelembapan); lcd.print(" ");
  }
  delay(300); lcd.clear(); return(runManual());
}

void runAuto() {
  delay(300); lcd.clear();
  runIntruksiA = 'N';
  intruksi = 1;
      
  while (!O()) {
    lcd.setCursor(0,0); lcd.print("RUN AUTO");

    DateTime now = rtc.now();
    tanggalWaktu(); suhuKelembapan();

    lcd.setCursor(0,1);
    lcd.print(tampilHari[now.dayOfTheWeek()]); lcd.print(",");
    sprintf(buf1,"%2d-%2d-%2d ",(int)hari,(int)bulan,(int)tahun);
    lcd.print(buf1);

    lcd.setCursor(0,2);
    sprintf(buf2,"%2d:%2d:%2d ",(int)jam,(int)menit,(int)detik);
    lcd.print(buf2);

    lcd.setCursor(0,3);
    sprintf(buf3,"Suhu:%2dC Klbpan:%2dRH ",(int)suhu,(int)kelembapan);
    lcd.print(buf3);

    if (menit > 1) {
      runIntruksiA = 'N';
    }
    
    if (jam == 7 && menit <= 1) {
      if (runIntruksiA == 'N') {
        detikBuka = 0;
        runIntruksiA = 'Y';
      }
      //lcd.setCursor(17,1); lcd.print(detikBuka); lcd.print("   ");
            
      if (detikBuka <= 10) {
        openServo();
        openSelenoid();
        lcd.setCursor(10,2); lcd.print("Buka..."); lcd.print("  ");
      }
      if (detikBuka > 10) {
        closeServo();
        closeSelenoid();
        lcd.setCursor(10,2); lcd.print("Tutup..."); lcd.print("  ");
      }
    }
    else if (jam == 12 && menit <= 1) {
      if (runIntruksiA == 'N') {
        detikBuka = 0;
        runIntruksiA = 'Y';
      }
      //lcd.setCursor(17,1); lcd.print(detikBuka); lcd.print("   ");
      
      if (detikBuka <= 10) {
        openServo();
        openSelenoid();
        lcd.setCursor(10,2); lcd.print("Buka..."); lcd.print("  ");
      }
      if (detikBuka > 10) {
        closeServo();
        closeSelenoid();
        lcd.setCursor(10,2); lcd.print("Tutup..."); lcd.print("  ");
      }  
    }
    else if (jam == 16 && menit <= 1) {
      if (runIntruksiA == 'N') {
        detikBuka = 0;
        runIntruksiA = 'Y';
      }
      //lcd.setCursor(17,1); lcd.print(detikBuka); lcd.print("   ");
      
      if (detikBuka <= 10) {
        openServo();
        openSelenoid();
        lcd.setCursor(10,2); lcd.print("Buka..."); lcd.print("  ");
      }
      if (detikBuka > 10) {
        closeServo();
        closeSelenoid();
        lcd.setCursor(10,2); lcd.print("Tutup..."); lcd.print("  ");
      }
    }
    
    count++;
    if (count > 1000) {
  
      tanggalWaktu(); suhuKelembapan();
      Serial.print(String("!,") + (int)intruksi + String(",") + (int)jam + String(",") + (int)menit + String(",") + (int)detik + String(",") + (int)suhu + String(",") + (int)kelembapan + String(",\n"));
      count = 0;
    }
  }
  delay(300); lcd.clear();
}

void runManual() {
  delay(300); lcd.clear();
  int m = 1;
  intruksi = 2;
  
  while (!O()) {
    lcd.setCursor(0,m); lcd.print(">");
    lcd.setCursor(0,0); lcd.print("RUN MANUAL");
    lcd.setCursor(1,1); lcd.print("Buka Makan");
    lcd.setCursor(1,2); lcd.print("Buka Minum");
    lcd.setCursor(1,3); lcd.print("Buka Makan/Minum");
  
    if (m < 1) { m = 3; } 
    if (m > 3) { m = 1; }
    if (U()) {lcd.setCursor(0, m); lcd.print(" "); m--; delay(150); }
    if (D()) {lcd.setCursor(0, m); lcd.print(" "); m++; delay(150); }

    if ((P() || M()) && m == 1) { bukaMakan(); }
    if ((P() || M()) && m == 2) { bukaMinum(); }
    if ((P() || M()) && m == 3) { bukaMakanMinum(); }

    closeServo();
    closeSelenoid();

    count++;
    if (count > 1000) {
  
      tanggalWaktu(); suhuKelembapan();
      Serial.print(String("@,") + (int)intruksi + String(",") + (int)jam + String(",") + (int)menit + String(",") + (int)detik + String(",") + (int)suhu + String(",") + (int)kelembapan + String(",\n"));
      count = 0;
    }
  }
  delay(300); lcd.clear();
}

void runAutoManual() {
  delay(300); lcd.clear();
  intruksi = 3;
  
  while (!O()) {
    lcd.setCursor(0,0); lcd.print("RUN AUTO MANUAL");
    
    DateTime now = rtc.now();
    tanggalWaktu(); suhuKelembapan();

    lcd.setCursor(0,1);
    lcd.print(tampilHari[now.dayOfTheWeek()]); lcd.print(",");
    sprintf(buf1,"%2d-%2d-%2d ",(int)hari,(int)bulan,(int)tahun);
    lcd.print(buf1);

    lcd.setCursor(0,2);
    sprintf(buf2,"%2d:%2d:%2d ",(int)jam,(int)menit,(int)detik);
    lcd.print(buf2);

    lcd.setCursor(0,3);
    sprintf(buf3,"Suhu:%2dC Klbpan:%2dRH ",(int)suhu,(int)kelembapan);
    lcd.print(buf3);

    listenTelegram();

    if (enableMakan == 'Y' && enableMinum == 'N') {

      if (runIntruksiB == 'N') {
        detikBuka = 0;
        runIntruksiB = 'Y';
      }
      //lcd.setCursor(17,1); lcd.print(detikBuka); lcd.print("   ");
      
      if (detikBuka <= 10) {
        lcd.setCursor(18,0); lcd.print("1"); lcd.print("  ");
        openServo();
      }

      if (detikBuka > 10) {
        lcd.setCursor(18,0); lcd.print("   ");
        closeServo();
      }
    }
    else if (enableMakan == 'N' && enableMinum == 'Y') {

      if (runIntruksiB == 'N') {
        detikBuka = 0;
        runIntruksiB = 'Y';
      }
      //lcd.setCursor(17,1); lcd.print(detikBuka); lcd.print("   ");
      
      if (detikBuka <= 10) {
        lcd.setCursor(18,0); lcd.print("2"); lcd.print("  ");
        openSelenoid(); 
      }

      if (detikBuka > 10) {
        lcd.setCursor(18,0); lcd.print("   ");
        closeSelenoid();
      }
    }
    else if (enableMakan == 'Y' && enableMinum == 'Y') {

      if (runIntruksiB == 'N') {
        detikBuka = 0;
        runIntruksiB = 'Y';
      }
      //lcd.setCursor(17,1); lcd.print(detikBuka); lcd.print("   ");
      
      if (detikBuka <= 10) {
        lcd.setCursor(18,0); lcd.print("3"); lcd.print("  ");
        openServo();
        openSelenoid(); 
      }

      if (detikBuka > 10) {
        lcd.setCursor(18,0); lcd.print("   ");
        closeServo();
        closeSelenoid();
      }
    } 

    count++;
    if (count > 1000) {
  
      tanggalWaktu(); suhuKelembapan();
      Serial.print(String("?,") + (int)intruksi + String(",") + (int)jam + String(",") + (int)menit + String(",") + (int)detik + String(",") + (int)suhu + String(",") + (int)kelembapan + String(",\n"));
      count = 0;
    }
  }
  delay(300); lcd.clear();
}

void listenTelegram() {
  if (Serial.available() > 0) {
    char data = Serial.read();
    
    if (data == 'A') {
      lcd.setCursor(10, 2);
      lcd.print("Cek Start ");
      cekStart = 'Y';
      cekWaktu = 'N';
      cekSuhu = 'N';
    }
    else if (data == 'B') {
      lcd.setCursor(10, 2);
      lcd.print("Cek Waktu ");
      cekStart = 'N';
      cekWaktu = 'Y';
      cekSuhu = 'N';
    }
    else if (data == 'C') {
      lcd.setCursor(10, 2);
      lcd.print("Cek Suhu  ");
      cekStart = 'N';
      cekWaktu = 'N';
      cekSuhu = 'Y';
    }
    else if (data == 'D') {
      lcd.setCursor(10, 2);
      lcd.print("Cek All   ");
      cekStart = 'Y';
      cekWaktu = 'Y';
      cekSuhu = 'Y';
    }
    else if (data == 'E') {
      lcd.setCursor(10, 2);
      lcd.print("Makan     ");
      enableMakan = 'Y';
      enableMinum = 'N';
      runIntruksiB = 'N';  
    }
    else if (data == 'F') {
      lcd.setCursor(10, 2);
      lcd.print("Minum     ");
      enableMakan = 'N';
      enableMinum = 'Y';
      runIntruksiB = 'N';
    }
    else if (data == 'G') {
      lcd.setCursor(10, 2);
      lcd.print("Mkan/Mnum ");
      enableMakan = 'Y';
      enableMinum = 'Y';
      runIntruksiB = 'N';
    }
    else if (data == 'S') {
      lcd.setCursor(10, 2);
      lcd.print("Memilih...");
    }
  }
}
