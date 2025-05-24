/**
 * @file ParanodeWifi.cpp
 * @brief Implementation of the WiFi connection manager for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "ParanodeWifi.h"

ParanodeWifi::ParanodeWifi() : _isConnected(false) {}

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