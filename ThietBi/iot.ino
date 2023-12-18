#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Wire.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "time.h"
#include "addons/TokenHelper.h"
// // Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// Insert your network credentials
#define WIFI_SSID "Oneplus 11"
#define WIFI_PASSWORD "Hieu1812"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDwX9EwnR4LIbSq_PA1Vxoc32E6HZCtiuk"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "hieutrungbox3@gmail.com"
#define USER_PASSWORD "Hieu1812"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp8266-mq2-14a94-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String timePath = "/timestamp";
String gasPath = "/gas";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

#define DHTTYPE DHT11  // Type of DHT sensor
#define gasPin A0
#define BUZZER_PIN D2
#define LED D6
#define dhtPin D5
//#define SERVO_PIN 23
DHT dht(dhtPin, DHTTYPE); 

WiFiClient espClient;
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org");

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 15000;
void initDHT() {
  dht.begin();
}


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as an output
  initDHT();
  initWiFi();

  timeClient.begin();
  timeClient.setTimeOffset(0 * 60 * 60);  // Điều chỉnh múi giờ
  // configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();


    timeClient.update();
    Serial.println(timeClient.getEpochTime());

    parentPath= databasePath + "/" + String(timeClient.getEpochTime());
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int gasValue = analogRead(gasPin);
    printf("khi gas: %d\n", gasValue);

    Serial.println(gasValue);
    const char* result = Firebase.RTDB.getJSON(&fbdo, "/devices") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str();
    Serial.printf("Get json... %s\n", result);
    char onoff[3];
    int cnt=0;
    for(int i = 0; i<strlen(result); i++){
      if(result[i]=='0' || result[i]=='1'){
        onoff[cnt++]=result[i];
      }
    }


    if (gasValue > 220) {
    // Activate the buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED, HIGH);

      if(onoff[0] == '1' && onoff[1] == '1'){
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED, HIGH);
      }
      else if(onoff[0] == '1' && onoff[1] == '0'){
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED, LOW);
      }
      else if(onoff[0] == '0' && onoff[1] == '1'){
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED, HIGH);
      }
      else if (gasValue < 220){
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED, LOW);
      }
      delay(1000);  // Buzzer on for 1 second

    }
    else { 
      digitalWrite(LED, LOW);    
      digitalWrite(BUZZER_PIN, LOW);

    }
    json.set(tempPath.c_str(), String(temperature));
    json.set(humPath.c_str(), String(humidity));
    json.set(timePath, String(timeClient.getEpochTime()));
    json.set(gasPath, String(gasValue));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

  // Chờ 1 giây trước khi đo lại
  delay(1000);
}
}