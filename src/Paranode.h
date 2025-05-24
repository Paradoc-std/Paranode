/**
 * @file Paranode.h
 * @brief Main header file for the Paranode IoT platform library
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
     * @param secretKey Secret key for authentication
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
     * @return True if data is sent successfully, false otherwise
     */
    bool sendData(const String &key, float value);

    /**
     * @brief Send telemetry data to the server
     * @param key Data key
     * @param value Data value
     * @return True if data is sent successfully, false otherwise
     */
    bool sendData(const String &key, int value);

    /**
     * @brief Send telemetry data to the server
     * @param key Data key
     * @param value Data value
     * @return True if data is sent successfully, false otherwise
     */
    bool sendData(const String &key, const String &value);

    /**
     * @brief Send telemetry data to the server
     * @param key Data key
     * @param value Data value
     * @return True if data is sent successfully, false otherwise
     */
    bool sendData(const String &key, bool value);

    /**
     * @brief Send multiple telemetry points data to the server
     * @param json JSON object containing data points
     * @return True if data is sent successfully, false otherwise
     */
    bool sendData(const JsonObject &json);

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
     * @brief Process incoming messages and maintain connection
     * @note This function must be called in the loop() function
     */
    void loop();

private:
    String _deviceId;
    String _secretKey;
    String _serverUrl;
    bool _isConnected;
    bool _isAuthenticated;

    ParanodeWifi _wifi;
    ParanodeSocket _socket;
    ParanodeConnection _connection;

    CommandCallback _commandCallback;
    ConnectionCallback _connectCallback;
    ConnectionCallback _disconnectCallback;

    unsigned long _lastHeartbeatTime;
    unsigned long _heartbeatInterval;

    void handleMessage(const String &message);
    void sendHeartbeat();
    bool authenticate();
};

#endif