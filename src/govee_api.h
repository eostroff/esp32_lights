#ifndef GOVEE_API_H
#define GOVEE_API_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "wifi_info.h"

struct GoveeDevice {
  String device_id;
  String device_name;
  String model;
  bool power_on;
  bool online;
};

class GoveeAPI {
private:
  HTTPClient http_client;
  const char* DEVICES_URL = "https://openapi.api.govee.com/router/api/v1/user/devices";
  const char* STATE_URL   = "https://openapi.api.govee.com/router/api/v1/device/state";
  const char* CONTROL_URL = "https://openapi.api.govee.com/router/api/v1/device/control";

  void parseDeviceResponse(String json_response, GoveeDevice* devices, int& device_count);
  void parseStateResponse(String json_response, GoveeDevice& device);

public:
  void getDevices(GoveeDevice* devices, int& device_count);
  void getDeviceStatus(GoveeDevice& device);
  void powerDevice(GoveeDevice& device, bool on);
  void printDevices(GoveeDevice* devices, int device_count);
};

#endif