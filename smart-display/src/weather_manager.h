#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <lvgl.h>

// Weather update interval (5 minutes in milliseconds)
#define WEATHER_UPDATE_INTERVAL 300000

class WeatherManager {
 public:
  // Check if weather needs updating and update if necessary
  // Returns true if an update was performed
  bool updateWeatherData();

 private:
  // Fetch weather data from Open-Meteo API
  String fetchWeatherData();

  // Update weekly forecast labels (dates and min/max temperatures)
  void updateWeeklyForecast();

  // Initialize the weather chart with a temperature series
  // void initWeatherChart(lv_obj_t* chart);

  // Update the weather chart with latest data
  // void updateWeatherChart(lv_obj_t* chart);
};

#endif  // WEATHER_MANAGER_H
