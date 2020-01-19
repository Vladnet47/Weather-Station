#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// Network
const char NETWORK[] = "network_name";					// MUST BE MODIFIED
const char PASSWORD[] = "network_password";				// MUST BE MODIFIED
const IPAddress SERVER(27, 0, 0, 1);					// MUST BE MODIFIED
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