/**
 * @file Paranode.cpp
 * @brief Enhanced implementation of the main Paranode class
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "Paranode.h"

#define PARANODE_HEARTBEAT_INTERVAL 30000
#define PARANODE_METRICS_INTERVAL 60000

Paranode::Paranode(const String &deviceId, const String &secretKey, const String &serverUrl)
    : _deviceId(deviceId),
      _secretKey(secretKey),
      _serverUrl(serverUrl),
      _macAddress(""),
      _firmwareVersion("1.0.0"),
      _hardwareVersion("1.0.0"),
      _isConnected(false),
      _isAuthenticated(false),
      _autoReconnect(true),
      _startTime(millis()),
      _wifi(),
      _socket(),
      _connection(_socket, _deviceId, _secretKey),
      _commandCallback(nullptr),
      _connectCallback(nullptr),
      _disconnectCallback(nullptr),
      _otaCallback(nullptr),
      _otaProgressCallback(nullptr),
      _lastHeartbeatTime(0),
      _heartbeatInterval(PARANODE_HEARTBEAT_INTERVAL),
      _lastMetricsTime(0),
      _metricsInterval(PARANODE_METRICS_INTERVAL)
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

    // Get MAC address if not set
    if (_macAddress.isEmpty())
    {
        _macAddress = getDefaultMacAddress();
    }

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

bool Paranode::sendData(const String &key, float value, const String &unit)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;
    if (!unit.isEmpty())
    {
        doc["unit"] = unit;
    }
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const String &key, int value, const String &unit)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;
    if (!unit.isEmpty())
    {
        doc["unit"] = unit;
    }
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const String &key, const String &value, const String &unit)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;
    if (!unit.isEmpty())
    {
        doc["unit"] = unit;
    }
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendData(const String &key, bool value, const String &unit)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "telemetry";
    doc["key"] = key;
    doc["value"] = value;
    if (!unit.isEmpty())
    {
        doc["unit"] = unit;
    }
    doc["timestamp"] = millis();

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
    doc["timestamp"] = millis();
    JsonObject data = doc.createNestedObject("data");

    for (JsonPair kv : json)
    {
        data[kv.key()] = kv.value();
    }

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendStatus(const String &status)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "status";
    doc["status"] = status;
    doc["timestamp"] = millis();
    doc["uptime"] = getUptime();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendError(const String &errorMessage, int errorCode)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "error";
    doc["message"] = errorMessage;
    if (errorCode != 0)
    {
        doc["code"] = errorCode;
    }
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendMetrics(uint32_t freeHeap, int rssi)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "metrics";
    JsonObject data = doc.createNestedObject("data");
    data["freeHeap"] = freeHeap;
    data["rssi"] = rssi;
    data["uptime"] = getUptime();
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

void Paranode::setDeviceInfo(const String &firmwareVersion, const String &hardwareVersion)
{
    _firmwareVersion = firmwareVersion;
    _hardwareVersion = hardwareVersion;
}

void Paranode::setMacAddress(const String &macAddress)
{
    _macAddress = macAddress;
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

void Paranode::onOTAUpdate(OTACallback callback)
{
    _otaCallback = callback;
}

void Paranode::onOTAProgress(OTAProgressCallback callback)
{
    _otaProgressCallback = callback;
}

bool Paranode::requestConfig()
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "config_request";

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

bool Paranode::sendCommandResponse(const String &commandId, const String &status, const String &response)
{
    if (!isConnected())
    {
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "command_response";
    doc["commandId"] = commandId;
    doc["status"] = status;
    if (!response.isEmpty())
    {
        doc["response"] = response;
    }
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

void Paranode::setAutoReconnect(bool enable)
{
    _autoReconnect = enable;
}

void Paranode::setHeartbeatInterval(unsigned long interval)
{
    if (interval >= 10000)
    {
        _heartbeatInterval = interval;
    }
}

unsigned long Paranode::getUptime()
{
    return (millis() - _startTime) / 1000;
}

void Paranode::loop()
{
    _socket.loop();

    unsigned long currentTime = millis();

    // Send heartbeat
    if (_isConnected && _isAuthenticated && (currentTime - _lastHeartbeatTime > _heartbeatInterval))
    {
        sendHeartbeat();
        _lastHeartbeatTime = currentTime;
    }

    // Send automatic metrics
    if (_isConnected && _isAuthenticated && (currentTime - _lastMetricsTime > _metricsInterval))
    {
#ifdef ESP8266
        uint32_t freeHeap = ESP.getFreeHeap();
#elif defined(ESP32)
        uint32_t freeHeap = ESP.getFreeHeap();
#endif

        int rssi = WiFi.RSSI();
        sendMetrics(freeHeap, rssi);
        _lastMetricsTime = currentTime;
    }

    // Auto-reconnect
    if (_autoReconnect && !_isConnected && _wifi.isConnected())
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
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        return;
    }

    String type = doc["type"];

    if (type == "auth_response")
    {
        _isAuthenticated = doc["success"];
        if (_isAuthenticated)
        {
            sendDeviceInfo();
        }
    }
    else if (type == "command" && _commandCallback)
    {
        JsonObject command = doc["command"].as<JsonObject>();
        _commandCallback(command);
    }
    else if (type == "ota_update" && _otaCallback)
    {
        handleOTAUpdate(doc["update"].as<JsonObject>());
    }
    else if (type == "config")
    {
        handleConfig(doc["config"].as<JsonObject>());
    }
    else if (type == "ota_progress" && _otaProgressCallback)
    {
        int progress = doc["progress"];
        _otaProgressCallback(progress);
    }
}

void Paranode::sendHeartbeat()
{
    StaticJsonDocument<256> doc;
    doc["type"] = "heartbeat";
    doc["uptime"] = getUptime();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["rssi"] = WiFi.RSSI();

    String message;
    serializeJson(doc, message);

    _socket.send(message);
}

bool Paranode::authenticate()
{
    StaticJsonDocument<512> doc;
    doc["type"] = "auth";
    doc["deviceId"] = _deviceId;
    doc["secretKey"] = _secretKey;
    doc["macAddress"] = _macAddress;
    doc["ipAddress"] = _wifi.getIPAddress();
    doc["firmwareVersion"] = _firmwareVersion;
    doc["hardwareVersion"] = _hardwareVersion;

    String message;
    serializeJson(doc, message);

    return _socket.send(message);
}

void Paranode::sendDeviceInfo()
{
    StaticJsonDocument<256> doc;
    doc["type"] = "device_info";
    doc["firmwareVersion"] = _firmwareVersion;
    doc["hardwareVersion"] = _hardwareVersion;
    doc["macAddress"] = _macAddress;
    doc["ipAddress"] = _wifi.getIPAddress();

    String message;
    serializeJson(doc, message);

    _socket.send(message);
}

void Paranode::handleOTAUpdate(const JsonObject &update)
{
    if (_otaCallback)
    {
        String url = update["url"];
        _otaCallback(url);
    }
}

void Paranode::handleConfig(const JsonObject &config)
{
    if (config.containsKey("heartbeatInterval"))
    {
        unsigned long interval = config["heartbeatInterval"];
        setHeartbeatInterval(interval);
    }

    if (config.containsKey("metricsInterval"))
    {
        _metricsInterval = config["metricsInterval"];
    }
}

String Paranode::getDefaultMacAddress()
{
#ifdef ESP8266
    return WiFi.macAddress();
#elif defined(ESP32)
    return WiFi.macAddress();
#endif
}