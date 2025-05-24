/**
 * @file MultipleDataPoints.ino
 * @brief Example of sending multiple data points using the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include <Paranode.h>
#include <DHT.h>

// WiFi credentials
const char *WIFI_SSID = "YourWiFiSSID";
const char *WIFI_PASSWORD = "YourWiFiPassword";

// Paranode credentials
const char *DEVICE_ID = "your-device-id";
const char *SECRET_KEY = "your-secret-key";
const char *SERVER_URL = "wss://api.paranode.io/ws";

// Initialize Paranode
Paranode paranode(DEVICE_ID, SECRET_KEY, SERVER_URL);

// DHT sensor
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Time tracking
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // 10 seconds

void setup()
{
    // Initialize serial
    Serial.begin(115200);
    while (!Serial)
    {
        ; // Wait for serial port to connect
    }

    Serial.println("\nParanode Multiple Data Points Example");

    // Initialize DHT sensor
    dht.begin();

    // Initialize Paranode
    if (!paranode.begin())
    {
        Serial.println("Failed to initialize Paranode");
        return;
    }

    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    if (paranode.connectWifi(WIFI_SSID, WIFI_PASSWORD))
    {
        Serial.println("connected!");
    }
    else
    {
        Serial.println("failed!");
        return;
    }

    // Set up callbacks
    paranode.onConnect([]()
                       { Serial.println("Connected to Paranode server"); });

    paranode.onDisconnect([]()
                          { Serial.println("Disconnected from Paranode server"); });

    // Connect to Paranode server
    Serial.print("Connecting to Paranode server...");
    if (paranode.connect())
    {
        Serial.println("initiated!");
    }
    else
    {
        Serial.println("failed!");
        return;
    }
}

void loop()
{
    // Process Paranode events
    paranode.loop();

    // Send sensor data every sendInterval
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval)
    {
        lastSendTime = currentTime;

        // Read DHT sensor
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        // Check if any reads failed
        if (isnan(humidity) || isnan(temperature))
        {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        // Calculate heat index
        float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

        // Send data individually
        if (paranode.isConnected())
        {
            Serial.println("Sending sensor data...");

            // Method 1: Send individual data points
            paranode.sendData("temperature", temperature);
            paranode.sendData("humidity", humidity);
            paranode.sendData("heat_index", heatIndex);

            // Method 2: Send multiple data points at once
            StaticJsonDocument<256> doc;
            JsonObject data = doc.to<JsonObject>();
            data["temperature"] = temperature;
            data["humidity"] = humidity;
            data["heat_index"] = heatIndex;

            paranode.sendData(data);

            Serial.println("Data sent!");
        }
    }
}