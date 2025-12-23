#include "weather_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>

String fetchWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return "";
  }
  
  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=53.607&longitude=9.9054&hourly=temperature_2m,rain,showers,snowfall";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return "";
  }
  
  String payload = http.getString();
  http.end();
  
  Serial.println("Weather data fetched successfully");
  return payload;
}

