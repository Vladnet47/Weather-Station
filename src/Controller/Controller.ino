#include <WiFi.h>
#include "Wire.h"
#include "Config.h"

#define WIND_SPEED_PIN      14
#define RAINFALL_PIN        25
#define WIND_DIRECTION_PIN  35
#define LED_PIN             5
#define FAILURE             -1
#define SUCCESS             0



// *********************************************************************************************
// Network and POST Request Information
// *********************************************************************************************

// Initialize the Ethernet client library
WiFiClient client;
unsigned long LAST_UPDATE_TIMESTAMP = 0;



// *********************************************************************************************
// Sensor Reading Variables and Interrupt Handlers
// *********************************************************************************************
// More information can be found here: https://learn.sparkfun.com/tutorials/weather-meter-hookup-guide

// Offset based on orientation of weather station, to make sure degrees = 0 points North
volatile int rainfallTicks = 0;
volatile unsigned long timeSinceLastWindSpeedTick = 0;
volatile unsigned long lastWindSpeedTick = 0;

// When rainfall or wind speed sensors register a tick, they cause an interrupt
void handleRainfallInterrupt(void)
{
    // Accumulate rain ticks
    ++rainfallTicks;
}

void handleWindSpeedInterrupt(void)
{
    // Record time between ticks
    timeSinceLastWindSpeedTick = millis() - lastWindSpeedTick;
    lastWindSpeedTick = millis();
}



// *********************************************************************************************
// Setup and Loop
// *********************************************************************************************

void setup()
{
    // Initialize serial and wait for port to open:
    Serial.begin(115200);
    while (!Serial);
    Wire.begin();

    // Initialize LED pin to be off
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Wind speed sensor
    pinMode(WIND_SPEED_PIN, INPUT);     
    attachInterrupt(digitalPinToInterrupt(WIND_SPEED_PIN), handleWindSpeedInterrupt, RISING);

    // Rain sensor
    pinMode(RAINFALL_PIN, INPUT);     
    attachInterrupt(digitalPinToInterrupt(RAINFALL_PIN), handleRainfallInterrupt, RISING);

    connectToWifiNetwork();
    
    // Blink three times upon successful connection
    blinkRepetitions(3, 300);
    printWifiStatus();
    Serial.println();
}

void loop() 
{
    // Read response from server
    String response = readResponse();
    if (response != "")
    {
        Serial.println(response);
    }

    // Send data to server
    if (timerElapsed(LAST_UPDATE_TIMESTAMP, UPDATE_INTERVAL))
    {
        // Calculate wind direction in degrees from North (based on current voltage input from sensor)
        float windDirection = convertWindDirToDegrees(analogRead(WIND_DIRECTION_PIN));

        // Calculate windspeed in miles per hour (based on time difference between sensor ticks)
        float windSpeed = 0;
        if (timeSinceLastWindSpeedTick != 0)
        {
            windSpeed = UPDATE_INTERVAL / timeSinceLastWindSpeedTick * 1.492;
        }

        // Calculate rainfall in inches (based on accumulated amount of sensor ticks)
        float rainfall = float(rainfallTicks) * 0.011;

        // POST data
        int requestStatus = postData(windDirection, windSpeed, rainfall);
        if (requestStatus == SUCCESS)
        {
            Serial.print("Posted: [wind speed = ");
            Serial.print(windSpeed);
            Serial.print(", wind direction = ");
            Serial.print(windDirection);
            Serial.print(", rainfall = ");
            Serial.print(rainfall);
            Serial.println("]");
            
            blinkRepetitions(2, 300);
            
            LAST_UPDATE_TIMESTAMP = millis();

            // Set rainfall to 0 between each successful request
            rainfallTicks = 0;
        }
        else
        {
            Serial.println("Failed to post data.");

            // Reconnect to WiFi in case network disconnected
            connectToWifiNetwork();
        }
    }
}



// *********************************************************************************************
// Helper Functions
// *********************************************************************************************

// Converts the voltage input from wind direction sensor to degrees
float convertWindDirToDegrees(int windDirection)
{
    if (windDirection < 150)
    {
        return 202.5;
    }
    else if (windDirection < 300)
    {
        return 180.0;
    }
    else if (windDirection < 400)
    {
        return 247.5;
    }
    else if (windDirection < 600)
    {
        return 225.0;
    }
    else if (windDirection < 900)
    {
        return 292.5;
    }
    else if (windDirection < 1100)
    {
        return 270.0;
    }
    else if (windDirection < 1500)
    {
        return 112.5;
    }
    else if (windDirection < 1700)
    {
        return 135.0;
    }
    else if (windDirection < 2250)
    {
        return 337.5;
    }
    else if (windDirection < 2350)
    {
        return 315.0;
    }
    else if (windDirection < 2700)
    {
        return 67.5;
    }
    else if (windDirection < 3000)
    {
        return 90.0;
    }
    else if (windDirection < 3200)
    {
        return 22.5;
    }
    else if (windDirection < 3400)
    {
        return 45.0;
    }
    return 0.0;
}

// Connects to server and sends POST request with given data in body
int postData(float windDirection, float windSpeed, float rainfall)
{
    // Close any prior connections to free the socket
    client.stop();

    // Try to connect and send POST request
    if (client.connect(SERVER, PORT))
    {
        // Body format: winddir=0&windspeed=0.0&rainfall=0
        String body = VARNAME_WIND_DIRECTION + "=" + String(windDirection);
        body += "&" + VARNAME_WIND_SPEED + "=" + String(windSpeed);
        body += "&" + VARNAME_RAINFALL + "=" + String(rainfall);

        // Format the POST request and send to client
        client.print("POST ");
        client.print(API_ENDPOINT);
        client.println(" HTTP/1.1");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(body.length());
        client.println();
        client.println(body);

        return SUCCESS;
    }

    return FAILURE;
}

// Returns POST response if available, empty string if not
String readResponse()
{
    String response = "";
    while (client.available()) 
    {
        char c = client.read();
        response += c;
    }
    
    return response;
}

// Returns true if current time has surpassed max time
// Works even when unsigned long overlaps
bool timerElapsed(unsigned long currentTime, unsigned long maxTime)
{
    return abs(millis() - currentTime) > maxTime;
}



// *********************************************************************************************
// Initialization Functions
// *********************************************************************************************

// Connects or reconnects to WiFi network
void connectToWifiNetwork()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial.println("Connecting to network...");
    int dotCount = 0;
    
    WiFi.begin(NETWORK, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(WIFI_CONNECT_RETRY_DELAY);

        // Print dot every second
        Serial.print(".");
        ++dotCount;
        if (dotCount > 60)
        {
            Serial.println();
            dotCount = 0;
        }
    }

    LAST_UPDATE_TIMESTAMP = millis();
    
    Serial.println();
    Serial.println("Connected to network!");
}

// Prints WiFi statistics (SSID, IP, MAC, Signal Strength)
void printWifiStatus() 
{
    Serial.println("||=========|| WiFi Status ||=========||");
    
    // Print the SSID of network
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // Print device's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // Print device's MAC address
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[5],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.println(mac[0],HEX);

    // Print signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");

    Serial.println("||===================================||");
}

// Blink for desired number of repetitions with delay (ms) between each blink.
void blinkRepetitions(int repetitions, int timeDelay) 
{
    for (int i = 0; i < repetitions; ++i)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
        delay(timeDelay);
    }
}
