#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>

// Initialize time manager and fetch initial unix time
void initTimeManager();

// Fetch current unix time from API
unsigned long fetchUnixTime();

// Format unix timestamp to date/time string
String formatUnixTime(unsigned long unixTime);

// Get current unix time (call this every second)
unsigned long getCurrentUnixTime();

// Increment internal time by 1 second
void incrementTime();

// Update time from server (call periodically)
void syncTimeFromServer();

#endif // TIME_MANAGER_H
