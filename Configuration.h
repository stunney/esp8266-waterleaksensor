#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <ArduinoJson.h>
#include <FS.h>
#include "WaterLeakConfiguration.h"

class Configuration {
private:
  const char* FILENAME = "/config.json";   
protected:
  const String CONFIG_MQTT_SERVER = "mqtt_server";
  const String CONFIG_WIFI_SSID = "wifissid";
  const String CONFIG_WIFI_PASSWORD = "wifipassword";

  //const char* CONFIG_TLS_CA_THUMBPRINT = "tlsCAThumbprint";
  //const char* CONFIG_TLS_CLIENT_CERTIFICATE = "tlsClientCertificate";

  virtual bool readFromFS() {
    File configFile = SPIFFS.open(FILENAME, "r");
    if (!configFile) {
      Serial.println("Failed to open config file");
      return false;
    }
  
    size_t size = configFile.size();
    if (size > 1024) {
      Serial.println("Config file size is too large");
      return false;
    }
  
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
      
    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);
  
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.parseObject(buf.get());
  
    if (!configJson.success()) {
      Serial.println("Failed to parse config file");      
      return false;
    }
    setConfigurationFromJsonObject(configJson);
  }

  virtual void setConfigurationFromJsonObject(const JsonObject& configJson) {
    m_waterLeakConfiguration->ssid = configJson[CONFIG_WIFI_SSID].as<String>();
    m_waterLeakConfiguration->ssidpw = configJson[CONFIG_WIFI_PASSWORD].as<String>();
    m_waterLeakConfiguration->mqtt = configJson[CONFIG_MQTT_SERVER].as<String>();
  }

  virtual void setJsonObjectFromConfiguration(JsonObject& configJson) const {
    configJson[CONFIG_WIFI_SSID] = m_waterLeakConfiguration->ssid;
    configJson[CONFIG_WIFI_PASSWORD] = m_waterLeakConfiguration->ssidpw;
    configJson[CONFIG_MQTT_SERVER] = m_waterLeakConfiguration->mqtt;
  }

  virtual bool mountFilesystem() const {
    if (!SPIFFS.begin()) {
      Serial.println("Failed to mount file system");
      return false;
    }
    return true;
  }

public:
  std::unique_ptr<WaterLeakConfiguration> m_waterLeakConfiguration = std::unique_ptr<WaterLeakConfiguration>(new WaterLeakConfiguration());

  Configuration() {
    if(mountFilesystem()) {
      if(!readFromFS()) {
        Serial.println("Configuration load failure on ctor.  Writing fresh with no values.");
        setEmptyValues();        
      }
    }
  }

  virtual void writeToSerial() const {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.createObject();
    setJsonObjectFromConfiguration(configJson);
    configJson.printTo(Serial);
  }

  virtual bool saveToFS() const {
    File configFile = SPIFFS.open(FILENAME, "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
      return false;
    }
    DynamicJsonBuffer jsonBuffer;
    JsonObject& configJson = jsonBuffer.createObject();
    setJsonObjectFromConfiguration(configJson);

    configJson.printTo(configFile);        
    return true;
  }

  /*
   * { "mqtt_server": "", "wifissid": "", "wifipassword": "" }
   * 
   */ 

  virtual void setEmptyValues() {
    m_waterLeakConfiguration->ssid = String("");
    m_waterLeakConfiguration->ssidpw = String("");
    m_waterLeakConfiguration->mqtt = String("");
  }

  virtual bool clearAndSave() {
    setEmptyValues();
    return saveToFS();
  }
};

#endif
