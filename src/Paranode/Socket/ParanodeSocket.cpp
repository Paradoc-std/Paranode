/**
 * @file ParanodeSocket.cpp
 * @brief Implementation of the WebSocket client for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "ParanodeSocket.h"

ParanodeSocket::ParanodeSocket() : _isConnected(false),
                                   _messageCallback(nullptr),
                                   _connectCallback(nullptr),
                                   _disconnectCallback(nullptr)
{
}

bool ParanodeSocket::connect(const String &url)
{
    // Optimized URL parsing - avoid multiple String operations
    const char* urlStr = url.c_str();
    int urlLen = url.length();

    // Find protocol
    const char* protocolEnd = strstr(urlStr, "://");
    if (!protocolEnd) return false;

    bool isSecure = (protocolEnd - urlStr == 3 && strncmp(urlStr, "wss", 3) == 0);
    int port = isSecure ? 443 : 80;

    // Extract host and path
    const char* hostStart = protocolEnd + 3;
    const char* pathStart = strchr(hostStart, '/');

    char host[128];
    char path[256];

    if (pathStart) {
        int hostLen = pathStart - hostStart;
        if (hostLen >= sizeof(host)) hostLen = sizeof(host) - 1;
        strncpy(host, hostStart, hostLen);
        host[hostLen] = '\0';

        int pathLen = urlLen - (pathStart - urlStr);
        if (pathLen >= sizeof(path)) pathLen = sizeof(path) - 1;
        strncpy(path, pathStart, pathLen);
        path[pathLen] = '\0';
    } else {
        int hostLen = urlLen - (hostStart - urlStr);
        if (hostLen >= sizeof(host)) hostLen = sizeof(host) - 1;
        strncpy(host, hostStart, hostLen);
        host[hostLen] = '\0';
        strcpy(path, "/");
    }

    // Check for port in host
    char* colonPos = strchr(host, ':');
    if (colonPos) {
        *colonPos = '\0';
        port = atoi(colonPos + 1);
    }

    _socket.begin(host, port, path, isSecure ? "wss" : "ws");
    _socket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                    { this->handleWebSocketEvent(type, payload, length); });
    _socket.setReconnectInterval(5000);

    return true;
}

void ParanodeSocket::disconnect()
{
    _socket.disconnect();
    _isConnected = false;
}

bool ParanodeSocket::isConnected()
{
    return _isConnected;
}

bool ParanodeSocket::send(const String &message)
{
    if (!_isConnected)
    {
        return false;
    }

    return _socket.sendTXT(message.c_str());
}

void ParanodeSocket::onMessage(MessageCallback callback)
{
    _messageCallback = callback;
}

void ParanodeSocket::onConnect(ConnectionCallback callback)
{
    _connectCallback = callback;
}

void ParanodeSocket::onDisconnect(ConnectionCallback callback)
{
    _disconnectCallback = callback;
}

void ParanodeSocket::loop()
{
    _socket.loop();
}

void ParanodeSocket::handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_CONNECTED:
        _isConnected = true;
        if (_connectCallback)
        {
            _connectCallback();
        }
        break;

    case WStype_DISCONNECTED:
        _isConnected = false;
        if (_disconnectCallback)
        {
            _disconnectCallback();
        }
        break;
    case WStype_TEXT:
        if (_messageCallback)
        {
            String message((char *)payload, length);
            _messageCallback(message);
        }
        break;
    default:
        break;
    }
}