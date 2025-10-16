//This project requires that you include the following libraries on your ~/Documents/Arduino/libraries folder first!
//https://github.com/marvinroger/ESP8266TrueRandom
//https://github.com/bblanchon/ArduinoJson
//https://github.com/DanielOgorchock/ST_Anything --Follow the instructions on this site VERY carefully!!!
//

#include <ESP8266WiFi.h>
#include <SmartThingsESP8266WiFi.h>

#include "Configuration.h"
#include "ConfigurationWebServer.h"
#include "SmartThingsAdapter.hpp"
#include "CurrentTimePrinter.hpp"

std::unique_ptr<ConfigurationWebServer> configServer;
std::shared_ptr<Configuration> configuration;
std::shared_ptr<SmartThingsAdapter> smartThings;
std::shared_ptr<CurrentTimePrinter> timePrinter;

#define PIN_CONTACT              2 //Easier, GPIO 0 is for programming.  Leave it be, we have a SINGLE function application here.
const unsigned int serverPort = 80; // port to run the http server on

void setup() {
  Serial.begin(115200); //Lots of the modules require this for Serial.print operations

  delay(5000); //Do NOT remove

  configuration = std::shared_ptr<Configuration>(new Configuration());  

  if(configuration->m_waterLeakConfiguration->isConfigurationComplete()) {
    //Normal operations
    Serial.print("Configuration read OKAY, validating now...");

    timePrinter = std::shared_ptr<CurrentTimePrinter>(new CurrentTimePrinter(5));   //Eastern Std Time

    IPAddress hubIp;
    hubIp.fromString(configuration->m_waterLeakConfiguration->mqtt);

    smartThings = std::unique_ptr<SmartThingsAdapter>(new SmartThingsAdapter(configuration->m_waterLeakConfiguration->ssid, configuration->m_waterLeakConfiguration->ssidpw, hubIp));
    //smartThings->addWaterLeakSensor(PIN_CONTACT);
    smartThings->addContactSensor(PIN_CONTACT);
    smartThings->initDevices();
  }
  else
  {
    Serial.print("Init Mode.  Setting up configuration AP");
    configServer = std::unique_ptr<ConfigurationWebServer>(new ConfigurationWebServer(configuration));
    configServer->setup();
  }
  Serial.println(" setup!");
}

void loop() {
  if(configuration->m_waterLeakConfiguration->isConfigurationComplete()) {
    Serial.println("loop!");
    Serial.println(WiFi.localIP());

    timePrinter->Print();
    
    smartThings->loop();
    delay(5 * 1000); //Give ST_Anything a chance to detect something before going back to sleep.

    Serial.println("Going to sleep for a minute.  Good-night.");
    //Soldering this on the ESP8266-01 is too hard for me :(
    //Went for the Adafruit HUZZAH ESP8266 Breakout instead
    //https://www.adafruit.com/product/2471
    //If the above link is dead, oh well.  Google it!

    //Time to save us some battery!  Remember, if GPIO 16 is NOT connected to the RST pin, this will hang your device and a hard reset will be required!
    //https://www.losant.com/blog/making-the-esp8266-low-powered-with-deep-sleep
    ESP.deepSleep(480e6); // Ne6 is N seconds
  }
  else if(configServer.get()) {
    configServer->loop();
    delay(1);
  }
  else {
    delay(3000);
    Serial.println("Couldn't do anything.  Clearing configuration and rebooting");
    
    configuration->clearAndSave();
    delay(3000);
    
    ESP.restart();
  }
}
