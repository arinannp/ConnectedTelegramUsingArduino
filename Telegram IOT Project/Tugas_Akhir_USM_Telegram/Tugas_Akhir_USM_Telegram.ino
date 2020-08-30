#include "arduino_secrets.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "CTBot.h"

// Initialize Wifi connection to the router
const char* ssid  = SECRET_SSID;
const char* password = SECRET_PASS;

// Initialize Telegram BOT
const char BotToken[] = SECRET_BOT_TOKEN;

String pesan;
int messageID = 881788417;
//int messageID = 742060894;

WiFiClientSecure client;
CTBot myBot;

String dataIn, dt[30];
String status1, status2;
boolean parsing = false, loopStatus = false;

int jam, menit, detik, suhu, kelembapan;
int kodeBalasan;
long count;
int i;

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print("Cek Koneksi!");
    //Serial.print(".");
    delay(500);
  }
  
  myBot.wifiConnect(ssid, password);
  myBot.setTelegramToken(BotToken);

  //Serial.println("");
  //delay(2000);
  
  if (myBot.testConnection()){
    //Serial.println("\nTersambung? YA"); 
    digitalWrite(2,LOW);
  }
  else{
    //Serial.println("\nTersambung? NO"); 
    digitalWrite(2,HIGH);
  }

  pinMode(2,OUTPUT); 
}

void loop() {
  // put your main code here, to run repeatedly:
  waitReply();
  updateMessage();
}

void updateMessage() {
  
  yield();
  TBMessage msg;
  myBot.getNewMessage(msg);

  if (msg.text.equals("/start")) {
    digitalWrite(2,LOW);
    Serial.print('S');
    pesan = "Selamat Datang di Telegram IOT Project\n\n";
    pesan = pesan + "   ~ Pilihan setting alat =\n";
    pesan = pesan + "1. Auto: Memberi makan & minum ternak secara otomatis\n";
    pesan = pesan + "2. Manual: Memberi makan & minum ternak secara manual\n";
    pesan = pesan + "3. Auto Manual: Memberi makan & minum ternak melalui telegram\n\n";
    pesan = pesan + "   ~ Pilihan checking alat =\n";
    pesan = pesan + "/cek_start : Cek kondisi setting alat\n";
    pesan = pesan + "/cek_waktu : Cek waktu pemberian pakan terakhir\n";
    pesan = pesan + "/cek_suhu : Cek suhu dan kelembapan ruangan\n";
    pesan = pesan + "/cek_all : Cek semua kondisi data alat\n\n";
    pesan = pesan + "   ~ Pilihan jika setting alat Auto Manual =\n";
    pesan = pesan + "/add_makan : Memberi makan sekarang\n";
    pesan = pesan + "/add_minum : Memberi minum sekarang\n";
    pesan = pesan + "/add_all : Memberi makan/minum sekarang\n\n\n"; 
    pesan = pesan + "Silahkan masukan perintah!!!\n";
    myBot.sendMessage(msg.sender.id, pesan);
    
    pesan = "";
    return;
  } 
  if (msg.text.equals("/cek_start")) {
    digitalWrite(2,HIGH);
    Serial.print('A');
    pesan = "Mohon tunggu sebentar...\n";
    myBot.sendMessage(msg.sender.id, pesan);
    kodeBalasan = 1;
    loopStatus = true;
    while (loopStatus) {
      waitReply();
      if (loopStatus ==  false) {
        break;
      }
    }
    
    pesan = "";
    return;
  }
  if (msg.text.equals("/cek_waktu")) {
    digitalWrite(2,HIGH);
    Serial.print('B');
    pesan = "Mohon tunggu sebentar...\n";
    myBot.sendMessage(msg.sender.id, pesan);
    kodeBalasan = 2;
    loopStatus = true;
    while (loopStatus) {
      waitReply();
      if (loopStatus ==  false) {
        break;
      }
    }
    
    pesan = "";
    return;
  }
  if (msg.text.equals("/cek_suhu")) {
    digitalWrite(2,HIGH);
    Serial.print('C');
    pesan = "Mohon tunggu sebentar...\n";
    myBot.sendMessage(msg.sender.id, pesan);
    kodeBalasan = 3;
    loopStatus = true;
    while (loopStatus) {
      waitReply();
      if (loopStatus ==  false) {
        break;
      }
    }

    pesan = "";
    return;
  }
  if (msg.text.equals("/cek_all")) {
    digitalWrite(2,LOW);
    Serial.print('D');
    pesan = "Mohon tunggu sebentar...\n";
    myBot.sendMessage(msg.sender.id, pesan);
    kodeBalasan = 4;
    loopStatus = true;
    while (loopStatus) {
      waitReply();
      if (loopStatus ==  false) {
        break;
      }
    }

    pesan = "";
    return;
  }
  if (msg.text.equals("/add_makan")) {
    digitalWrite(2,HIGH);
    Serial.print('E');
    pesan = "Ternak sudah diberi makan!\n";
    myBot.sendMessage(msg.sender.id, pesan);

    pesan = "";
    return;
  }
  if (msg.text.equals("/add_minum")) {
    digitalWrite(2,HIGH);
    Serial.print('F');
    pesan = "Ternak sudah diberi minum!\n";
    myBot.sendMessage(msg.sender.id, pesan);

    pesan = "";
    return;
  }
  if (msg.text.equals("/add_all")) {
    digitalWrite(2,LOW);
    Serial.print('G');
    pesan = "Ternak sudah diberi makan dan minum!\n";
    myBot.sendMessage(msg.sender.id, pesan);

    pesan = "";
    return;
  }
}

void dataReceive() {
  char inChar;
  while (Serial.available() > 0) {
    inChar = Serial.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
  }
  if (parsing) {
    parsingData();
    parsing = false;
    dataIn = "";
  }
}

void parsingData() {
  
  int j = 0;
  dt[j] = "";    
                                          
  for (i = 1; i < dataIn.length(); i++) {                  
    if (dataIn[i] == ',') {
      j++;                                                
      dt[j] = "";                                         
    }
    else {
      dt[j] = dt[j] + dataIn[i];                          
    }
  }
}

void Statement1() {
  parsingData();
  if (dt[1].toInt() == 0) {
    status1 = "Status Perangkat tidak diketahui";
  } 
  else if (dt[1].toInt() == 1) {
    status1 = "Status Perangkat Auto Mode";
  }
  else if (dt[1].toInt() == 2) {
    status1 = "Status Perangkat Manual Mode";
  }
  else if (dt[1].toInt() == 3) {
    status1 = "Status Perangkat Auto Manual Mode";
  }
  jam = dt[2].toInt();
  menit = dt[3].toInt();
  detik = dt[4].toInt();
  suhu = dt[5].toInt();
  kelembapan = dt[6].toInt();
}

void Statement2() {
  Statement1(); parsingData();
  if ((jam >= 7 && jam < 12) && dt[1].toInt() == 1) {
    status2 = "Sudah diberi makan/minum pagi saat jam 7";
  }
  else if ((jam >= 12 && jam < 16) && dt[1].toInt() == 1) {
    status2 = "Sudah diberi makan/minum siang saat jam 12"; 
  }
  else if ((jam >= 16 || jam < 7) && dt[1].toInt() == 1) {
    status2 = "Sudah diberi makan/minum sore saat jam 16"; 
  }
  else {
    status2 = "Setting Perangkat ke Auto Mode !!!";
  }  
}

void waitReply() {

  dataReceive();
  pesan = "";
  switch (kodeBalasan) {
    case 1 : {
      Statement1();
      pesan = "Status Perangkat Saat Ini. ";
      pesan += "\n\nStatus : ";
      pesan += status1;
      
      myBot.sendMessage(messageID, pesan);
      loopStatus = false;
      kodeBalasan = 0;
      break;
    }
    case 2 : {
      Statement1();
      pesan = "Cek Waktu Perangkat Saat Ini. ";
      pesan += "\n\nWaktu : ";
      pesan += jam;
      pesan += ":";
      pesan += menit;
      pesan += ":";
      pesan += detik;
      
      myBot.sendMessage(messageID, pesan);
      loopStatus = false;
      kodeBalasan = 0;
      break;
    }
    case 3 : {
      Statement1();
      pesan = "Cek Suhu Perangkat Saat Ini. ";
      pesan += "\n\nSuhu : ";
      pesan += suhu;
      pesan += "\nKelembapan : ";
      pesan += kelembapan;
      
      myBot.sendMessage(messageID, pesan);
      loopStatus = false;
      kodeBalasan = 0;
      break;
    }
    case 4 : {
      Statement1();
      Statement2();
      pesan = "Cek Semua Kondisi Perangkat Saat Ini. ";
      pesan += "\n\nWaktu : ";
      pesan += jam;
      pesan += ":";
      pesan += menit;
      pesan += ":";
      pesan += detik;
      pesan += "\nSuhu : ";
      pesan += suhu;
      pesan += "\nKelembapan : ";
      pesan += kelembapan;
      pesan += "\nStatus 1 : ";
      pesan += status1;
      pesan += "\nStatus 2 : ";
      pesan += status2;
      
      myBot.sendMessage(messageID, pesan);
      loopStatus = false;
      kodeBalasan = 0;
      break;
    }
  }
}
