#ifndef __CONFIGURATIONWEBSERVER_H__
#define __CONFIGURATIONWEBSERVER_H__

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266TrueRandom.h>

#include "WaterLeakConfiguration.h"
#include "Configuration.h"

class ConfigurationWebServer {
private:
  std::unique_ptr<ESP8266WebServer> m_server;
  String m_host = "WLS-Broken";
  bool m_isAPWorking = false;
  IPAddress m_myIP;
protected:
  virtual String generateSSID() {
    //The code herein pulled from: https://github.com/esp8266/Arduino/issues/2189
    uint8_t mac[WL_MAC_ADDR_LENGTH];

    WiFi.softAPmacAddress(mac);

    //AP_BSSID = String (mac [0],HEX) +":"+  String (mac [1],HEX) +":"+ String (mac [2],HEX) +":"+ String (mac [3],HEX) +":"+ String (mac [4],HEX) +":"+ String (mac [5],HEX) ;

    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);

    macID.toUpperCase();

    Serial.println(macID);
    
    return "WLS-" + macID ;
  }  

  const String HTTP_FORM_ARG_SSID = "SSID";
  const String HTTP_FORM_ARG_SSIDPWD = "SSIDPassword";
  const String HTTP_FORM_ARG_MQTT = "MQTTServer";

  String getMACAddress() {  
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    String strMAC = String (mac [0],HEX) +":"+  String (mac [1],HEX) +":"+ String (mac [2],HEX) +":"+ String (mac [3],HEX) +":"+ String (mac [4],HEX) +":"+ String (mac [5],HEX) ;
    return strMAC;
  }


  virtual void handlePOST_update() {
    m_server.get()->on("/update", HTTP_POST, [this](){
      m_server.get()->sendHeader("Connection", "close");
      m_server.get()->sendHeader("Access-Control-Allow-Origin", "*");
      m_server.get()->send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      
      Serial.print("Handling in main POST handle with arg count of ");
      Serial.println(m_server.get()->args());
      
      Serial.println("Creating Configuration model");
      WaterLeakConfiguration c;
      
      for(int i = 0; i < 3; i++) {
        String argName = m_server.get()->argName(i);
        String argValue = m_server.get()->arg(i);
        
        Serial.print(argName);
        Serial.print(" == ");
        Serial.println(argValue);
        
        if(HTTP_FORM_ARG_SSID == argName) {
          Serial.println("Injecting SSID");
          c.ssid = argValue;
        }
        else if(HTTP_FORM_ARG_SSIDPWD == argName) {
          Serial.println("Injecting password");
          c.ssidpw = argValue;
        }
        else if(HTTP_FORM_ARG_MQTT == argName) {
          Serial.println("Injecting mqttServer");
          c.mqtt = argValue;
        }
      }

      if(c.isConfigurationComplete()) {
        Serial.println("writeToSerial");      
        Serial.println("saveToFS");
        m_configuration->m_waterLeakConfiguration->operator=(c);
        m_configuration->saveToFS();
        
        //I am ashamed of myself
        Serial.println("delay(2)");
        delay(2);
  
        Serial.println("restart");
        ESP.restart();
      } else {
        Serial.println("Submitted configuration was incomplete");
      }
    },[this](){      
    });
  }  

  const char* getIndexHtml() {
    static String ret = String("MAC Address: <b>") + getMACAddress() + String("</b><hr/><br/><form method='POST' action='/update' enctype='multipart/form-data'><h1>SSID: </h1><input type='text' name='SSID'><hr/><h1>SSID Password: </h1><input type='text' name='SSIDPassword'><hr/><h1>SmartThings HubIP Address: </h1><input type='text' name='MQTTServer'<hr/><input type='submit' value='Update'></form>");
    return ret.c_str();
  }

  virtual void handleGet() {
    m_server.get()->on("/", HTTP_GET, [this](){
      m_server.get()->sendHeader("Connection", "close");
      m_server.get()->sendHeader("Access-Control-Allow-Origin", "*");
      m_server.get()->send(200,
          "text/html",
          getIndexHtml());
    });
  }

  virtual void startListeningForHTTPRequests() {
    m_server = std::unique_ptr<ESP8266WebServer>(new ESP8266WebServer(80));
    
    MDNS.begin(m_host.c_str());
    
    handleGet();
    handlePOST_update();
    m_server.get()->begin();
    
    MDNS.addService("http", "tcp", 80);
    Serial.print("Ready! Open http://");
    Serial.print(m_myIP);
    Serial.println(" into your browser to configure the device");
  }

  std::shared_ptr<Configuration> m_configuration;
public:
  ConfigurationWebServer(std::shared_ptr<Configuration>& configuration) {
    m_configuration = configuration;
  }

  virtual void setup() {
    delay(1000);

    m_host = generateSSID();

    WiFi.softAP(m_host.c_str(), "nomoreleaks");

    m_myIP = WiFi.softAPIP();
    Serial.println(m_myIP);

    startListeningForHTTPRequests();    
  }

  virtual void loop() {
    m_server.get()->handleClient();
    delay(1);
  }
};

#endif
