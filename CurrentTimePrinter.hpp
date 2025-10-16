#ifndef __CURRENTTIMEPRINTER_HPP__
#define __CURRENTTIMEPRINTER_HPP__

#include <time.h>

class CurrentTimePrinter {
public:
  CurrentTimePrinter(int timezone = 5, int dst = 1) {
    configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("\nWaiting for time");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println("");
  }
    
  void Print() {
    time_t now;
    struct tm* timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    Serial.print(timeinfo->tm_year + 1900); //Epoc is Jan 1, 1900
    Serial.print("-");
    Serial.print(timeinfo->tm_mon + 1); //Zero-based.  January would be 0 and December would be 11.  Add one ya dummy.
    Serial.print("-");
    Serial.print(timeinfo->tm_mday);
    Serial.print(" ");
    Serial.print(timeinfo->tm_hour);
    Serial.print(":");
    Serial.print(timeinfo->tm_min);
    Serial.print(":");
    Serial.print(timeinfo->tm_sec);
    Serial.println();
  }
};

#endif
