#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

void ntptime_print(){
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    Serial.println(buffer);
  }
}

void ntptime_init(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
