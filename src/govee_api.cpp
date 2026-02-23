#include "govee_api.h"

void GoveeAPI::getDevices(GoveeDevice* devices, int& device_count) {
  device_count = 0;

  if (!http_client.begin(DEVICES_URL)) {
    Serial.println("[GOVEE] Failed to begin HTTP connection");
    return;
  }

  http_client.addHeader("Govee-API-Key", GOVEE_API_KEY);
  http_client.addHeader("Content-Type", "application/json");

  int http_code = http_client.GET();

  if (http_code == 200) {
    String payload = http_client.getString();
    parseDeviceResponse(payload, devices, device_count);

    for (int i = 0; i < device_count; i++) {
      getDeviceStatus(devices[i]);
    }
  } else {
    Serial.print("[GOVEE] HTTP Error getting devices: ");
    Serial.println(http_code);
  }

  http_client.end();
}

void GoveeAPI::getDeviceStatus(GoveeDevice& device) {
  http_client.end();

  if (!http_client.begin(STATE_URL)) {
    Serial.println("[GOVEE] Failed to begin HTTP connection for status");
    return;
  }

  http_client.addHeader("Govee-API-Key", GOVEE_API_KEY);
  http_client.addHeader("Content-Type", "application/json");

  JsonDocument payload_doc;
  payload_doc["requestId"] = "esp32-status-check";
  JsonObject payload_obj = payload_doc["payload"].to<JsonObject>();
  payload_obj["sku"] = device.model;
  payload_obj["device"] = device.device_id;

  String json_payload;
  serializeJson(payload_doc, json_payload);

  int http_code = http_client.POST(json_payload);

  if (http_code == 200) {
    String response = http_client.getString();
    parseStateResponse(response, device);
  } else {
    Serial.print("[GOVEE] HTTP Error getting status: ");
    Serial.println(http_code);
  }

  http_client.end();
}

void GoveeAPI::powerDevice(GoveeDevice& device, bool on) {
  http_client.end();

  if (!http_client.begin(CONTROL_URL)) {
    Serial.println("[GOVEE] Failed to begin HTTP connection for power control");
    return;
  }

  http_client.addHeader("Govee-API-Key", GOVEE_API_KEY);
  http_client.addHeader("Content-Type", "application/json");

  JsonDocument payload_doc;
  payload_doc["requestId"] = "esp32-power-control";
  JsonObject payload_obj = payload_doc["payload"].to<JsonObject>();
  payload_obj["sku"] = device.model;
  payload_obj["device"] = device.device_id;

  JsonObject capability = payload_obj["capability"].to<JsonObject>();
  capability["type"] = "devices.capabilities.on_off";
  capability["instance"] = "powerSwitch";
  capability["value"] = on ? 1 : 0;

  String json_payload;
  serializeJson(payload_doc, json_payload);

  int http_code = http_client.POST(json_payload);

  if (http_code == 200) {
    Serial.print("[GOVEE] Power control successful for ");
    Serial.println(device.device_name);
    device.power_on = on;
  } else {
    Serial.print("[GOVEE] HTTP Error controlling power: ");
    Serial.println(http_code);
  }

  http_client.end();
}

void GoveeAPI::printDevices(GoveeDevice* devices, int device_count) {
  Serial.println("\n=== Govee Devices ===");
  for (int i = 0; i < device_count; i++) {
    Serial.print("Device ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(devices[i].device_name);
    Serial.print("  ID: ");
    Serial.println(devices[i].device_id);
    Serial.print("  Model: ");
    Serial.println(devices[i].model);
    Serial.print("  Online: ");
    Serial.println(devices[i].online ? "YES" : "NO");
    Serial.print("  Power: ");
    Serial.println(devices[i].power_on ? "ON" : "OFF");
    Serial.println();
  }
}

void GoveeAPI::parseDeviceResponse(String json_response, GoveeDevice* devices, int& device_count) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json_response);

  if (error) {
    Serial.print("[GOVEE] JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }

  int code = doc["code"];
  if (code != 200) {
    Serial.print("[GOVEE] API Error code: ");
    Serial.println(code);
    return;
  }

  JsonArray data_array = doc["data"];
  device_count = 0;

  for (JsonObject device_obj : data_array) {
    if (device_count >= 10) break;

    devices[device_count].model = device_obj["sku"].as<String>();
    devices[device_count].device_id = device_obj["device"].as<String>();
    devices[device_count].device_name = device_obj["deviceName"] | "Unknown";
    devices[device_count].power_on = false;
    devices[device_count].online = false;

    JsonArray capabilities = device_obj["capabilities"];
    for (JsonObject capability : capabilities) {
      String cap_type = capability["type"];
      if (cap_type == "devices.capabilities.on_off") {
        break;
      }
    }

    device_count++;
  }
}

void GoveeAPI::parseStateResponse(String json_response, GoveeDevice& device) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json_response);

  if (error) {
    Serial.print("[GOVEE] JSON state parse error: ");
    Serial.println(error.c_str());
    return;
  }

  int code = doc["code"];
  if (code != 200) {
    Serial.print("[GOVEE] State API Error for ");
    Serial.print(device.device_name);
    Serial.print(": ");
    Serial.println(code);
    return;
  }

  JsonObject payload = doc["payload"];
  JsonArray capabilities = payload["capabilities"];

  for (JsonObject capability : capabilities) {
    String type = capability["type"];
    String instance = capability["instance"];

    if (type == "devices.capabilities.online") {
      device.online = capability["state"]["value"];
    }

    if (type == "devices.capabilities.on_off" && instance == "powerSwitch") {
      int power_value = capability["state"]["value"];
      device.power_on = (power_value == 1);
    }
  }
}