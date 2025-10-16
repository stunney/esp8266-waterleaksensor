#ifndef __WATERLEAKCONFIGURATION_H__
#define __WATERLEAKCONFIGURATION_H__

class WaterLeakConfiguration {
private:
  static bool isStringNullOrEmpty(const String& value) {
    /*
    Serial.print("Checking for nullOrEmpty on [");
    Serial.print(value);
    Serial.println("]");
    */
    
    return(NULL == value || value.compareTo("") == 0);
  }
public:
  WaterLeakConfiguration() {
    
  }
  
  WaterLeakConfiguration(const WaterLeakConfiguration * const copy) {
    this->ssid = copy->ssid;
    this->ssidpw = copy->ssidpw;
    this->mqtt = copy->mqtt;
  }

  virtual bool isConfigurationComplete() const {
    if(isStringNullOrEmpty(mqtt) || 
       isStringNullOrEmpty(ssid) ||
       isStringNullOrEmpty(ssidpw)) {        
        return false;
    }
    Serial.println(mqtt);
    Serial.println(ssid);
    Serial.println(ssidpw);

    return true;
  }

  virtual void operator=(const WaterLeakConfiguration& rhs) {
    this->ssid = rhs.ssid;
    this->ssidpw = rhs.ssidpw;
    this->mqtt = rhs.mqtt;
  }
  
  String ssid = "";
  String ssidpw = "";
  String mqtt = "";
};

#endif
