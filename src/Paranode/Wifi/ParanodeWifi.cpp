/**
 * @file ParanodeWifi.cpp
 * @brief Implementation of the WiFi connection manager for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "ParanodeWifi.h"

ParanodeWifi::ParanodeWifi() : _isConnected(false), _isConnecting(false), _connectStartTime(0) {}

bool ParanodeWifi::connect(const char *ssid, const char *password, unsigned long timeout)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();
        delay(10);
    }

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < timeout))
    {
        delay(100);
    }

    _isConnected = (WiFi.status() == WL_CONNECTED);

    return _isConnected;
}

void ParanodeWifi::disconnect()
{
    WiFi.disconnect();
    _isConnected = false;
}

bool ParanodeWifi::isConnected()
{
    _isConnected = (WiFi.status() == WL_CONNECTED);
    return _isConnected;
}

String ParanodeWifi::getIPAddress()
{
    if (!_isConnected)
    {
        return "";
    }

    return WiFi.localIP().toString();
}

bool ParanodeWifi::connectAsync(const char *ssid, const char *password)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();
        delay(10);
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    _isConnecting = true;
    _isConnected = false;
    _connectStartTime = millis();

    return true;
}

bool ParanodeWifi::checkConnection()
{
    if (!_isConnecting)
    {
        return _isConnected;
    }

    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED)
    {
        _isConnected = true;
        _isConnecting = false;
        return true;
    }

    // Check timeout (30 seconds default)
    if (millis() - _connectStartTime > 30000)
    {
        _isConnecting = false;
        _isConnected = false;
        return false;
    }

    // Still connecting
    return false;
}

wl_status_t ParanodeWifi::getStatus()
{
    return WiFi.status();
}