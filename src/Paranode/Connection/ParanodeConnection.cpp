/**
 * @file ParanodeConnection.h
 * @brief Implementation of the connection manager for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "ParanodeConnection.h"
#include <ArduinoJson.h>

ParanodeConnection::ParanodeConnection(ParanodeSocket &socket, const String &deviceId, const String &secretKey) : _socket(socket),
                                                                                                                  _deviceId(deviceId),
                                                                                                                  _secretKey(secretKey),
                                                                                                                  _isAuthenticated(false)
{
}

bool ParanodeConnection::authenticate()
{
    if (!_socket.isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "auth";
    doc["deviceId"] = _deviceId;
    doc["secretKey"] = _secretKey;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool ParanodeConnection::isAuthenticated()
{
    return _isAuthenticated;
}

bool ParanodeConnection::send(const String &message)
{
    if (!_socket.isConnected() || !_isAuthenticated)
    {
        return false;
    }

    return _socket.send(message);
}