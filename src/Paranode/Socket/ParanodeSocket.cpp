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
    String protocol = url.substring(0, url.indexOf("://"));
    String host = url.substring(url.indexOf("://") + 3, url.indexOf("/", url.indexOf("://") + 3));
    String path = url.substring(url.indexOf("/", url.indexOf("://") + 3));

    int port = (protocol == "wss") ? 443 : 80;
    int colonPos = host.indexOf(":");
    if (colonPos > 0)
    {
        port = host.substring(colonPos + 1).toInt();
        host = host.substring(0, colonPos);
    }

    _socket.begin(host, port, path, (protocol == "wss") ? "wss" : "ws");
    _socket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                    { this->handleWebSocketEvent(type, payload, length); });
    _socket.setReconnectInterval(5000);

    return true;
};

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