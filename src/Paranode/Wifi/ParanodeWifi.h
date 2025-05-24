/**
 * @file ParanodeWifi.h
 * @brief WiFi connection manager for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#ifndef PARANODE_WIFI_H
#define PARANODE_WIFI_H

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error "This library only supports ESP8266 and ESP32 boards"
#endif

/**
 * @class ParanodeWifi;
 * @brief WiFi connection manager for the Paranode library
 */
class ParanodeWifi
{
public:
    /**
     * @brief Constructor
     */
    ParanodeWifi();

    /**
     * @brief Connect to WiFi
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @param timeout Connection timeout in milliseconds
     * @return True if connection is successful, false otherwise
     */
    bool connect(const char *ssid, const char *password, unsigned long timeout = 30000);

    /**
     * @brief Disconnect from WiFi
     */
    void disconnect();

    /**
     * @brief Check if connected to WiFi
     * @return True if connected, false otherwise
     */
    bool isConnected();

    /**
     * @brief Get the current IP address
     * @return Current IP address as a string
     */
    String getIPAddress();

private:
    bool _isConnected;
};

#endif