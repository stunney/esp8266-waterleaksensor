#ifndef __SMARTTHINGSADAPTER_HPP__
#define __SMARTTHINGSADAPTER_HPP__

#include <SmartThingsESP8266WiFi.h>


//******************************************************************************************
// ST_Anything Library 
//******************************************************************************************
#include <Constants.h>       //Constants.h is designed to be modified by the end user to adjust behavior of the ST_Anything library
#include <Device.h>          //Generic Device Class, inherited by Sensor and Executor classes
#include <Sensor.h>          //Generic Sensor Class, typically provides data to ST Cloud (e.g. Temperature, Motion, etc...)
#include <Executor.h>        //Generic Executor Class, typically receives data from ST Cloud (e.g. Switch)
#include <InterruptSensor.h> //Generic Interrupt "Sensor" Class, waits for change of state on digital input 
#include <PollingSensor.h>   //Generic Polling "Sensor" Class, polls Arduino pins periodically
#include <Everything.h>      //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications

#include <PS_TemperatureHumidity.h>  //Implements a Polling Sensor (PS) to measure Temperature and Humidity via DHT library. 
#include <PS_Water.h>        //Implements a Polling Sensor (PS) to measure presence of water (i.e. leak detector)
#include <IS_Contact.h>      //Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin
//******************************************************************************************


class SmartThingsAdapter {
  std::unique_ptr<st::SmartThingsESP8266WiFi> x;
protected:
  //******************************************************************************************
  //st::Everything::callOnMsgSend() optional callback routine.  This is a sniffer to monitor 
  //    data being sent to ST.  This allows a user to act on data changes locally within the 
  //    Arduino sktech.
  //******************************************************************************************
  static void callback(const String &msg)
  {
    Serial.print(F("ST_Anything Callback: Sniffed data = "));
    Serial.println(msg);
    
    //TODO:  Add local logic here to take action when a device's value/state is changed
    
    //Masquerade as the ThingShield to send data to the Arduino, as if from the ST Cloud (uncomment and edit following line)
    //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send
  }

  virtual void enableDebug() {
    st::Everything::debug=true;
    st::Executor::debug=true;
    st::Device::debug=true;
    st::PollingSensor::debug=true;
    st::InterruptSensor::debug=true;
  }

public:
  SmartThingsAdapter(String& ssid, String& ssidpw, IPAddress& smartThingsHubIP, unsigned int serverPort = 80, unsigned int smartThingsHubPort = 39500) {
    enableDebug();    
    
    st::Everything::callOnMsgSend = SmartThingsAdapter::callback;    
    x = std::unique_ptr<st::SmartThingsESP8266WiFi>(new st::SmartThingsESP8266WiFi(ssid, ssidpw, serverPort, smartThingsHubIP, smartThingsHubPort, st::receiveSmartString));
    st::Everything::init();
  }

  virtual void initDevices() {
    st::Everything::initDevices();
  }

  virtual void addWaterLeakSensor(const unsigned int waterSensorGPIOPin = 2) {
    static st::PS_Water waterSensor(F("water"), waterSensorGPIOPin, LOW, true);
    st::Everything::addSensor(&waterSensor);
  }

  virtual void addContactSensor(const unsigned int contactSensorGPIOPin = 2) {
    static st::IS_Contact contactSensor(F("contact1"), contactSensorGPIOPin, LOW, true);
    st::Everything::addSensor(&contactSensor);
  }

  virtual void loop() {
    //*****************************************************************************
    //Execute the Everything run method which takes care of "Everything"
    //*****************************************************************************
    for(int i = 0; i < 100; i++) {
      st::Everything::run();  
      delay(1000);
    }
  }
};

#endif
