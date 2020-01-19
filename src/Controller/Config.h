#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// MUST BE MODIFIED
const char NETWORK[] = "network_name";						
const char PASSWORD[] = "network_password";
const IPAddress SERVER(27, 0, 0, 1);
// END

// Network
const unsigned long WIFI_CONNECT_RETRY_DELAY = 1000;
const int PORT = 8080;

// POST request frequency (ms)
const unsigned long UPDATE_INTERVAL = 10000;

// POST request to server API
const String API_ENDPOINT = "/weatherstation/windrain";
const String VARNAME_WIND_DIRECTION = "winddir";
const String VARNAME_WIND_SPEED = "windspeed";
const String VARNAME_RAINFALL = "rainfall";

#endif