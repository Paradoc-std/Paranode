/**
 * @file Paranode.h
 * @brief Enhanced main header file for the Paranode IoT platform library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#ifndef PARANODE_H
#define PARANODE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <vector>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error "This library only supports ESP8266 and ESP32 boards."
#endif

#include "Paranode/Connection/ParanodeConnection.h"
#include "Paranode/Wifi/ParanodeWifi.h"
#include "Paranode/Socket/ParanodeSocket.h"
#include "Paranode/Utils/ParanodeJsonBuilder.h"
#include "Paranode/Utils/ParanodeMessageQueue.h"

typedef std::function<void(const JsonObject &)> CommandCallback;
typedef std::function<void(void)> ConnectionCallback;
typedef std::function<void(const String &)> OTACallback;
typedef std::function<void(int)> OTAProgressCallback;

/**
 * @class Paranode
 * @brief Main class for the Paranode IoT platform
 */
class Paranode
{
public:
    /**
     * @brief Constructor (Legacy - for backward compatibility)
     * @param deviceId Unique identifier for the device
     * @param secretKey Secret key for authentication
     * @param serverUrl URL of the Paranode server
     * @deprecated Use Paranode(projectToken) for new projects
     */
    Paranode(const String &deviceId, const String &secretKey, const String &serverUrl = "wss://api.paranode.io/ws");

    /**
     * @brief Constructor with Project Token (Recommended)
     * @param projectToken Unique project token from paranode.io dashboard
     * @param serverUrl Optional custom server URL
     *
     * How to get your project token:
     * 1. Sign up at https://paranode.io
     * 2. Create a new project in your dashboard
     * 3. Copy the project token
     * 4. Use it here: Paranode paranode("your-project-token");
     *
     * Free tier includes 3 projects. Upgrade for unlimited projects.
     */
    Paranode(const String &projectToken, bool useTokenAuth = true);

    /**
     * @brief Initialize the Paranode library
     * @return True if initialization is successful, false otherwise
     */
    bool begin();

    /**
     * @brief Connect to WiFi
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @param timeout Connection timeout in milliseconds
     * @return True if connection is successful, false otherwise
     */
    bool connectWifi(const char *ssid, const char *password, unsigned long timeout = 30000);

    /**
     * @brief Connect to the Paranode server
     * @return True if connection is successful, false otherwise
     */
    bool connect();

    /**
     * @brief Check if connected to the Paranode server
     * @return True if connected, false otherwise
     */
    bool isConnected();

    /**
     * @brief Send telemetry data to the server (optimized template version)
     * @param key Data key
     * @param value Data value (int, float, bool, String, const char*)
     * @param unit Optional unit of measurement
     * @param useQueue If true, queue message for batching (default: false)
     * @return True if data is sent successfully, false otherwise
     */
    template<typename T>
    bool sendData(const char* key, const T& value, const char* unit = "", bool useQueue = false);

    // Backward compatibility wrappers
    bool sendData(const String &key, float value, const String &unit = "");
    bool sendData(const String &key, int value, const String &unit = "");
    bool sendData(const String &key, const String &value, const String &unit = "");
    bool sendData(const String &key, bool value, const String &unit = "");

    /**
     * @brief Send multiple telemetry points data to the server
     * @param json JSON object containing data points
     * @return True if data is sent successfully, false otherwise
     */
    bool sendData(const JsonObject &json);

    /**
     * @brief Flush queued messages (send all buffered messages)
     * @return Number of messages sent
     */
    int flushQueue();

    /**
     * @brief Enable/disable message batching
     * @param enable True to enable batching
     * @param batchSize Number of messages to batch together
     */
    void setBatching(bool enable, int batchSize = 5);

    /**
     * @brief Get number of queued messages
     */
    size_t getQueuedCount() const;

    /**
     * @brief Send device status update
     * @param status Device status (ONLINE, OFFLINE, MAINTENANCE, ERROR, UPDATING)
     * @return True if status is sent successfully, false otherwise
     */
    bool sendStatus(const String &status);

    /**
     * @brief Send error log to server
     * @param errorMessage Error message
     * @param errorCode Optional error code
     * @return True if error is sent successfully, false otherwise
     */
    bool sendError(const String &errorMessage, int errorCode = 0);

    /**
     * @brief Send performance metrics
     * @param freeHeap Free heap memory
     * @param rssi WiFi RSSI value
     * @return True if metrics are sent successfully, false otherwise
     */
    bool sendMetrics(uint32_t freeHeap, int rssi);

    /**
     * @brief Set device information
     * @param firmwareVersion Firmware version
     * @param hardwareVersion Hardware version
     */
    void setDeviceInfo(const String &firmwareVersion, const String &hardwareVersion);

    /**
     * @brief Set device MAC address (auto-detected if not set)
     * @param macAddress Device MAC address
     */
    void setMacAddress(const String &macAddress);

    /**
     * @brief Set callback for commands received from the server
     * @param callback Function to be called when a command is received
     */
    void onCommand(CommandCallback callback);

    /**
     * @brief Set callback for successful connection to the server
     * @param callback Function to be called when connection is established
     */
    void onConnect(ConnectionCallback callback);

    /**
     * @brief Set callback for disconnection from the server
     * @param callback Function to be called when disconnected from the server
     */
    void onDisconnect(ConnectionCallback callback);

    /**
     * @brief Set callback for OTA update notifications
     * @param callback Function to be called when OTA update is available
     */
    void onOTAUpdate(OTACallback callback);

    /**
     * @brief Set callback for OTA progress updates
     * @param callback Function to be called with progress percentage (0-100)
     */
    void onOTAProgress(OTAProgressCallback callback);

    /**
     * @brief Request configuration from server
     * @return True if request is sent successfully, false otherwise
     */
    bool requestConfig();

    /**
     * @brief Send command response/acknowledgment
     * @param commandId Original command ID
     * @param status Command execution status
     * @param response Optional response message
     * @return True if response is sent successfully, false otherwise
     */
    bool sendCommandResponse(const String &commandId, const String &status, const String &response = "");

    /**
     * @brief Enable/disable auto-reconnect
     * @param enable True to enable auto-reconnect
     */
    void setAutoReconnect(bool enable);

    /**
     * @brief Set heartbeat interval
     * @param interval Interval in milliseconds (minimum 10000)
     */
    void setHeartbeatInterval(unsigned long interval);

    /**
     * @brief Get device uptime in seconds
     * @return Uptime in seconds
     */
    unsigned long getUptime();

    /**
     * @brief Send geolocation data
     * @param latitude Device latitude
     * @param longitude Device longitude
     * @param accuracy Optional accuracy in meters
     * @return True if sent successfully
     */
    bool sendGeolocation(double latitude, double longitude, float accuracy = 0.0);

    /**
     * @brief Request WiFi configuration change from server
     * Triggers onWiFiConfig callback when server sends new WiFi credentials
     * @return True if request sent successfully
     */
    bool requestWiFiConfig();

    /**
     * @brief Set callback for WiFi configuration updates from web app
     * @param callback Function called with new SSID and password
     */
    void onWiFiConfig(std::function<void(const String &ssid, const String &password)> callback);

    /**
     * @brief Update device online status and metadata
     * @param metadata JSON object with additional device info (location, IP, etc.)
     * @return True if sent successfully
     */
    bool updateDeviceStatus(const JsonObject &metadata);

    /**
     * @brief Get project information from server
     * @return True if request sent successfully
     */
    bool requestProjectInfo();

    /**
     * @brief Process incoming messages and maintain connection
     * @note This function must be called in the loop() function
     */
    void loop();

private:
    String _deviceId;
    String _secretKey;
    String _projectToken;
    String _serverUrl;
    String _macAddress;
    String _firmwareVersion;
    String _hardwareVersion;
    bool _isConnected;
    bool _isAuthenticated;
    bool _autoReconnect;
    bool _useTokenAuth;
    unsigned long _startTime;

    ParanodeWifi _wifi;
    ParanodeSocket _socket;
    ParanodeConnection _connection;
    ParanodeMessageQueue _messageQueue;

    CommandCallback _commandCallback;
    ConnectionCallback _connectCallback;
    ConnectionCallback _disconnectCallback;
    OTACallback _otaCallback;
    OTAProgressCallback _otaProgressCallback;
    std::function<void(const String &, const String &)> _wifiConfigCallback;

    unsigned long _lastHeartbeatTime;
    unsigned long _heartbeatInterval;
    unsigned long _lastMetricsTime;
    unsigned long _metricsInterval;

    // Optimization: Reusable buffers to avoid repeated allocations
    char _messageBuffer[PARANODE_MAX_MESSAGE_SIZE];
    char _batchBuffer[1024];

    // Batching configuration
    bool _batchingEnabled;
    int _batchSize;
    unsigned long _lastBatchTime;
    unsigned long _batchInterval;

    void handleMessage(const String &message);
    void sendHeartbeat();
    bool authenticate();
    void sendDeviceInfo();
    void handleOTAUpdate(const JsonObject &update);
    void handleConfig(const JsonObject &config);
    String getDefaultMacAddress();

    // Optimized message sending
    bool sendMessageDirect(const char* message);
    bool sendMessageQueued(const char* message, uint8_t priority = 1);
    void processQueue();

    // Template implementation helpers
    template<typename T>
    bool buildAndSendMessage(const char* key, const T& value, const char* unit, bool useQueue);
};

// Template implementation (must be in header)
template<typename T>
bool Paranode::sendData(const char* key, const T& value, const char* unit, bool useQueue) {
    return buildAndSendMessage(key, value, unit, useQueue);
}

// Specialized template implementations
template<>
inline bool Paranode::buildAndSendMessage<int>(const char* key, const int& value, const char* unit, bool useQueue) {
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "telemetry");
    builder.addString("key", key);
    builder.addInt("value", value);
    if (unit && unit[0] != '\0') {
        builder.addString("unit", unit);
    }
    builder.addULong("timestamp", millis());
    builder.endObject();

    return useQueue ? sendMessageQueued(builder.getJson()) : sendMessageDirect(builder.getJson());
}

template<>
inline bool Paranode::buildAndSendMessage<float>(const char* key, const float& value, const char* unit, bool useQueue) {
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "telemetry");
    builder.addString("key", key);
    builder.addFloat("value", value);
    if (unit && unit[0] != '\0') {
        builder.addString("unit", unit);
    }
    builder.addULong("timestamp", millis());
    builder.endObject();

    return useQueue ? sendMessageQueued(builder.getJson()) : sendMessageDirect(builder.getJson());
}

template<>
inline bool Paranode::buildAndSendMessage<bool>(const char* key, const bool& value, const char* unit, bool useQueue) {
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "telemetry");
    builder.addString("key", key);
    builder.addBool("value", value);
    if (unit && unit[0] != '\0') {
        builder.addString("unit", unit);
    }
    builder.addULong("timestamp", millis());
    builder.endObject();

    return useQueue ? sendMessageQueued(builder.getJson()) : sendMessageDirect(builder.getJson());
}

template<>
inline bool Paranode::buildAndSendMessage<const char*>(const char* key, const char* const& value, const char* unit, bool useQueue) {
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "telemetry");
    builder.addString("key", key);
    builder.addString("value", value);
    if (unit && unit[0] != '\0') {
        builder.addString("unit", unit);
    }
    builder.addULong("timestamp", millis());
    builder.endObject();

    return useQueue ? sendMessageQueued(builder.getJson()) : sendMessageDirect(builder.getJson());
}

template<>
inline bool Paranode::buildAndSendMessage<String>(const char* key, const String& value, const char* unit, bool useQueue) {
    return buildAndSendMessage(key, value.c_str(), unit, useQueue);
}

#endif