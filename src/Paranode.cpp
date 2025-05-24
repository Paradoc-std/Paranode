/**
 * @file Paranode.cpp
 * @brief Implementation of the main Paranode class
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "Paranode.h"

#define PARANODE_HEARTBEAT_INTERVAL 30000

Paranode::Paranode(const String &deviceId, const String &secretKey, const String &serverUrl) : _deviceId(deviceId),
                                                                                               _secretKey(secretKey),
                                                                                               _serverUrl(serverUrl),
                                                                                               _isConnected(false),
                                                                                               _isAuthenticated(false),
                                                                                               _wifi(),
                                                                                               _socket(),
                                                                                               _connection(_socket, _deviceId, _secretKey),
                                                                                               _commandCallback(nullptr),
                                                                                               _connectCallback(nullptr),
                                                                                               _disconnectCallback(nullptr),
                                                                                               _lastHeartbeatTime(0),
                                                                                               _heartbeatInterval(PARANODE_HEARTBEAT_INTERVAL)
{
}

bool Paranode::begin()
{
    _socket.onMessage([this](const String &message)
                      { this->handleMessage(message); });

    _socket.onConnect([this]()
                      { 
                        this->_isConnected = true;
                        if (this->_connectCallback) {
                            this->_connectCallback();
                        }
                        this->authenticate(); });

    _socket.onDisconnect([this]()
                         { 
                            this->_isConnected = false;
                            this->_isAuthenticated = false;
                            if (this->_disconnectCallback) {
                                this->_disconnectCallback();
                            } });

    return true;
}

bool Paranode::connectWifi(const char *ssid, const char *password, unsigned long timeout)
{
    return _wifi.connect(ssid, password, timeout);
}

bool Paranode::connect()
{
    if (!_wifi.isConnected())
    {
        return false;
    }

    return _socket.connect(_serverUrl);
}

bool Paranode::isConnected()
{
    return _isConnected && _isAuthenticated;
}

bool Paranode::sendData(const String &key, float value)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const String &key, int value)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const String &key, const String &value)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const String &key, bool value)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const JsonObject &json)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<512> doc;
    doc["type"] = "telemetry";
    JsonObject data = doc.createNestedObject("data");

    for (JsonPair kv : json)
    {
        data[kv.key()] = kv.value();
    }

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

void Paranode::onCommand(CommandCallback callback)
{
    _commandCallback = callback;
}

void Paranode::onConnect(ConnectionCallback callback)
{
    _connectCallback = callback;
}

void Paranode::onDisconnect(ConnectionCallback callback)
{
    _disconnectCallback = callback;
}

void Paranode::loop()
{
    _socket.loop();

    unsigned long currentTime = millis();
    if (_isConnected && _isAuthenticated && (currentTime - _lastHeartbeatTime > _heartbeatInterval))
    {
        sendHeartbeat();
        _lastHeartbeatTime = currentTime;
    }

    if (!_isConnected && _wifi.isConnected())
    {
        static unsigned long lastReconnectAttempt = 0;
        if (currentTime - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = currentTime;
            connect();
        }
    }
}

void Paranode::handleMessage(const String &message)
{
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        return;
    }

    String type = doc["type"];

    if (type == "auth_response")
    {
        _isAuthenticated = doc["success"];
    }
    else if (type == "command" && _commandCallback)
    {
        JsonObject command = doc["command"].as<JsonObject>();
        _commandCallback(command);
    }
}

void Paranode::sendHeartbeat()
{
    StaticJsonDocument<64> doc;
    doc["type"] = "heartbeat";

    String message;
    serializeJson(doc, message);

    _socket.send(message);
}

bool Paranode::authenticate()
{
    StaticJsonDocument<256> doc;
    doc["type"] = "auth";
    doc["deviceId"] = _deviceId;
    doc["secretKey"] = _secretKey;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}