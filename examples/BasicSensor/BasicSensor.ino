/**
 * @file BasicSensor.ino
 * @brief Basic example of using the Paranode library with a sensor
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include <Paranode.h>

const char *WIFI_SSID = "YourWiFiSSID";
const char *WIFI_PASSWORD = "YourWiFiPassword";

const char *DEVICE_ID = "your-device-id";
const char *SECRET_KEY = "your-secret-key";
const char *SERVER_URL = "wss://api.paranode.io/ws";

Paranode paranode(DEVICE_ID, SECRET_KEY, SERVER_URL);

const int SENSOR_PIN = A0;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect
    }

    Serial.println("\nParanode Basic Sensor Example");

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

    paranode.onCommand([](const JsonObject &command)
                       {
    // Process command
    String action = command["action"];
    Serial.print("Received command: ");
    Serial.println(action);
    
    // Handle LED control command
    if (action == "led" && command.containsKey("value")) {
      bool ledState = command["value"];
      digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
      Serial.print("LED turned ");
      Serial.println(ledState ? "ON" : "OFF");
    } });

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

    // Set up LED pin
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    paranode.loop();

    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval)
    {
        lastSendTime = currentTime;

        // Read sensor
        int sensorValue = analogRead(SENSOR_PIN);

        // Send data
        if (paranode.isConnected())
        {
            Serial.print("Sending sensor value: ");
            Serial.println(sensorValue);

            paranode.sendData("sensor", sensorValue);
        }
    }
}