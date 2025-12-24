#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <lvgl.h>

// Time update intervals
#define TIME_DISPLAY_UPDATE_INTERVAL 1000  // Update display every 1 second
#define TIME_SYNC_INTERVAL 60000           // Sync with server every 60 seconds

class TimeManager {
 public:
  // Initialize time manager and fetch initial unix time
  void initTimeManager();

  // Get current unix time
  unsigned long getCurrentUnixTime();

  // Process time updates (display and sync)
  // Returns true if display was updated
  bool processTimeUpdates();

 private:
  // Fetch current unix time from API
  unsigned long fetchUnixTime();

  // Format unix timestamp to date/time string
  String formatUnixTime(unsigned long unixTime);

  // Update time from server
  void syncTimeFromServer();
};

#endif  // TIME_MANAGER_H
