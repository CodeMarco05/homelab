#include "weather_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ui.h"
#include "time_manager.h"

static lv_chart_series_t* temp_series = NULL;

extern objects_t objects;

// Helper function to get current day of week (0=Sunday, 1=Monday, ..., 6=Saturday)
int getCurrentDayOfWeek() {
  unsigned long currentTime = getCurrentUnixTime();
  if (currentTime == 0) {
    return -1;  // Invalid
  }

  time_t rawTime = (time_t)currentTime;
  struct tm* timeInfo = gmtime(&rawTime);
  return timeInfo->tm_wday;  // 0=Sunday, 1=Monday, etc.
}

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

void updateWeeklyForecast() {
  // Fetch weather data
  String jsonData = fetchWeatherData();
  if (jsonData.isEmpty()) {
    Serial.println("Failed to fetch weather data for weekly forecast");
    return;
  }

  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);

  if (error) {
    Serial.printf("JSON parsing failed: %s\n", error.c_str());
    return;
  }

  // Extract hourly data
  JsonArray times = doc["hourly"]["time"];
  JsonArray temperatures = doc["hourly"]["temperature_2m"];

  if (times.isNull() || temperatures.isNull()) {
    Serial.println("No time or temperature data found");
    return;
  }

  // Array of weekday name label pointers (7 days)
  // Order: Mon=0, Tue=1, Wed=2, Thu=3, Fri=4, Sat=5, Sun=6
  lv_obj_t* dayNameLbls[] = {
    objects.mon, objects.tue, objects.wed,
    objects.thu, objects.fri, objects.sat, objects.sun
  };

  // Array of date label pointers (7 days)
  lv_obj_t* dateLbls[] = {
    objects.mon_date, objects.tue_date, objects.wed_date,
    objects.thu_date, objects.fri_date, objects.sat_date, objects.sun_date
  };

  // Array of degree label pointers (7 days)
  lv_obj_t* degreeLbls[] = {
    objects.mon_degree, objects.tue_degree, objects.wed_degree,
    objects.thu_degree, objects.fri_degree, objects.sat_degree, objects.sun_degree
  };

  // Get current day of week to determine which label to highlight
  // tm_wday: 0=Sunday, 1=Monday, 2=Tuesday, 3=Wednesday, 4=Thursday, 5=Friday, 6=Saturday
  int currentDayOfWeek = getCurrentDayOfWeek();

  // Convert tm_wday to array index (Mon-Sun array: 0=Mon, 1=Tue, ..., 6=Sun)
  // Sunday (0) -> 6, Monday (1) -> 0, Tuesday (2) -> 1, Wednesday (3) -> 2, etc.
  int currentDayIndex = (currentDayOfWeek == 0) ? 6 : (currentDayOfWeek - 1);

  // Process each of the 7 days from the forecast
  for (int day = 0; day < 7; day++) {
    int startIdx = day * 24;  // Each day has 24 hours

    // Check if we have enough data
    if (startIdx >= (int)times.size()) {
      break;
    }

    // Calculate which label index this forecast day should go to
    // If today is Wednesday (currentDayIndex=2), then:
    //   day 0 (today) -> index 2 (wed)
    //   day 1 (tomorrow) -> index 3 (thu)
    //   day 2 -> index 4 (fri)
    //   day 6 -> index 1 (tue, wrapping around)
    int labelIndex = (currentDayIndex + day) % 7;

    // Extract day of month from time string (format: "2025-12-23T00:00")
    String timeStr = times[startIdx].as<String>();
    int dayOfMonth = 0;

    // Parse day from "YYYY-MM-DD..." format
    if (timeStr.length() >= 10) {
      String dayStr = timeStr.substring(8, 10);  // Extract "DD"
      dayOfMonth = dayStr.toInt();
    }

    // Calculate min and max temperature for this day
    float minTemp = 999.0;
    float maxTemp = -999.0;

    for (int hour = 0; hour < 24 && (startIdx + hour) < (int)temperatures.size(); hour++) {
      float temp = temperatures[startIdx + hour];
      if (temp < minTemp) minTemp = temp;
      if (temp > maxTemp) maxTemp = temp;
    }

    // Update date label (day of month) at the correct position
    char dateBuffer[8];
    snprintf(dateBuffer, sizeof(dateBuffer), "%d", dayOfMonth);
    lv_label_set_text(dateLbls[labelIndex], dateBuffer);

    // Update degree label (min / max) at the correct position
    char degreeBuffer[16];
    snprintf(degreeBuffer, sizeof(degreeBuffer), "%d / %d", (int)minTemp, (int)maxTemp);
    lv_label_set_text(degreeLbls[labelIndex], degreeBuffer);
  }

  // Now apply highlighting to the current day's labels
  if (currentDayIndex >= 0 && currentDayIndex < 7) {
    // Set highlight color (custom hex color) for today's weekday name, date, and temperature
    lv_color_t highlightColor = lv_color_hex(0x4FB06D);

    lv_obj_set_style_text_color(dayNameLbls[currentDayIndex], highlightColor, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(dateLbls[currentDayIndex], highlightColor, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(degreeLbls[currentDayIndex], highlightColor, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Reset all other days to white
    for (int i = 0; i < 7; i++) {
      if (i != currentDayIndex) {
        lv_obj_set_style_text_color(dayNameLbls[i], lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(dateLbls[i], lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(degreeLbls[i], lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
    }

    Serial.printf("Highlighted day index: %d (day of week: %d)\n", currentDayIndex, currentDayOfWeek);
  }

  Serial.println("Weekly forecast updated");
}

