#include <DHT.h>
#define gasPin A0 
#define buzzerPin D2 
#define redLedPin D6 
#define dhtPin D5 
#define THRESHOLD_GAS 230 // Ngưỡng nồng độ khí gas
#define TEMPERATURE_THRESHOLD 32.0 // Ngưỡng nhiệt độ cảnh báo
  
DHT dht(dhtPin, DHT11);

void setup() {
  Serial.begin(9600); 
  pinMode(buzzerPin, OUTPUT); 
  pinMode(redLedPin, OUTPUT); 
  dht.begin(); 
}

void loop() {
  // Đọc giá trị từ cảm biến khí gas
  int gasValue = analogRead(gasPin);

  // Đo nhiệt độ và độ ẩm từ cảm biến DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // In giá trị nhiệt độ, độ ẩm và nồng độ khí gas ra Serial Monitor
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" °C, Do am: ");
  Serial.print(humidity);
  Serial.print("%, Nong do khi gas: ");
  Serial.println(gasValue);
  Serial.println("----------------------------------------------------------------");
  
  if (gasValue > THRESHOLD_GAS || temperature > TEMPERATURE_THRESHOLD) {
    // Bật còi 
    digitalWrite(buzzerPin, HIGH);
    // Bật đèn LED đỏ
    digitalWrite(redLedPin, HIGH);
    Serial.print("Canh bao phat hien ro ri khi gas !!");
  } else {
    // Tắt còi
    digitalWrite(buzzerPin, LOW);
    // Tắt đèn LED đỏ
    digitalWrite(redLedPin, LOW);
  }

  // Chờ 1 giây trước khi đo lại
  delay(3000);
}
