#include <Arduino.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

const int numLoops = 3; // Number of loops
float lightReadings[numLoops]; // Array to store light readings
const float intensityWithoutSmoke = 1230; // Maximum intensity without smoke
const int LED_PIN = 26; // LED indicator for when the device is reading the data
bool startReading = false;

void setup() {
  Serial.begin(9600);
  SerialBT.begin("ESP32SmokeDetector"); // Bluetooth device name
  Serial.println("The device started, now you can pair it with Bluetooth!");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    if (command == "START") {
      startReading = true;
      Serial.println("Start command received");
    }
  }

  if (startReading) {
    static unsigned long previousMillis = 0;
    const long interval = 10000; // Interval to send data

    if (millis() - previousMillis >= interval) {
      previousMillis = millis();
      Serial.println("---------------");
      float lightIntensity = analogRead(32);
      Serial.print(lightIntensity);
      Serial.println("---------------");
      float average = readAverageOpacity();
      Serial.println("---------------");
      sendOpacityData(average);
      Serial.println("---------------");
      startReading = false; // Stop reading after sending the data
    }
  }
}

float readAverageOpacity() {
  for (int i = 0; i < numLoops; ++i) {
    float lightIntensity = analogRead(32);
    float opacity = 3 - ((lightIntensity / intensityWithoutSmoke) * 3);
    lightReadings[i] = opacity;

    digitalWrite(LED_PIN, HIGH);
    Serial.print("Loop ");
    Serial.print(i + 1);
    Serial.print(": Light percentage = ");
    Serial.println(opacity);
    
    delay(5000);
  }

  float total = 0;
  for (int i = 0; i < numLoops; ++i) {
    total += lightReadings[i];
  }

  digitalWrite(LED_PIN, LOW);
  

  return total / numLoops;
}

void sendOpacityData(float average) {
  if (SerialBT.hasClient()) {
    Serial.println("Bluetooth client connected");
    String data = String(average);
    SerialBT.println(data);
    Serial.println("Data sent: " + data);
  } else {
    Serial.println("No Bluetooth client connected");
  }
}
