/**
 * @file ParanodeFullExample.ino
 * @brief Complete example demonstrating all features of the Paranode
 * @author Muhammad Daffa
 * @date 2025-06-11
 *
 * This example demonstrates:
 *  - Device authentication and connection
 *  - Telemetry data sending with units
 *  - Command handling with response
 *  - OTA update handling
 *  - Error reporting
 *  - Performance metrics
 *  - Device status updates
 *  - Configuration management
 */

#include <Paranode.h>

// WiFi credentials
const char *WIFI_SSID = "yourWiFiSSID";
const char *WIFI_PASSWORD = "yourWiFiPassword";

// Paranode credentials
const char *DEVICE_ID = "device-example-001";
const char *API_KEY = "your-api-key-here";

// Initialize Paranode
Paranode paranode(DEVICE_ID, API_KEY);

// Hardware pin definitions
const int LED_PIN = LED_BUILTIN;
const int RELAY_PIN = 5;
const int TEMP_SENSOR_PIN = A0;
const int HUMIDITY_SENSOR_PIN = A1;
const int BUTTON_PIN = 0;

// Sensor simulation (replace with real sensor readings)
float temperature = 25.0;
float humidity = 60.0;
bool ledState = false;
bool relayState = false;
bool sensorError = false;
int errorCount = 0;

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastStatusUpdate = 0;
const unsigned long sensorInterval = 5000;
const unsigned long statusInterval = 30000;

// OTA Update variables
bool otaInProgress = false;
String otaUrl = "";

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect
    }

    Serial.println("\n========================================");
    Serial.println("Paranode IoT Platform - Full Example");
    Serial.println("========================================\n");

    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);

    Serial.println("Initializing Paranode...");
    if (!paranode.begin())
    {
        Serial.println("Failed to initialize Paranode. Check your API key and device ID.");
        while (1)
        {
            delay(1000);
        }
    }

    Serial.println("Paranode initialized successfully.");

    // Set device information
    paranode.setDeviceInfo("2.0.0", "ESP32-DevKit-v1");

    // Configure event callbacks
    setupCallbacks();

    // Connect to WiFi
    Serial.print("\nConnecting to WiFi");
    if (paranode.connectWifi(WIFI_SSID, WIFI_PASSWORD))
    {
        Serial.println(" Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println(" Failed!");
        Serial.println("Please check your WiFi credentials");
        while (1)
        {
            delay(1000);
        }
    }

    // Connect to Paranode server
    Serial.print("Connecting to Paranode server...");
    if (paranode.connect())
    {
        Serial.println(" Connection initiated!");
    }
    else
    {
        Serial.println(" Failed to initiate connection!");
    }

    // Configure Paranode settings
    paranode.setAutoReconnect(true);
    paranode.setHeartbeatInterval(30000); // 30 seconds

    Serial.println("\nSetup complete! Waiting for commands... \n");
}

void loop()
{
    paranode.loop();

    if (otaInProgress)
    {
        handleOTAUpdate();
        return;
    }

    if (millis() - lastSensorRead >= sensorInterval)
    {
        lastSensorRead = millis();
        readAndSendSensorData();
    }

    if (millis() - lastStatusUpdate >= statusInterval)
    {
        lastStatusUpdate = millis();
        sendDeviceStatus();
    }

    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH)
    {
        Serial.println("Button pressed - Toggling LED");
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);

        paranode.sendData("button_pressed", true);
        paranode.sendData("led_state", ledState);
    }
    lastButtonState = buttonState;

    static unsigned long lastErrorCheck = 0;
    if (millis() - lastErrorCheck > 20000)
    { // Check every 20 seconds
        lastErrorCheck = millis();
        if (random(0, 10) == 0)
        { // 10% chance of error
            simulateSensorError();
        }
    }
}

void setupCallbacks()
{
    // Connection callback
    paranode.onConnect([]()
                       {
        Serial.println("\n✓ Connected to Paranode server!");
        Serial.println("Requesting device configuration...");
        paranode.requestConfig();
        
        // Send initial status
        paranode.sendStatus("ONLINE"); });

    // Disconnection callback
    paranode.onDisconnect([]()
                          {
                              Serial.println("\n✗ Disconnected from Paranode server!");
                              otaInProgress = false; // Cancel any OTA in progress
                          });

    // Command callback
    paranode.onCommand([](const JsonObject &cmd)
                       {
        String commandId = cmd["id"] | "";
        String action = cmd["action"] | "";
        
        Serial.println("\n📥 Received command:");
        Serial.print("  ID: ");
        Serial.println(commandId);
        Serial.print("  Action: ");
        Serial.println(action);
        
        bool success = processCommand(action, cmd);
        
        // Send command response
        paranode.sendCommandResponse(
            commandId,
            success ? "SUCCESS" : "FAILED",
            success ? "Command executed successfully" : "Command execution failed"
        ); });

    // OTA Update callback
    paranode.onOTAUpdate([](const String &url)
                         {
        Serial.println("\n🔄 OTA Update available!");
        Serial.print("  URL: ");
        Serial.println(url);
        
        otaUrl = url;
        otaInProgress = true;
        
        // Send status update
        paranode.sendStatus("UPDATING");
        
        Serial.println("Starting OTA update process..."); });

    // OTA Progress callback
    paranode.onOTAProgress([](int progress)
                           {
        Serial.print("OTA Progress: ");
        Serial.print(progress);
        Serial.println("%");
        
        // Blink LED to indicate progress
        digitalWrite(LED_PIN, (progress % 10 < 5) ? HIGH : LOW); });
}

bool processCommand(const String &action, const JsonObject &cmd)
{
    Serial.println("Processing command: " + action);

    if (action == "led")
    {
        if (cmd.containsKey("value"))
        {
            ledState = cmd["value"].as<bool>();
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            Serial.print("LED turned ");
            Serial.println(ledState ? "ON" : "OFF");

            // Send telemetry update
            paranode.sendData("led_state", ledState);
            return true;
        }
    }
    else if (action == "relay")
    {
        if (cmd.containsKey("value"))
        {
            relayState = cmd["value"].as<bool>();
            digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
            Serial.print("Relay turned ");
            Serial.println(relayState ? "ON" : "OFF");

            // Send telemetry update
            paranode.sendData("relay_state", relayState);
            return true;
        }
    }
    else if (action == "reset")
    {
        Serial.println("Resetting device...");
        paranode.sendStatus("MAINTENANCE");
        delay(1000);
        ESP.restart();
        return true;
    }
    else if (action == "get_status")
    {
        // Send comprehensive status
        sendDeviceStatus();
        return true;
    }
    else if (action == "clear_errors")
    {
        errorCount = 0;
        sensorError = false;
        Serial.println("Error count cleared");
        return true;
    }
    else if (action == "set_interval")
    {
        // Example of configuration command
        if (cmd.containsKey("sensor_interval"))
        {
            unsigned long newInterval = cmd["sensor_interval"].as<unsigned long>() * 1000;
            if (newInterval >= 1000 && newInterval <= 3600000)
            { // 1s to 1h
                // Store this in non-volatile memory in real implementation
                Serial.print("Sensor interval updated to: ");
                Serial.print(newInterval / 1000);
                Serial.println(" seconds");
                return true;
            }
        }
    }

    Serial.println("Unknown command or missing parameters");
    return false;
}

void readAndSendSensorData()
{
    if (paranode.isConnected())
    {
        // Simulate reading sensors (replace with actual sensor code)
        temperature = 20.0 + (random(0, 100) / 10.0); // 20.0 to 30.0°C
        humidity = 40.0 + (random(0, 400) / 10.0);    // 40.0 to 80.0%

        // Check for sensor errors
        if (!sensorError)
        {
            Serial.println("\n📊 Sending sensor data:");
            Serial.print("  Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");
            Serial.print("  Humidity: ");
            Serial.print(humidity);
            Serial.println("%");

            // Send individual data points with units
            paranode.sendData("temperature", temperature, "°C");
            paranode.sendData("humidity", humidity, "%");

            // Calculate heat index
            float heatIndex = calculateHeatIndex(temperature, humidity);
            paranode.sendData("heat_index", heatIndex, "°C");

            // Send multiple data points at once
            StaticJsonDocument<256> doc;
            JsonObject data = doc.to<JsonObject>();
            data["temperature"] = temperature;
            data["humidity"] = humidity;
            data["heat_index"] = heatIndex;
            data["led_state"] = ledState;
            data["relay_state"] = relayState;

            paranode.sendData(data);
        }
        else
        {
            Serial.println("\n⚠️ Sensor error - skipping data send");
        }
    }
}

void sendDeviceStatus()
{
    if (paranode.isConnected())
    {
        Serial.println("\n📡 Sending device status");

        // Send device status
        paranode.sendStatus(sensorError ? "ERROR" : "ONLINE");

        // Send performance metrics
        uint32_t freeHeap = ESP.getFreeHeap();
        int rssi = WiFi.RSSI();
        paranode.sendMetrics(freeHeap, rssi);

        // Send additional status information
        StaticJsonDocument<256> doc;
        JsonObject status = doc.to<JsonObject>();
        status["uptime"] = paranode.getUptime();
        status["error_count"] = errorCount;
        status["sensor_error"] = sensorError;
        status["wifi_connected"] = WiFi.isConnected();
        status["free_heap"] = freeHeap;
        status["rssi"] = rssi;

        paranode.sendData(status);
    }
}

void simulateSensorError()
{
    sensorError = true;
    errorCount++;

    Serial.println("\n❌ Simulated sensor error!");

    // Report error
    paranode.sendError("Sensor read failed - timeout", 101);
    paranode.sendStatus("ERROR");

    // Auto-recover after 5 seconds
    static unsigned long errorTime = millis();
    if (millis() - errorTime > 5000)
    {
        sensorError = false;
        Serial.println("✓ Sensor recovered");
        paranode.sendStatus("ONLINE");
    }
}

float calculateHeatIndex(float temperature, float humidity)
{
    // Simplified heat index calculation
    float hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (humidity * 0.094));

    if (hi > 79)
    {
        hi = -42.379 + 2.04901523 * temperature + 10.14333127 * humidity - 0.22475541 * temperature * humidity - 0.00683783 * temperature * temperature - 0.05481717 * humidity * humidity + 0.00122874 * temperature * temperature * humidity + 0.00085282 * temperature * humidity * humidity - 0.00000199 * temperature * temperature * humidity * humidity;
    }

    return hi;
}

void handleOTAUpdate()
{
    // This is a simplified OTA handler
    // In a real implementation, you would:
    // 1. Download the firmware from otaUrl
    // 2. Verify the firmware integrity
    // 3. Write to flash
    // 4. Reboot

    static unsigned long otaStartTime = millis();
    static int progress = 0;

    // Simulate OTA progress
    if (millis() - otaStartTime > 100)
    {
        otaStartTime = millis();
        progress += 2;

        if (progress <= 100)
        {
            // Send progress update
            if (paranode.isConnected())
            {
                StaticJsonDocument<128> doc;
                doc["type"] = "ota_progress";
                doc["progress"] = progress;
                String message;
                serializeJson(doc, message);
                // paranode would send this internally
            }

            // Visual feedback
            digitalWrite(LED_PIN, (progress % 10 < 5) ? HIGH : LOW);
        }
        else
        {
            // OTA complete
            Serial.println("\n✓ OTA Update completed!");
            Serial.println("Rebooting...");

            paranode.sendStatus("ONLINE");
            delay(1000);
            ESP.restart();
        }
    }
}