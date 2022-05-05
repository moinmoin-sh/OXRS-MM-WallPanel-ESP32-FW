/*
 * OXRS_WT32.cpp
 */

#include "Arduino.h"
#include <OXRS_WT32.h>

#include <Ethernet.h>     // For networking
#include <WiFi.h>         // Required for Ethernet to get MAC
#include <PubSubClient.h> // For MQTT

#ifdef WIFI_MODE
#include <WiFiManager.h>  // For WiFi AP config
#endif

#ifndef WIFI_MODE
// Ethernet client
EthernetClient _client;
#else
// Wifi client
WiFiClient _client;
#endif

// MQTT client
PubSubClient _mqttClient(_client);
OXRS_MQTT _mqtt(_mqttClient);

// REST API
#ifndef WIFI_MODE
EthernetServer _server(REST_API_PORT);
#else
WiFiServer _server(REST_API_PORT);
#endif

OXRS_API _api(_mqtt);

// Supported firmware config and command schemas
DynamicJsonDocument _fwConfigSchema(2048);
DynamicJsonDocument _fwCommandSchema(2048);

// MQTT callbacks wrapped by _mqttConfig/_mqttCommand
jsonCallback _onConfig;
jsonCallback _onCommand;

/* JSON helpers */
void _mergeJson(JsonVariant dst, JsonVariantConst src)
{
  if (src.is<JsonObject>())
  {
    for (auto kvp : src.as<JsonObjectConst>())
    {
      _mergeJson(dst.getOrAddMember(kvp.key()), kvp.value());
    }
  }
  else
  {
    dst.set(src);
  }
}

/* Adoption info builders */
void _getFirmwareJson(JsonVariant json)
{
  JsonObject firmware = json.createNestedObject("firmware");

  firmware["name"] = STRINGIFY(FW_NAME);
  firmware["shortName"] = STRINGIFY(FW_SHORT_NAME);
  firmware["maker"] = STRINGIFY(FW_MAKER);
  firmware["version"] = STRINGIFY(FW_VERSION);
}

void _getSystemJson(JsonVariant json)
{
  JsonObject system = json.createNestedObject("system");

  system["heapUsedBytes"] = ESP.getHeapSize();
  system["heapFreeBytes"] = ESP.getFreeHeap();
  system["heapMaxAllocBytes"] = ESP.getMaxAllocHeap();
  system["flashChipSizeBytes"] = ESP.getFlashChipSize();

  system["sketchSpaceUsedBytes"] = ESP.getSketchSize();
  system["sketchSpaceTotalBytes"] = ESP.getFreeSketchSpace();

  system["fileSystemUsedBytes"] = SPIFFS.usedBytes();
  system["fileSystemTotalBytes"] = SPIFFS.totalBytes();
}

void _getNetworkJson(JsonVariant json)
{
  byte mac[6];

  JsonObject network = json.createNestedObject("network");

#ifndef WIFI_MODE
  Ethernet.MACAddress(mac);
  
  network["mode"] = "ethernet";
  network["ip"] = Ethernet.localIP();
#else
  WiFi.macAddress(mac);

  network["mode"] = "wifi";
  network["ip"] = WiFi.localIP();
#endif

  char mac_display[18];
  sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  network["mac"] = mac_display;
}

void _getConfigSchemaJson(JsonVariant json)
{
  JsonObject configSchema = json.createNestedObject("configSchema");

  // Config schema metadata
  configSchema["$schema"] = JSON_SCHEMA_VERSION;
  configSchema["title"] = STRINGIFY(FW_SHORT_NAME);
  configSchema["type"] = "object";

  JsonObject properties = configSchema.createNestedObject("properties");

  // Firmware config schema (if any)
  if (!_fwConfigSchema.isNull())
  {
    _mergeJson(properties, _fwConfigSchema.as<JsonVariant>());
  }
}

void _getCommandSchemaJson(JsonVariant json)
{
  JsonObject commandSchema = json.createNestedObject("commandSchema");

  // Command schema metadata
  commandSchema["$schema"] = JSON_SCHEMA_VERSION;
  commandSchema["title"] = STRINGIFY(FW_SHORT_NAME);
  commandSchema["type"] = "object";

  JsonObject properties = commandSchema.createNestedObject("properties");

  // Firmware command schema (if any)
  if (!_fwCommandSchema.isNull())
  {
    _mergeJson(properties, _fwCommandSchema.as<JsonVariant>());
  }

  // Rack32 commands
  JsonObject restart = properties.createNestedObject("restart");
  restart["title"] = "Restart";
  restart["type"] = "boolean";
}

/* API callbacks */
void _apiAdopt(JsonVariant json)
{
  // Build device adoption info
  _getFirmwareJson(json);
  _getSystemJson(json);
  _getNetworkJson(json);
  _getConfigSchemaJson(json);
  _getCommandSchemaJson(json);
}

/* MQTT callbacks */
void _mqttConnected()
{
  // Publish device adoption info
  DynamicJsonDocument json(JSON_ADOPT_MAX_SIZE);
  _mqtt.publishAdopt(_api.getAdopt(json.as<JsonVariant>()));

  // Log the fact we are now connected
  Serial.println("[wt32] mqtt connected");
}

void _mqttDisconnected(int state)
{
  // Log the disconnect reason
  // See https://github.com/knolleary/pubsubclient/blob/2d228f2f862a95846c65a8518c79f48dfc8f188c/src/PubSubClient.h#L44
  switch (state)
  {
  case MQTT_CONNECTION_TIMEOUT:
    Serial.println(F("[wt32] mqtt connection timeout"));
    break;
  case MQTT_CONNECTION_LOST:
    Serial.println(F("[wt32] mqtt connection lost"));
    break;
  case MQTT_CONNECT_FAILED:
    Serial.println(F("[wt32] mqtt connect failed"));
    break;
  case MQTT_DISCONNECTED:
    Serial.println(F("[wt32] mqtt disconnected"));
    break;
  case MQTT_CONNECT_BAD_PROTOCOL:
    Serial.println(F("[wt32] mqtt bad protocol"));
    break;
  case MQTT_CONNECT_BAD_CLIENT_ID:
    Serial.println(F("[wt32] mqtt bad client id"));
    break;
  case MQTT_CONNECT_UNAVAILABLE:
    Serial.println(F("[wt32] mqtt unavailable"));
    break;
  case MQTT_CONNECT_BAD_CREDENTIALS:
    Serial.println(F("[wt32] mqtt bad credentials"));
    break;
  case MQTT_CONNECT_UNAUTHORIZED:
    Serial.println(F("[wt32] mqtt unauthorised"));
    break;
  }
}

void _mqttConfig(JsonVariant json)
{
  // Pass on to the firmware callback
  if (_onConfig)
  {
    _onConfig(json);
  }
}

void _mqttCommand(JsonVariant json)
{
  // Check for library commands
  if (json.containsKey("restart") && json["restart"].as<bool>())
  {
    ESP.restart();
  }

  // Pass on to the firmware callback
  if (_onCommand)
  {
    _onCommand(json);
  }
}

void _mqttCallback(char *topic, byte *payload, int length)
{
  // Pass down to our MQTT handler and check it was processed ok
  int state = _mqtt.receive(topic, payload, length);
  switch (state)
  {
  case MQTT_RECEIVE_ZERO_LENGTH:
    Serial.println(F("[wt32] empty mqtt payload received"));
    break;
  case MQTT_RECEIVE_JSON_ERROR:
    Serial.println(F("[wt32] failed to deserialise mqtt json payload"));
    break;
  case MQTT_RECEIVE_NO_CONFIG_HANDLER:
    Serial.println(F("[wt32] no mqtt config handler"));
    break;
  case MQTT_RECEIVE_NO_COMMAND_HANDLER:
    Serial.println(F("[wt32] no mqtt command handler"));
    break;
  }
}

/* Main program */
void OXRS_WT32::setMqttBroker(const char *broker, uint16_t port)
{
  _mqtt.setBroker(broker, port);
}

void OXRS_WT32::setMqttClientId(const char *clientId)
{
  _mqtt.setClientId(clientId);
}

void OXRS_WT32::setMqttAuth(const char *username, const char *password)
{
  _mqtt.setAuth(username, password);
}

void OXRS_WT32::setMqttTopicPrefix(const char *prefix)
{
  _mqtt.setTopicPrefix(prefix);
}

void OXRS_WT32::setMqttTopicSuffix(const char *suffix)
{
  _mqtt.setTopicSuffix(suffix);
}

void OXRS_WT32::begin(jsonCallback config, jsonCallback command)
{
  // We wrap the callbacks so we can intercept messages intended for the Rack32
  _onConfig = config;
  _onCommand = command;

  // Set up ethernet and obtain an IP address
  byte mac[6];
#ifndef WIFI_MODE
  _initialiseEthernet(mac);
#else
  _initialiseWifi(mac);
#endif

  // Set up MQTT (don't attempt to connect yet)
  _initialiseMqtt(mac);

  // Set up the REST API
  _initialiseRestApi();
}

void OXRS_WT32::loop(void)
{
  // Check our network connection
  if (_isNetworkConnected())
  {
    // Maintain our DHCP lease
#ifndef WIFI_MODE
    Ethernet.maintain();
#endif

    // Handle any MQTT messages
    _mqtt.loop();

    // Handle any REST API requests
#ifndef WIFI_MODE
    EthernetClient client = _server.available();
    _api.checkEthernet(&client);
#else
    WiFiClient client = _server.available();
    _api.checkWifi(&client);
#endif
  }
}

void OXRS_WT32::setConfigSchema(JsonVariant json)
{
  _mergeJson(_fwConfigSchema.as<JsonVariant>(), json);
}

void OXRS_WT32::setCommandSchema(JsonVariant json)
{
  _mergeJson(_fwCommandSchema.as<JsonVariant>(), json);
}

boolean OXRS_WT32::publishStatus(JsonVariant json)
{
  // Exit early if no network connection
  if (!_isNetworkConnected())
  {
    return false;
  }

  boolean success = _mqtt.publishStatus(json);
  return success;
}

boolean OXRS_WT32::publishTelemetry(JsonVariant json)
{
  // Exit early if no network connection
  if (!_isNetworkConnected())
  {
    return false;
  }

  boolean success = _mqtt.publishTelemetry(json);
  return success;
}

#ifndef WIFI_MODE
void OXRS_WT32::_initialiseEthernet(byte *mac)
{
  // Get ESP32 base MAC address
  WiFi.macAddress(mac);

  // Ethernet MAC address is base MAC + 3
  // See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html#mac-address
  mac[5] += 3;

  // Display the MAC address on serial
  char mac_display[18];
  sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(F("[wt32] mac address: "));
  Serial.println(mac_display);

  // Initialise ethernet library
  Ethernet.init(ETHERNET_CS_PIN);

  // Reset Wiznet W5500
  pinMode(WIZNET_RESET_PIN, OUTPUT);
  digitalWrite(WIZNET_RESET_PIN, HIGH);
  delay(250);
  digitalWrite(WIZNET_RESET_PIN, LOW);
  delay(50);
  digitalWrite(WIZNET_RESET_PIN, HIGH);
  delay(350);

  // Get an IP address via DHCP and display on serial
  Serial.print(F("[wt32] ip address: "));
  if (Ethernet.begin(mac, DHCP_TIMEOUT_MS, DHCP_RESPONSE_TIMEOUT_MS))
  {
    Serial.println(Ethernet.localIP());
  }
  else
  {
    Serial.println(F("none"));
  }
}
#else
void OXRS_WT32::_initialiseWifi(byte *mac)
{
  // Ensure we are in the correct WiFi mode
  WiFi.mode(WIFI_STA);

  // Connect using saved creds, or start captive portal if none found
  // Blocks until connected or the portal is closed
  WiFiManager wm;
  if (!wm.autoConnect("OXRS_WiFi", "superhouse"))
  {
    // If we are unable to connect then restart
    ESP.restart();
  }

  // Get ESP8266 base MAC address
  WiFi.macAddress(mac);

  // Format the MAC address for display
  char mac_display[18];
  sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Display MAC/IP addresses on serial
  sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(F("[wt32] wifi mac address: "));
  Serial.println(mac_display);
}
/**
void OXRS_WT32::_initialiseWifi(byte * mac)
{
// Get ESP32 base MAC address
WiFi.macAddress(mac);

// Display the MAC address on serial
char mac_display[18];
sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
Serial.print(F("[wt32] wifi mac address: "));
Serial.println(mac_display);

// Get an IP address via DHCP and display on serial
Serial.print(F("[wt32] wifi ip address: "));
if (WiFi.begin(WIFI_SSID, WIFI_PASSW))
{
  Serial.println(WiFi.localIP());
}
else
{
  Serial.println(F("none"));
}
while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
  }
  Serial.println(WiFi.localIP());

}
**/
#endif

void OXRS_WT32::_initialiseMqtt(byte *mac)
{
  // NOTE: this must be called *before* initialising the REST API since
  //       that will load MQTT config from file, which has precendence

  // Set the default client ID to last 3 bytes of the MAC address
  char clientId[32];
  sprintf_P(clientId, PSTR("%02x%02x%02x"), mac[3], mac[4], mac[5]);
  _mqtt.setClientId(clientId);

  // Register our callbacks
  _mqtt.onConnected(_mqttConnected);
  _mqtt.onDisconnected(_mqttDisconnected);
  _mqtt.onConfig(_mqttConfig);
  _mqtt.onCommand(_mqttCommand);

  // Start listening for MQTT messages
  _mqttClient.setCallback(_mqttCallback);
}

void OXRS_WT32::_initialiseRestApi(void)
{
  // NOTE: this must be called *after* initialising MQTT since that sets
  //       the default client id, which has lower precendence than MQTT
  //       settings stored in file and loaded by the API

  // Set up the REST API
  _api.begin();
  _server.begin();

  // Register our callbacks
  _api.onAdopt(_apiAdopt);
}

void OXRS_WT32::restartApi(void)
{
  _api.begin();
}

boolean OXRS_WT32::_isNetworkConnected(void)
{
#ifndef WIFI_MODE
  return Ethernet.linkStatus() == LinkON;
#else
  return WiFi.status() == WL_CONNECTED;
#endif
}

// getters for LVGL

void OXRS_WT32::getIPaddressTxt(char *buffer)
{
  IPAddress ip = IPAddress(0, 0, 0, 0);

  if (_isNetworkConnected())
  {
#ifndef WIFI_MODE
    ip = Ethernet.localIP();
#else
    ip = WiFi.localIP();
#endif
  }

  if (ip[0] == 0)
  {
    sprintf(buffer, "---.---.---.---");
  }
  else
  {
    sprintf(buffer, "%03d.%03d.%03d.%03d", ip[0], ip[1], ip[2], ip[3]);
  }
}

void OXRS_WT32::getMACaddressTxt(char *buffer)
{
  byte mac[6];

#ifndef WIFI_MODE
  Ethernet.MACAddress(mac);
#else
  WiFi.macAddress(mac);
#endif
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void OXRS_WT32::getMQTTtopicTxt(char *buffer)
{
  char topic[64];
  if (!_mqtt.connected())
  {
    sprintf(buffer, "-/------");
  }
  else
  {
    _mqtt.getWildcardTopic(topic);
    strcpy(buffer, "");
    strncat(buffer, topic, 39);
  }
}

connectionState_t OXRS_WT32::getConnectionState(void)
{
  if (_mqtt.connected())
    return CONNECTED_MQTT;
    
  if (_isNetworkConnected())
    return CONNECTED_IP;

  return CONNECTED_NONE;
}
