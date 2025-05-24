/**
 * @file ParanodeSocket.h
 * @brief WebSocket client for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#ifndef PARANODE_SOCKET_H
#define PARANODE_SOCKET_H

#include <Arduino.h>
#include <functional>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebSocketsClient.h>
#else
#error "This library only supports ESP8266 and ESP32 boards"
#endif

typedef std::function<void(const String &)> MessageCallback;
typedef std::function<void(void)> ConnectionCallback;

/**
 * @class ParanodeSocket
 * @brief WebSocket client for the Paranode library
 */
class ParanodeSocket
{
public:
    /**
     * @brief Constructor
     */
    ParanodeSocket();

    /**
     * @brief Connect to the WebSocket server
     * @param url Server URL
     * @return True if connection is initiated successfully, false otherwise
     */
    bool connect(const String &url);

    /**
     * @brief Disconnect from the WebSocket server
     */
    void disconnect();

    /**
     * @brief Check if connected to the WebSocket server
     * @return True if connected, false otherwise
     */
    bool isConnected();

    /**
     * @brief Send a message to the WebSocket server
     * @param message Message to send
     * @return True if message is sent successfully, false otherwise
     */
    bool send(const String &message);

    /**
     * @brief Set callback for received messages
     * @param callback Function to be called when a message is received
     */
    void onMessage(MessageCallback callback);

    /**
     * @brief Set callback for successful connection
     * @param callback Function to be called when connection is established
     */
    void onConnect(ConnectionCallback callback);

    /**
     * @brief Set callback for disconnection
     * @param callback Function to be called when disconnected
     */
    void onDisconnect(ConnectionCallback callback);

    /**
     * @brief Process WebSocket events
     * @note This function must be called in the loop() function
     */
    void loop();

private:
    WebSocketsClient _socket;
    bool _isConnected;

    MessageCallback _messageCallback;
    ConnectionCallback _connectCallback;
    ConnectionCallback _disconnectCallback;

    void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
};

#endif