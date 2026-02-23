#include <WiFi.h>
#include <HCSR04.h>
#include "wifi_info.h"

#define LED_PIN 2
#define TRIGGER_PIN 12
#define ECHO_PIN 13

UltraSonicDistanceSensor sensor(TRIGGER_PIN, ECHO_PIN);

// put function declarations here:
void blinkLed();

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway (Router IP): ");
  Serial.println(WiFi.gatewayIP()); // Get router's IP address
}

void loop() {
  // put your main code here, to run repeatedly:
  blinkLed();
  Serial.println("Distance: " + String(sensor.measureDistanceCm()) + " cm");
}

// put function definitions here:
void blinkLed() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}