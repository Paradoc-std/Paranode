/**
 * @file WebIntegration.ino
 * @brief Complete web app integration example for Paranode platform
 * @author Muhammad Daffa
 * @date 2025-10-25
 *
 * This example demonstrates integration with Paranode web app:
 *  - Token-based project authentication
 *  - Geolocation tracking
 *  - Remote WiFi configuration from web app
 *  - On/Off control from web dashboard
 *  - Real-time telemetry display
 *  - Device status updates
 *
 * How to use:
 * 1. Sign up at https://paranode.io
 * 2. Create a new project in your dashboard
 * 3. Copy your project token
 * 4. Replace PROJECT_TOKEN below with your token
 * 5. Upload this sketch to your ESP32/ESP8266
 * 6. Monitor your device from the web dashboard!
 *
 * Free tier: 3 projects included
 * Premium: Unlimited projects + advanced features
 */

#include <Paranode.h>

// ============ CONFIGURATION ============
// Get your project token from https://paranode.io/dashboard
const char* PROJECT_TOKEN = "your-project-token-here";  // <-- CHANGE THIS!

// Initial WiFi credentials (can be changed from web app)
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Hardware pins
const int LED_PIN = LED_BUILTIN;
const int RELAY_PIN = 5;

// ========================================

// Initialize Paranode with project token (NEW METHOD)
Paranode paranode(PROJECT_TOKEN);

// Device state
bool ledState = false;
bool relayState = false;
unsigned long lastTelemetry = 0;
const unsigned long telemetryInterval = 10000; // 10 seconds

// Geolocation (set your device location or use GPS module)
double latitude = 0.0;   // Set your latitude
double longitude = 0.0;  // Set your longitude

void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("\n========================================");
    Serial.println("   Paranode Web Integration Example");
    Serial.println("========================================\n");

    // Check if token is set
    if (String(PROJECT_TOKEN) == "your-project-token-here") {
        Serial.println("ERROR: Please set your PROJECT_TOKEN!");
        Serial.println("1. Go to https://paranode.io");
        Serial.println("2. Sign up and create a project");
        Serial.println("3. Copy your project token");
        Serial.println("4. Replace PROJECT_TOKEN in this sketch");
        while (1) delay(1000);
    }

    // Initialize hardware
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);

    // Initialize Paranode
    Serial.println("Initializing Paranode...");
    if (!paranode.begin()) {
        Serial.println("Failed to initialize!");
        while (1) delay(1000);
    }

    // Set device info
    paranode.setDeviceInfo("2.1.0", "ESP32-Web");

    // ===== Setup Callbacks =====

    // 1. Connection callback
    paranode.onConnect([]() {
        Serial.println("\n✓ Connected to Paranode Cloud!");
        Serial.println("View your device at: https://paranode.io/dashboard");

        // Send initial geolocation (if set)
        if (latitude != 0.0 && longitude != 0.0) {
            paranode.sendGeolocation(latitude, longitude);
            Serial.println("✓ Geolocation sent");
        }

        // Request project info
        paranode.requestProjectInfo();

        // Send initial device status
        StaticJsonDocument<256> metadata;
        metadata["device_name"] = "My ESP32 Device";
        metadata["location"] = "Home Office";
        metadata["version"] = "2.1.0";
        paranode.updateDeviceStatus(metadata.as<JsonObject>());
    });

    // 2. Disconnection callback
    paranode.onDisconnect([]() {
        Serial.println("\n✗ Disconnected from cloud");
    });

    // 3. Command callback (receives commands from web app)
    paranode.onCommand([](const JsonObject& cmd) {
        String action = cmd["action"];
        Serial.print("\n📥 Command from web app: ");
        Serial.println(action);

        String commandId = cmd["id"] | "";

        if (action == "led") {
            // Toggle LED from web dashboard
            ledState = cmd["value"] | false;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            Serial.print("LED: ");
            Serial.println(ledState ? "ON" : "OFF");

            // Send confirmation back
            paranode.sendCommandResponse(commandId, "SUCCESS",
                ledState ? "LED turned ON" : "LED turned OFF");

            // Update telemetry
            paranode.sendData("led_state", ledState);
        }
        else if (action == "relay") {
            // Control relay from web dashboard
            relayState = cmd["value"] | false;
            digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
            Serial.print("Relay: ");
            Serial.println(relayState ? "ON" : "OFF");

            paranode.sendCommandResponse(commandId, "SUCCESS",
                relayState ? "Relay turned ON" : "Relay turned OFF");

            paranode.sendData("relay_state", relayState);
        }
        else if (action == "update_location") {
            // Update geolocation from web app
            if (cmd.containsKey("latitude") && cmd.containsKey("longitude")) {
                latitude = cmd["latitude"];
                longitude = cmd["longitude"];
                Serial.printf("Location updated: %.6f, %.6f\n", latitude, longitude);

                paranode.sendCommandResponse(commandId, "SUCCESS", "Location updated");
            }
        }
        else if (action == "wifi_scan") {
            // Request WiFi networks scan
            paranode.sendCommandResponse(commandId, "SUCCESS", "WiFi scan started");
            performWiFiScan();
        }
        else {
            Serial.println("Unknown command");
            paranode.sendCommandResponse(commandId, "FAILED", "Unknown command");
        }
    });

    // 4. WiFi configuration callback (change WiFi from web app)
    paranode.onWiFiConfig([](const String& newSSID, const String& newPassword) {
        Serial.println("\n📶 WiFi configuration received from web app:");
        Serial.print("New SSID: ");
        Serial.println(newSSID);

        // Disconnect from current WiFi
        WiFi.disconnect();
        delay(100);

        // Connect to new WiFi
        Serial.print("Connecting to new WiFi...");
        if (paranode.connectWifi(newSSID.c_str(), newPassword.c_str())) {
            Serial.println(" Success!");
            Serial.print("New IP: ");
            Serial.println(WiFi.localIP());

            // Notify web app of successful connection
            paranode.sendStatus("ONLINE");
        } else {
            Serial.println(" Failed!");
            // Try to reconnect to old WiFi
            paranode.connectWifi(WIFI_SSID, WIFI_PASSWORD);
            paranode.sendError("Failed to connect to new WiFi", 102);
        }
    });

    // Connect to WiFi
    Serial.print("Connecting to WiFi");
    if (paranode.connectWifi(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println(" Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC Address: ");
        Serial.println(WiFi.macAddress());
    } else {
        Serial.println(" Failed!");
        while (1) delay(1000);
    }

    // Connect to Paranode cloud
    Serial.print("Connecting to Paranode cloud...");
    if (paranode.connect()) {
        Serial.println(" Connected!");
    }

    // Enable auto-reconnect
    paranode.setAutoReconnect(true);

    Serial.println("\n✓ Setup complete!");
    Serial.println("Monitor your device at: https://paranode.io/dashboard\n");
}

void loop()
{
    // CRITICAL: Must call loop() to process messages
    paranode.loop();

    // Send telemetry data periodically
    if (millis() - lastTelemetry >= telemetryInterval) {
        lastTelemetry = millis();
        sendTelemetryData();
    }
}

void sendTelemetryData()
{
    if (!paranode.isConnected()) {
        Serial.println("Not connected, skipping telemetry");
        return;
    }

    Serial.println("📊 Sending telemetry to web dashboard...");

    // Simulated sensor data (replace with real sensors)
    float temperature = 20.0 + (random(0, 100) / 10.0);
    float humidity = 50.0 + (random(0, 300) / 10.0);
    int rssi = WiFi.RSSI();
    uint32_t freeHeap = ESP.getFreeHeap();

    // Send individual data points
    paranode.sendData<float>("temperature", temperature, "°C");
    paranode.sendData<float>("humidity", humidity, "%");
    paranode.sendData<int>("wifi_rssi", rssi, "dBm");
    paranode.sendData<bool>("led_state", ledState);
    paranode.sendData<bool>("relay_state", relayState);

    // Send device metrics
    paranode.sendMetrics(freeHeap, rssi);

    // Update geolocation (if it changes - e.g., mobile device with GPS)
    if (latitude != 0.0 && longitude != 0.0) {
        // Only send geolocation every 5 minutes to save bandwidth
        static unsigned long lastGeoUpdate = 0;
        if (millis() - lastGeoUpdate > 300000) {
            paranode.sendGeolocation(latitude, longitude, 10.0); // 10m accuracy
            lastGeoUpdate = millis();
        }
    }

    Serial.println("✓ Telemetry sent");
}

void performWiFiScan()
{
    Serial.println("Scanning WiFi networks...");
    int n = WiFi.scanNetworks();

    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.printf("Found %d networks:\n", n);

        // Send scan results to web app
        StaticJsonDocument<512> doc;
        JsonArray networks = doc.createNestedArray("networks");

        for (int i = 0; i < n && i < 5; i++) { // Max 5 networks
            JsonObject net = networks.createNestedObject();
            net["ssid"] = WiFi.SSID(i);
            net["rssi"] = WiFi.RSSI(i);
            net["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "secured";

            Serial.printf("  %d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }

        // Send to web app (you'll see this in your dashboard)
        String message;
        serializeJson(doc, message);
        // You could send this as a custom telemetry message
    }

    WiFi.scanDelete();
}

/**
 * WEB APP FEATURES YOU CAN USE:
 *
 * 1. Real-time Dashboard
 *    - View all telemetry data in real-time
 *    - See device online/offline status
 *    - Monitor WiFi signal strength
 *    - Track device location on map
 *
 * 2. Remote Control
 *    - Toggle LED on/off
 *    - Control relay
 *    - Send custom commands
 *
 * 3. WiFi Management
 *    - Change WiFi credentials remotely
 *    - Scan for networks
 *    - Monitor connection status
 *
 * 4. Geolocation
 *    - View device location on map
 *    - Update location from web app
 *    - Track movement (if GPS enabled)
 *
 * 5. Project Management
 *    - Free tier: 3 projects
 *    - Manage multiple devices per project
 *    - View historical data
 *    - Set alerts and notifications
 *
 * 6. Analytics
 *    - View telemetry history
 *    - Export data
 *    - Create custom dashboards
 *
 * Get started at: https://paranode.io
 */
