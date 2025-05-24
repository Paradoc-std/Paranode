/**
 * @file CommandListener.ino
 * @brief Example of handling commands using the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include <Paranode.h>

// WiFi credentials
const char *WIFI_SSID = "YourWiFiSSID";
const char *WIFI_PASSWORD = "YourWiFiPassword";

// Paranode credentials
const char *DEVICE_ID = "your-device-id";
const char *SECRET_KEY = "your-secret-key";
const char *SERVER_URL = "wss://api.paranode.io/ws";

// Initialize Paranode
Paranode paranode(DEVICE_ID, SECRET_KEY, SERVER_URL);

// Define pins
const int LED_PIN = LED_BUILTIN;
const int RELAY_PIN = 5;

void setup()
{
    // Initialize serial
    Serial.begin(115200);
    while (!Serial)
    {
        ; // Wait for serial port to connect
    }

    Serial.println("\nParanode Command Listener Example");

    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    // Set initial state
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);

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

    // Set up command handler
    paranode.onCommand([](const JsonObject &command)
                       {
     // Process command
     String action = command["action"];
     Serial.print("Received command: ");
     Serial.println(action);
     
     // Handle LED control command
     if (action == "led" && command.containsKey("value")) {
       bool ledState = command["value"];
       digitalWrite(LED_PIN, ledState ? HIGH : LOW);
       Serial.print("LED turned ");
       Serial.println(ledState ? "ON" : "OFF");
       
       // Send response back to server
       StaticJsonDocument<128> doc;
       JsonObject response = doc.to<JsonObject>();
       response["led_state"] = ledState;
       paranode.sendData("led_response", response);
     }
     
     // Handle relay control command
     else if (action == "relay" && command.containsKey("value")) {
       bool relayState = command["value"];
       digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
       Serial.print("Relay turned ");
       Serial.println(relayState ? "ON" : "OFF");
       
       // Send response back to server
       StaticJsonDocument<128> doc;
       JsonObject response = doc.to<JsonObject>();
       response["relay_state"] = relayState;
       paranode.sendData("relay_response", response);
     }
     
     // Handle unknown command
     else {
       Serial.println("Unknown command or missing parameters");
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
}

void loop()
{
    // Process Paranode events
    paranode.loop();
}