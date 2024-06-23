#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <SPI.h>
#include <MFRC522.h>
#define pin_reset 0      
#define pin_ss    2     
#include <LiquidCrystal_I2C.h>     

LiquidCrystal_I2C lcd(0x27, 16, 2);
int input_status = 0;

const int myPin = D8;

String mac;
byte buffer1[18];
const int rs = 3, en = 15, d4 = 16, d5 = 5, d6 = 4, d7 = 1;

MFRC522 rfid(pin_ss,pin_reset);

const char* ssid = "WIFI_SSID";          // Ganti dengan nama SSID WiFi Anda
const char* password = "WIFI_PASSWORD";  // Ganti dengan password WiFi Anda

// const char* serverName = "http://localhost:1211/api/ListMahasiswa/InsertData"; // Ganti dengan URL API Anda
const char* ServerNameAbsen = "http://localhost:1211/api/ListAbsen/InsertData"; // Ganti dengan URL API Anda
const char* ServerNameAkun = "http://localhost:1211/api/ListMahasiswa/InsertData"; // Ganti dengan URL API Anda

void setup() {
  Serial.begin(115200);
  pinMode(myPin, OUTPUT);
  // RFID & LCD INIT START
  SPI.begin();    
  rfid.PCD_Init();   
  
  lcd.init();
  lcd.begin(16,2);
  lcd.backlight();
  // RFID & LCD INIT END
  // Hubungkan ke WiFi
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  
  Serial.println("Terhubung ke WiFi");
  Serial.println("Input ID Card");
}

String baca_rfid(){
  mac = "";
  MFRC522::MIFARE_Key key;
  MFRC522::StatusCode status;

  for (byte i = 0; i < 6; i++){
   key.keyByte[i] = 0xFF;
  }

  rfid.PICC_ReadCardSerial();

  Serial.println(F("**Card Detected:**"));
  Serial.print("ID :");

  for (byte i = 0; i < rfid.uid.size; i++)
  {
    mac.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    mac.concat(String(rfid.uid.uidByte[i], HEX));
  }

  Serial.println(mac);
  lcd.setCursor(0, 0);
  lcd.print(mac);
  Serial.println("\n**Pembacaan Selesai**\n");
  return mac; 
}

void input_rrfid(const char* serverName){
  WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");

      // Membuat body JSON
      // String jsonData = "{\"nim\":\"" + String(i) + "\", \"nama_mhs\":\"otong" + String(i) + "\"}";;
      String jsonData = "{\"uuid_card\":\"" + baca_rfid() + "\", \"nama_mhs\":\"\"}";

      // Mengirimkan permintaan POST
      int httpResponseCode = http.POST(jsonData);
      
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("HTTP Response code: " + String(httpResponseCode));
        Serial.println("Response: " + response);
      } else {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
      }
      // Menutup koneksi
      http.end();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (input_status == 1 && rfid.PICC_IsNewCardPresent()){
      digitalWrite(myPin, HIGH);
      input_rrfid(ServerNameAbsen);
      delay(500);
      digitalWrite(myPin, LOW);
      delay(1500);
    }else if(input_status != 1 && rfid.PICC_IsNewCardPresent()){
      digitalWrite(myPin, HIGH);
      input_rrfid(ServerNameAkun);
      input_status = 1;
      delay(500);
      digitalWrite(myPin, LOW);
      delay(1500);
    }
  } else {
    Serial.println("WiFi Disconnected");
  }
}
