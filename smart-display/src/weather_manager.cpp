#include "weather_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static lv_chart_series_t* temp_series = NULL;

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

void initWeatherChart(lv_obj_t* chart) {
  if (chart == NULL) return;

  // Configure chart
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(chart, 24);  // Show 24 hours
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -10, 45);  // Temperature range -10°C to 45°C

  // Enable chart division lines
  lv_chart_set_div_line_count(chart, 5, 6);

  // Create temperature series (blue line)
  temp_series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

  // Update chart mode to circular (will loop around when adding new points)
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

  Serial.println("Weather chart initialized");
}

void updateWeatherChart(lv_obj_t* chart) {
  if (chart == NULL || temp_series == NULL) {
    Serial.println("Chart not initialized!");
    return;
  }

  // Fetch weather data
  String jsonData = fetchWeatherData();
  if (jsonData.isEmpty()) {
    Serial.println("Failed to fetch weather data");
    return;
  }

  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);

  if (error) {
    Serial.printf("JSON parsing failed: %s\n", error.c_str());
    return;
  }

  // Extract temperature array from hourly data
  JsonArray temperatures = doc["hourly"]["temperature_2m"];

  if (temperatures.isNull()) {
    Serial.println("No temperature data found");
    return;
  }

  // Update chart with first 24 hours of data
  int pointsToShow = min(24, (int)temperatures.size());

  for (int i = 0; i < pointsToShow; i++) {
    float temp = temperatures[i];
    lv_chart_set_next_value(chart, temp_series, (int)temp);
  }

  // Refresh the chart
  lv_chart_refresh(chart);

  Serial.printf("Chart updated with %d temperature points\n", pointsToShow);

  // Print first few values for debugging
  Serial.print("First 6 temps: ");
  for (int i = 0; i < min(6, pointsToShow); i++) {
    Serial.printf("%.1f°C ", (float)temperatures[i]);
  }
  Serial.println();
}

