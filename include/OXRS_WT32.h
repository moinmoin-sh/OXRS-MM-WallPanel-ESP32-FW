/*
 * OXRS_WT32.h
 */

#ifndef OXRS_WT32_H
#define OXRS_WT32_H

#include <OXRS_MQTT.h> // For MQTT pub/sub
#include <OXRS_API.h>  // For REST API

// Ethernet
#define ETHERNET_CS_PIN 26
#define WIZNET_RESET_PIN 13
#define DHCP_TIMEOUT_MS 15000
#define DHCP_RESPONSE_TIMEOUT_MS 4000

// REST API
#define REST_API_PORT 80

class OXRS_WT32
{
public:
  OXRS_WT32(
      const char *fwName,
      const char *fwShortName,
      const char *fwMaker,
      const char *fwVersion,
      const uint8_t *fwLogo = NULL);

  // These are only needed if performing manual configuration in your sketch, otherwise
  // config is provisioned via the API and bootstrap page
  void setMqttBroker(const char *broker, uint16_t port);
  void setMqttClientId(const char *clientId);
  void setMqttAuth(const char *username, const char *password);
  void setMqttTopicPrefix(const char *prefix);
  void setMqttTopicSuffix(const char *suffix);

  void begin(jsonCallback config, jsonCallback command);
  void loop(void);

  // Firmware can define the config/commands it supports - for device discovery and adoption
  void setConfigSchema(JsonVariant json);
  void setCommandSchema(JsonVariant json);

  // Helpers for publishing to stat/ and tele/ topics
  boolean publishStatus(JsonVariant json);
  boolean publishTelemetry(JsonVariant json);

  void getIPaddressTxt(char *buffer);
  void getMACaddressTxt(char *buffer);
  void getMQTTtopicTxT(char *buffer);
  bool getConnected(void);

  void restartApi(void);

private:
#ifndef WIFI_MODE
  void _initialiseEthernet(byte *mac);
#else
  void _initialiseWifi(byte *mac);
#endif
  void _initialiseMqtt(byte *mac);
  void _initialiseRestApi(void);

  boolean _isNetworkConnected(void);
};

#endif
