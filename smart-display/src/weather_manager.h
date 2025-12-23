#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <lvgl.h>

// Fetch weather data from Open-Meteo API
String fetchWeatherData();

// Initialize the weather chart with a temperature series
void initWeatherChart(lv_obj_t* chart);

// Update the weather chart with latest data
void updateWeatherChart(lv_obj_t* chart);

// Update weekly forecast labels (dates and min/max temperatures)
void updateWeeklyForecast();


#endif // WEATHER_MANAGER_H
