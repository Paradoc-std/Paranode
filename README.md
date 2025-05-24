# Paranode Arduino Library

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![ESP8266](https://img.shields.io/badge/ESP8266-Compatible-green.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight, modular IoT platform for ESP32/ESP8266 microcontrollers inspired by Blynk. Paranode enables real-time monitoring and control of devices through WebSocket communication with a cloud backend, providing seamless device connectivity, telemetry data capture, command control, and real-time communication for efficient device-to-cloud interactions.

## 🚀 Features

- **Simple API**: Easy-to-use interface for connecting devices to WiFi and cloud services
- **Device Authentication**: Secure device authentication using secret keys
- **Real-time Communication**: WebSocket-based bi-directional communication
- **Telemetry Data Upload**: Send sensor data to the cloud in real-time
- **Command Handling**: Receive and process commands from the cloud
- **Multiple Data Types**: Support for int, float, string, and boolean data types
- **Automatic Reconnection**: Built-in connection management and auto-reconnect
- **Heartbeat System**: Keep-alive mechanism for reliable connections
- **Multi-platform Support**: Compatible with both ESP32 and ESP8266

## 📋 Requirements

### Hardware

- ESP32 or ESP8266 development board
- Stable WiFi connection

### Software

- Arduino IDE 1.8.0 or later
- ESP32/ESP8266 board package installed in Arduino IDE

## 📦 Installation

### Method 1: Arduino Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries...**
3. Search for "Paranode"
4. Click **Install**

### Method 2: Manual Installation

1. Download the latest release from [GitHub](https://github.com/yourusername/paranode-arduino)
2. Extract the ZIP file
3. Rename the folder to "Paranode"
4. Move the folder to your Arduino libraries directory:
   - **Windows**: `Documents\Arduino\libraries\`
   - **macOS**: `~/Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`
5. Restart Arduino IDE

### Method 3: Git Clone

```bash
cd ~/Documents/Arduino/libraries/
git clone https://github.com/yourusername/paranode-arduino.git Paranode
```

## 🔧 Dependencies

The following libraries are required and will be installed automatically:

- [ArduinoJson](https://arduinojson.org/) (v6.21.3 or later)
- [WebSockets](https://github.com/Links2004/arduinoWebSockets) (v2.4.1 or later)

## 🎯 Quick Start

### Basic Example

```cpp
#include <Paranode.h>

// WiFi credentials
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Paranode credentials
const char* DEVICE_ID = "your-device-id";
const char* SECRET_KEY = "your-secret-key";

// Initialize Paranode
Paranode paranode(DEVICE_ID, SECRET_KEY);

void setup() {
  Serial.begin(115200);
  
  // Initialize Paranode
  paranode.begin();
  
  // Connect to WiFi
  if (paranode.connectWifi(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("WiFi connected!");
  }
  
  // Set up event callbacks
  paranode.onConnect([]() {
    Serial.println("Connected to Paranode server");
  });
  
  paranode.onCommand([](const JsonObject& command) {
    String action = command["action"];
    Serial.println("Received command: " + action);
  });
  
  // Connect to Paranode server
  paranode.connect();
}

void loop() {
  paranode.loop();
  
  // Send sensor data every 5 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    lastSend = millis();
    
    if (paranode.isConnected()) {
      float temperature = 25.5; // Your sensor reading
      paranode.sendData("temperature", temperature);
    }
  }
}
```

## 📚 API Reference

### Constructor

```cpp
Paranode(const String& deviceId, const String& secretKey, const String& serverUrl = "wss://api.paranode.io/ws")
```

**Parameters:**

- `deviceId`: Unique identifier for your device
- `secretKey`: Secret key for authentication
- `serverUrl`: (Optional) WebSocket server URL

### Core Methods

#### `bool begin()`

Initialize the Paranode library. Call this in `setup()`.

#### `bool connectWifi(const char* ssid, const char* password, unsigned long timeout = 30000)`

Connect to WiFi network.

**Parameters:**

- `ssid`: WiFi network name
- `password`: WiFi password
- `timeout`: Connection timeout in milliseconds

**Returns:** `true` if connected successfully

#### `bool connect()`

Connect to the Paranode server.

**Returns:** `true` if connection initiated successfully

#### `bool isConnected()`

Check if connected to the Paranode server.

**Returns:** `true` if connected and authenticated

#### `void loop()`

Process Paranode events. **Must be called in Arduino `loop()` function.**

### Data Transmission

#### `bool sendData(const String& key, value)`

Send telemetry data to the server. Supports multiple data types:

```cpp
paranode.sendData("temperature", 25.5f);    // float
paranode.sendData("humidity", 60);          // int
paranode.sendData("status", "online");      // String
paranode.sendData("led_state", true);       // bool
```

#### `bool sendData(const JsonObject& json)`

Send multiple data points at once:

```cpp
StaticJsonDocument<256> doc;
JsonObject data = doc.to<JsonObject>();
data["temperature"] = 25.5;
data["humidity"] = 60;
data["pressure"] = 1013.25;

paranode.sendData(data);
```

### Event Callbacks

#### `void onConnect(ConnectionCallback callback)`

Set callback for successful connection.

```cpp
paranode.onConnect([]() {
  Serial.println("Connected to server!");
});
```

#### `void onDisconnect(ConnectionCallback callback)`

Set callback for disconnection.

```cpp
paranode.onDisconnect([]() {
  Serial.println("Disconnected from server!");
});
```

#### `void onCommand(CommandCallback callback)`

Set callback for received commands.

```cpp
paranode.onCommand([](const JsonObject& command) {
  String action = command["action"];
  
  if (action == "led" && command.containsKey("value")) {
    bool ledState = command["value"];
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
  }
});
```

## 💡 Examples

The library includes several comprehensive examples:

### 1. BasicSensor

Simple temperature sensor data transmission.

### 2. MultipleDataPoints

Send multiple sensor readings (temperature, humidity, heat index) simultaneously.

### 3. CommandListener

Handle commands from the cloud to control LEDs and relays.

### 4. AdvancedIoT

Complete IoT solution with multiple sensors, actuators, and robust error handling.

Access examples through: **File** → **Examples** → **Paranode**

## 🔧 Configuration

### Custom Server URL

```cpp
Paranode paranode("device-id", "secret-key", "wss://your-server.com/ws");
```

### Connection Timeouts

```cpp
// WiFi connection timeout (default: 30 seconds)
paranode.connectWifi(ssid, password, 10000); // 10 seconds timeout
```

## 🚨 Troubleshooting

### Common Issues

**WiFi Connection Failed**

- Check SSID and password
- Ensure strong WiFi signal
- Verify 2.4GHz network (ESP8266 doesn't support 5GHz)

**Server Connection Failed**

- Verify device ID and secret key
- Check internet connectivity
- Ensure server URL is correct
- Check firewall settings

**Data Not Sending**

- Verify `paranode.loop()` is called in main loop
- Check if `paranode.isConnected()` returns true
- Ensure JSON data size doesn't exceed limits

**Memory Issues**

- Reduce JSON document sizes
- Use `StaticJsonDocument` instead of `DynamicJsonDocument`
- Monitor heap usage with `ESP.getFreeHeap()`

### Debug Mode

Enable serial debugging by adding this before `paranode.begin()`:

```cpp
Serial.begin(115200);
// Paranode will automatically use Serial for debug output
```

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow Arduino library structure
- Add examples for new features
- Update documentation
- Test on both ESP32 and ESP8266
- Maintain backward compatibility

## 📄 License

This library is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by [Blynk](https://blynk.io/) IoT platform
- Built with [ArduinoJson](https://arduinojson.org/) library
- Uses [WebSockets](https://github.com/Links2004/arduinoWebSockets) for real-time communication

## 📞 Support

- **Documentation**: [GitHub Wiki](https://github.com/yourusername/paranode-arduino/wiki)
- **Issues**: [GitHub Issues](https://github.com/yourusername/paranode-arduino/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/paranode-arduino/discussions)
- **Email**: <support@paranode.io>

## 🗺️ Roadmap

- [ ] MQTT support
- [ ] Local server mode
- [ ] OTA (Over-The-Air) updates
- [ ] Advanced security features
- [ ] Dashboard web interface
- [ ] Mobile app integration
- [ ] LoRaWAN support

---

**Made with ❤️ for the Arduino and IoT community**
