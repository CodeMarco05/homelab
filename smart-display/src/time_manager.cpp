#include "time_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>

static unsigned long unixTime = 0;

void initTimeManager() {
  unixTime = fetchUnixTime();
  if (unixTime > 0) {
    Serial.print("Initial time: ");
    Serial.println(formatUnixTime(unixTime));
  }
}

unsigned long fetchUnixTime() {
  if (WiFi.status() != WL_CONNECTED) {
    return 0;
  }
  
  HTTPClient http;
  http.begin("https://digidates.de/api/v1/unixtime");
  
  int httpCode = http.GET();
  
  if (httpCode != HTTP_CODE_OK) {
    http.end();
    return 0;
  }
  
  String payload = http.getString();
  http.end();
  
  // Parse JSON manually: {"time": 1766526829}
  int startPos = payload.indexOf(":") + 1;
  int endPos = payload.indexOf("}");
  String timeStr = payload.substring(startPos, endPos);
  timeStr.trim();

  return timeStr.toInt() + 3600; // UTC+1 correction
}

String formatUnixTime(unsigned long unixTime) {
  if (unixTime == 0) {
    return "Invalid Time";
  }

  // Convert Unix timestamp to date/time
  time_t rawTime = (time_t)unixTime;
  struct tm* timeInfo = gmtime(&rawTime);

  // Weekdays in English
  const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

  // Format: Mon 12.03.2025 12:23:45
  char buffer[30];
  snprintf(buffer, sizeof(buffer), "%s %02d.%02d.%04d %02d:%02d:%02d",
           weekdays[timeInfo->tm_wday],
           timeInfo->tm_mday,
           timeInfo->tm_mon + 1,
           timeInfo->tm_year + 1900,
           timeInfo->tm_hour,
           timeInfo->tm_min,
           timeInfo->tm_sec);

  return String(buffer);
}

unsigned long getCurrentUnixTime() {
  return unixTime;
}

void incrementTime() {
  if (unixTime > 0) {
    unixTime++;
  }
}

void syncTimeFromServer() {
  unsigned long newTime = fetchUnixTime();
  if (newTime > 0) {
    unixTime = newTime;
    Serial.println("Time synchronized from server");
  }
}
