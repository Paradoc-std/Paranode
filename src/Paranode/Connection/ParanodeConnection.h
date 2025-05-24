/**
 * @file ParanodeConnection.h
 * @brief Connection manager for the Paranode library
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#ifndef PARANODE_CONNECTION_H
#define PARANODE_CONNECTION_H

#include <Arduino.h>
#include "Paranode/Socket/ParanodeSocket.h"

/**
 * @class ParanodeConnection
 * @brief Connection manager for the Paranode library
 */
class ParanodeConnection
{
public:
    /**
     * @brief Consturctor
     * @param socket WebSocket client
     * @param deviceId Device ID
     * @param secretKey Secret key
     */
    ParanodeConnection(ParanodeSocket &socket, const String &deviceId, const String &secretKey);

    /**
     * @brief Authenticate with the server
     * @return True if authentication is successful, false otherwise
     */
    bool authenticate();

    /**
     * @brief Check if authenticated
     * @return True if authenticated, false otherwise
     */
    bool isAuthenticated();

    /**
     * @brief Send a message to the server
     * @param message Message to send
     * @return True if message is sent successfully, false otherwise
     */
    bool send(const String &message);

private:
    ParanodeSocket &_socket;
    String _deviceId;
    String _secretKey;
    bool _isAuthenticated;
};

#endif