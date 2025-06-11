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
     * @brief Constructor
     * @param deviceId Unique identifier for the device
     * @param secretKey Secret key for authentication (or API key)
     * @param serverUrl URL of the Paranode server
     */
    Paranode(const String &deviceId, const String &secretKey, const String &serverUrl = "wss://api.paranode.io/ws");

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
     * @brief Send telemetry data to the server
     * @param key Data key
     * @param value Data value
     * @param unit Optional unit of measurement
     * @return True if data is sent successfully, false otherwise
     */
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
     * @brief Process incoming messages and maintain connection
     * @note This function must be called in the loop() function
     */
    void loop();

private:
    String _deviceId;
    String _secretKey;
    String _serverUrl;
    String _macAddress;
    String _firmwareVersion;
    String _hardwareVersion;
    bool _isConnected;
    bool _isAuthenticated;
    bool _autoReconnect;
    unsigned long _startTime;

    ParanodeWifi _wifi;
    ParanodeSocket _socket;
    ParanodeConnection _connection;

    CommandCallback _commandCallback;
    ConnectionCallback _connectCallback;
    ConnectionCallback _disconnectCallback;
    OTACallback _otaCallback;
    OTAProgressCallback _otaProgressCallback;

    unsigned long _lastHeartbeatTime;
    unsigned long _heartbeatInterval;
    unsigned long _lastMetricsTime;
    unsigned long _metricsInterval;

    void handleMessage(const String &message);
    void sendHeartbeat();
    bool authenticate();
    void sendDeviceInfo();
    void handleOTAUpdate(const JsonObject &update);
    void handleConfig(const JsonObject &config);
    String getDefaultMacAddress();
};

#endif