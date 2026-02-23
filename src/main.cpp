#include <WiFi.h>
#include <HCSR04.h>
#include "wifi_info.h"
#include "govee_api.h"

#define LED_PIN 2
#define TRIGGER_PIN 12
#define ECHO_PIN 13
#define WAVE_COOLDOWN_MS 2000

UltraSonicDistanceSensor sensor(TRIGGER_PIN, ECHO_PIN);
GoveeAPI govee;
GoveeDevice devices[10];
int device_count = 0;

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
  Serial.println(WiFi.gatewayIP());

  Serial.println("\nFetching Govee devices...");
  govee.getDevices(devices, device_count);
  Serial.println("Govee devices:");
  govee.printDevices(devices, device_count);
}

void loop() {
  static unsigned long last_wave_time = 0;
  static unsigned long last_govee_check = 0;

  double distance = sensor.measureDistanceCm();

  if (distance > 0 && distance < 20) {
    digitalWrite(LED_PIN, HIGH);

    if (millis() - last_wave_time > WAVE_COOLDOWN_MS) {
      last_wave_time = millis();
      Serial.println("Wave detected");

      // Refresh device info before toggle
      govee.getDevices(devices, device_count);

      // Toggle lights based on current state (ignoring my tv backlight - devices[0])
      if (devices[1].power_on) {
        govee.powerDevice(devices[1], false);
      } else {
        govee.powerDevice(devices[1], true);
      }

      if (devices[2].power_on) {
        govee.powerDevice(devices[2], false);
      } else {
        govee.powerDevice(devices[2], true);
      }
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}