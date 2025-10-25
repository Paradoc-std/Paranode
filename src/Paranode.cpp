/**
 * @file Paranode.cpp
 * @brief Enhanced implementation of the main Paranode class
 * @author Muhammad Daffa
 * @date 2025-05-21
 */

#include "Paranode.h"

#define PARANODE_HEARTBEAT_INTERVAL 30000
#define PARANODE_METRICS_INTERVAL 60000

// Legacy constructor (backward compatibility)
Paranode::Paranode(const String &deviceId, const String &secretKey, const String &serverUrl)
    : _deviceId(deviceId),
      _secretKey(secretKey),
      _projectToken(""),
      _serverUrl(serverUrl),
      _macAddress(""),
      _firmwareVersion("1.0.0"),
      _hardwareVersion("1.0.0"),
      _isConnected(false),
      _isAuthenticated(false),
      _autoReconnect(true),
      _useTokenAuth(false),
      _startTime(millis()),
      _wifi(),
      _socket(),
      _connection(_socket, _deviceId, _secretKey),
      _messageQueue(),
      _commandCallback(nullptr),
      _connectCallback(nullptr),
      _disconnectCallback(nullptr),
      _otaCallback(nullptr),
      _otaProgressCallback(nullptr),
      _wifiConfigCallback(nullptr),
      _lastHeartbeatTime(0),
      _heartbeatInterval(PARANODE_HEARTBEAT_INTERVAL),
      _lastMetricsTime(0),
      _metricsInterval(PARANODE_METRICS_INTERVAL),
      _batchingEnabled(false),
      _batchSize(5),
      _lastBatchTime(0),
      _batchInterval(10000)
{
    // Initialize buffers
    _messageBuffer[0] = '\0';
    _batchBuffer[0] = '\0';
}

// New token-based constructor (recommended)
Paranode::Paranode(const String &projectToken, bool useTokenAuth)
    : _deviceId(""),
      _secretKey(""),
      _projectToken(projectToken),
      _serverUrl("wss://api.paranode.io/ws"),
      _macAddress(""),
      _firmwareVersion("1.0.0"),
      _hardwareVersion("1.0.0"),
      _isConnected(false),
      _isAuthenticated(false),
      _autoReconnect(true),
      _useTokenAuth(true),
      _startTime(millis()),
      _wifi(),
      _socket(),
      _connection(_socket, "", ""),
      _messageQueue(),
      _commandCallback(nullptr),
      _connectCallback(nullptr),
      _disconnectCallback(nullptr),
      _otaCallback(nullptr),
      _otaProgressCallback(nullptr),
      _wifiConfigCallback(nullptr),
      _lastHeartbeatTime(0),
      _heartbeatInterval(PARANODE_HEARTBEAT_INTERVAL),
      _lastMetricsTime(0),
      _metricsInterval(PARANODE_METRICS_INTERVAL),
      _batchingEnabled(false),
      _batchSize(5),
      _lastBatchTime(0),
      _batchInterval(10000)
{
    // Initialize buffers
    _messageBuffer[0] = '\0';
    _batchBuffer[0] = '\0';

    // Device ID will be auto-generated from MAC address
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

// Backward compatibility wrappers using optimized template
bool Paranode::sendData(const String &key, float value, const String &unit)
{
    return sendData<float>(key.c_str(), value, unit.c_str(), _batchingEnabled);
}

bool Paranode::sendData(const String &key, int value, const String &unit)
{
    return sendData<int>(key.c_str(), value, unit.c_str(), _batchingEnabled);
}

bool Paranode::sendData(const String &key, const String &value, const String &unit)
{
    return sendData<const char*>(key.c_str(), value.c_str(), unit.c_str(), _batchingEnabled);
}

bool Paranode::sendData(const String &key, bool value, const String &unit)
{
    return sendData<bool>(key.c_str(), value, unit.c_str(), _batchingEnabled);
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

    // Use optimized JSON builder
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "status");
    builder.addString("status", status.c_str());
    builder.addULong("timestamp", millis());
    builder.addULong("uptime", getUptime());
    builder.endObject();

    return sendMessageDirect(builder.getJson());
}

bool Paranode::sendError(const String &errorMessage, int errorCode)
{
    if (!isConnected())
    {
        return false;
    }

    // Use optimized JSON builder
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "error");
    builder.addString("message", errorMessage.c_str());
    if (errorCode != 0)
    {
        builder.addInt("code", errorCode);
    }
    builder.addULong("timestamp", millis());
    builder.endObject();

    // Errors are high priority
    return sendMessageQueued(builder.getJson(), 2);
}

bool Paranode::sendMetrics(uint32_t freeHeap, int rssi)
{
    if (!isConnected())
    {
        return false;
    }

    // Use optimized JSON builder
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "metrics");
    builder.startNestedObject("data");
    builder.addULong("freeHeap", freeHeap);
    builder.addInt("rssi", rssi);
    builder.addULong("uptime", getUptime());
    builder.endObject(); // end data object
    builder.addULong("timestamp", millis());
    builder.endObject();

    return sendMessageQueued(builder.getJson(), 0); // Low priority
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

    // Process message queue
    if (_isConnected && _isAuthenticated)
    {
        processQueue();

        // Auto-batch send
        if (_batchingEnabled && !_messageQueue.isEmpty() &&
            (currentTime - _lastBatchTime > _batchInterval))
        {
            flushQueue();
            _lastBatchTime = currentTime;
        }
    }

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

    // Remove expired messages from queue (older than 5 minutes)
    if (!_messageQueue.isEmpty() && (currentTime % 30000) < 100)
    {
        _messageQueue.removeExpired(300000);
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

    if (type == "auth_response" || type == "auth_token_response")
    {
        _isAuthenticated = doc["success"];
        if (_isAuthenticated)
        {
            sendDeviceInfo();

            // If using token auth, store assigned device ID
            if (_useTokenAuth && doc.containsKey("deviceId")) {
                _deviceId = doc["deviceId"].as<String>();
            }

            // Check for project info in response
            if (doc.containsKey("project")) {
                JsonObject project = doc["project"];
                // You can store project limits, name, etc.
                // For example: _projectName = project["name"];
            }
        }
        else
        {
            // Authentication failed
            if (doc.containsKey("error")) {
                String errorMsg = doc["error"];
                // Could trigger an error callback
                Serial.print("Auth failed: ");
                Serial.println(errorMsg);
            }
        }
    }
    else if (type == "command" && _commandCallback)
    {
        JsonObject command = doc["command"].as<JsonObject>();
        _commandCallback(command);
    }
    else if (type == "wifi_config" && _wifiConfigCallback)
    {
        // Handle WiFi configuration from web app
        String ssid = doc["ssid"];
        String password = doc["password"];
        _wifiConfigCallback(ssid, password);
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
    else if (type == "project_info")
    {
        // Handle project information response
        if (doc.containsKey("project")) {
            JsonObject project = doc["project"];
            // Could expose this via callback if needed
        }
    }
}

void Paranode::sendHeartbeat()
{
    // Use optimized JSON builder
    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "heartbeat");
    builder.addULong("uptime", getUptime());
    builder.addULong("freeHeap", ESP.getFreeHeap());
    builder.addInt("rssi", WiFi.RSSI());
    builder.endObject();

    sendMessageDirect(builder.getJson());
}

bool Paranode::authenticate()
{
    if (_useTokenAuth) {
        // Token-based authentication (new method)
        ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
        builder.startObject();
        builder.addString("type", "auth_token");
        builder.addString("projectToken", _projectToken.c_str());
        builder.addString("deviceId", _deviceId.isEmpty() ? _macAddress.c_str() : _deviceId.c_str());
        builder.addString("macAddress", _macAddress.c_str());
        builder.addString("ipAddress", _wifi.getIPAddress().c_str());
        builder.addString("firmwareVersion", _firmwareVersion.c_str());
        builder.addString("hardwareVersion", _hardwareVersion.c_str());
        builder.addString("platform",
#ifdef ESP32
            "ESP32"
#else
            "ESP8266"
#endif
        );
        builder.endObject();

        return _socket.send(builder.getJson());
    } else {
        // Legacy device ID + secret key authentication
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

// New optimized methods
bool Paranode::sendMessageDirect(const char* message)
{
    if (!message || !_isConnected) {
        return false;
    }
    return _socket.send(message);
}

bool Paranode::sendMessageQueued(const char* message, uint8_t priority)
{
    if (!message) {
        return false;
    }

    // If connected and not batching, send immediately
    if (_isConnected && !_batchingEnabled) {
        return sendMessageDirect(message);
    }

    // Otherwise queue the message
    return _messageQueue.enqueue(message, strlen(message), priority);
}

void Paranode::processQueue()
{
    if (_messageQueue.isEmpty() || !_isConnected) {
        return;
    }

    // Send a few queued messages per loop iteration
    int sent = 0;
    int maxSend = 3; // Don't flood the connection

    while (!_messageQueue.isEmpty() && sent < maxSend)
    {
        char buffer[PARANODE_MAX_MESSAGE_SIZE];
        uint16_t len = _messageQueue.dequeue(buffer, sizeof(buffer));

        if (len > 0) {
            if (_socket.send(buffer)) {
                sent++;
            } else {
                // Re-queue if send failed
                _messageQueue.enqueue(buffer, len, 1);
                break;
            }
        }
    }
}

int Paranode::flushQueue()
{
    if (!_isConnected || _messageQueue.isEmpty()) {
        return 0;
    }

    if (_batchingEnabled) {
        // Batch multiple messages together
        int batched = _messageQueue.batchMessages(_batchBuffer, sizeof(_batchBuffer), _batchSize);
        if (batched > 0) {
            // Send batched message
            if (_socket.send(_batchBuffer)) {
                // Remove batched messages from queue
                for (int i = 0; i < batched; i++) {
                    char dummyBuffer[32];
                    _messageQueue.dequeue(dummyBuffer, sizeof(dummyBuffer));
                }
                return batched;
            }
        }
        return 0;
    } else {
        // Send all queued messages individually
        int sent = 0;
        while (!_messageQueue.isEmpty()) {
            char buffer[PARANODE_MAX_MESSAGE_SIZE];
            uint16_t len = _messageQueue.dequeue(buffer, sizeof(buffer));

            if (len > 0) {
                if (_socket.send(buffer)) {
                    sent++;
                } else {
                    // Re-queue and stop
                    _messageQueue.enqueue(buffer, len, 1);
                    break;
                }
            }

            // Yield to avoid watchdog timeout
            if (sent % 5 == 0) {
                yield();
            }
        }
        return sent;
    }
}

void Paranode::setBatching(bool enable, int batchSize)
{
    _batchingEnabled = enable;
    if (batchSize > 0 && batchSize <= 10) {
        _batchSize = batchSize;
    }
}

size_t Paranode::getQueuedCount() const
{
    return _messageQueue.count();
}

// Web Integration Features

bool Paranode::sendGeolocation(double latitude, double longitude, float accuracy)
{
    if (!isConnected()) {
        return false;
    }

    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "geolocation");
    builder.addDouble("latitude", latitude, 6);
    builder.addDouble("longitude", longitude, 6);
    if (accuracy > 0) {
        builder.addFloat("accuracy", accuracy);
    }
    builder.addULong("timestamp", millis());
    builder.endObject();

    return sendMessageDirect(builder.getJson());
}

bool Paranode::requestWiFiConfig()
{
    if (!isConnected()) {
        return false;
    }

    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "wifi_config_request");
    builder.addString("currentSSID", WiFi.SSID().c_str());
    builder.addInt("currentRSSI", WiFi.RSSI());
    builder.endObject();

    return sendMessageDirect(builder.getJson());
}

void Paranode::onWiFiConfig(std::function<void(const String &, const String &)> callback)
{
    _wifiConfigCallback = callback;
}

bool Paranode::updateDeviceStatus(const JsonObject &metadata)
{
    if (!isConnected()) {
        return false;
    }

    StaticJsonDocument<512> doc;
    doc["type"] = "device_status_update";
    doc["timestamp"] = millis();
    doc["uptime"] = getUptime();

    JsonObject meta = doc.createNestedObject("metadata");
    for (JsonPair kv : metadata) {
        meta[kv.key()] = kv.value();
    }

    String message;
    serializeJson(doc, message);

    return sendMessageDirect(message.c_str());
}

bool Paranode::requestProjectInfo()
{
    if (!isConnected()) {
        return false;
    }

    ParanodeJsonBuilder builder(_messageBuffer, sizeof(_messageBuffer));
    builder.startObject();
    builder.addString("type", "project_info_request");
    builder.endObject();

    return sendMessageDirect(builder.getJson());
}