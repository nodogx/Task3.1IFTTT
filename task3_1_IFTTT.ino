#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <BH1750.h>
#include <ArduinoHttpClient.h>
#include "secret.h"  // Include the secret file

WiFiSSLClient sslClient;
HttpClient client = HttpClient(sslClient, SECRET_SERVER, 443);  // Use HTTPS port 443
BH1750 lightMeter;

unsigned long lastSendTime = 0;  // Last time data was sent
const long sendInterval = 60000; // Send data every 60 seconds
const int ledPin = 8;  // LED pin

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Wire.begin();

  // Initialize WiFi with secrets
  if (WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Connected to WiFi");
  }

  // Initialize the BH1750 sensor
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("Failed to initialize BH1750");
    while (1);  // Halt if sensor initialization fails
  } else {
    Serial.println("BH1750 initialized");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  if (lux < 10) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED On: Low light detected");
    if (currentMillis - lastSendTime >= sendInterval) {
      sendToZapier(lux);
      lastSendTime = currentMillis; // Update the last send time
    }
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("LED Off");
  }

  delay(2000);  // delay between readings
}

void sendToZapier(float lux) {
  Serial.println("Sending data to Zapier...");
  String jsonData = "{\"lux\": \"" + String(lux, 2) + "\"}";

  client.beginRequest();
  client.post(SECRET_URL);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", jsonData.length());
  client.beginBody();
  client.print(jsonData);
  client.endRequest();

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}
