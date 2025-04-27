# XIAO ESP32S3 Relay Control with LoRaWAN

![Seeed Studio XIAO ESP32S3](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/img/ESP32S3-Wiki.jpg)

This project connects a Seeed Studio XIAO ESP32S3 with the Relay Add-on Module to The Things Network (TTN) via LoRaWAN, enabling remote control of the relay through TTN downlinks.

## Features

- 📡 LoRaWAN connectivity with The Things Network
- ⏱️ Timed relay activation with auto-off functionality
- 🔄 Bidirectional communication (status updates and commands)
- 🛡️ Secure OTAA authentication
- 📊 Status reporting including relay state and timer information

## Hardware Requirements

- [Seeed Studio XIAO ESP32S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html)
- [Seeed Studio Grove Shield for XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)
- [Relay Add-on Module for XIAO](https://wiki.seeedstudio.com/relay_add_on_module_for_xiao/)
- [Seeed Studio Grove - Wio-E5 (LoRaWAN Module)](https://www.seeedstudio.com/LoRa-E5-Wireless-Module-p-4745.html)

## Pin Connections

| Component | Pin on XIAO ESP32S3 |
|-----------|---------------------|
| Relay     | D0                  |
| SX1262 NSS/CS | 41             |
| SX1262 DIO1   | 39             |
| SX1262 RESET  | 42             |
| SX1262 BUSY   | 40             |
| SX1262 RF SWITCH | 38          |

## Setting Up The Things Network

1. **Create a TTN Account**
   - Go to [The Things Network Console](https://console.cloud.thethings.network/)
   - Create an account or log in

2. **Create a New Application**
   - Click on "Applications" > "Create application"
   - Fill in the required fields (Application ID, name, description)
   - Click "Create application"

3. **Register Your Device**
   - In your application, go to "End devices" > "Add end device"
   - Select "Manually" registration method
   - Fill in the following details:
     - Frequency plan: Choose your region (e.g., Europe 863-870 MHz, US 902-928 MHz)
     - LoRaWAN version: MAC V1.0.2
     - Regional Parameters: PHY V1.0.2 REV B
     - Device EUI: Generate or enter your device EUI (16 hex digits)
     - Application EUI/Join EUI: Generate or enter (16 hex digits)
     - App Key: Generate or enter (32 hex digits)
   - Click "Register end device"

4. **Configure Your Device**
   - Open the `config.h` file
   - Update the LoRaWAN credentials with those from TTN:

```cpp
#ifndef RADIOLIB_LORAWAN_JOIN_EUI
#define RADIOLIB_LORAWAN_JOIN_EUI  0x0000000000000000  // Replace with your Join EUI
#endif

#ifndef RADIOLIB_LORAWAN_DEV_EUI
#define RADIOLIB_LORAWAN_DEV_EUI   0x0000000000000000  // Replace with your Device EUI
#endif

#ifndef RADIOLIB_LORAWAN_APP_KEY
#define RADIOLIB_LORAWAN_APP_KEY   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // Replace with your App Key
#endif

#ifndef RADIOLIB_LORAWAN_NWK_KEY
#define RADIOLIB_LORAWAN_NWK_KEY   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // Same as App Key for OTAA
#endif
```

5. **Choose Your Region**
   - In the main `.ino` file, set your LoRaWAN region:

```cpp
// regional choices: EU868, US915, AU915, AS923, IN865, KR920, CN780, CN500
const LoRaWANBand_t Region = EU868;  // Change to your region
const uint8_t subBand = 0; // For US915 and AU915
```

## Uploading the Code

1. Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Add XIAO ESP32S3 board support:
   - Go to File > Preferences
   - Add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to Additional Boards URLs
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install "ESP32 by Espressif Systems"
3. Install required libraries:
   - Go to Sketch > Include Library > Manage Libraries
   - Search for and install:
     - RadioLib
     - LoRaWAN-Arduino
4. Open the `.ino` file in the Arduino IDE
5. Select the correct board: Tools > Board > ESP32 > XIAO_ESP32S3
6. Select the correct port: Tools > Port > (select the port)
7. Click the Upload button

## Controlling the Relay

### LoRaWAN Commands

The device responds to downlink messages on port 2 with the following commands:

| Command Format | Description |
|----------------|-------------|
| `0x01`         | Turn relay ON |
| `0x00`         | Turn relay OFF |
| `0x02`         | Toggle relay state |
| `0x03 0xXX`    | Turn relay ON for XX seconds (0-255 seconds) |
| `0x04 0xXX 0xYY` | Turn relay ON for XXYY seconds (0-65535 seconds) |

Examples:
- `01` - Turn ON
- `00` - Turn OFF
- `03 1E` - Turn ON for 30 seconds
- `04 01 2C` - Turn ON for 300 seconds (5 minutes)

### Uplink Format

The device sends uplink messages with the following format:

| Byte | Description |
|------|-------------|
| 0    | Relay state (0 = OFF, 1 = ON) |
| 1    | Timer status (0 = inactive, 1 = active) |
| 2-3  | Remaining timer time in seconds (if timer active) |

## Payload Formatter for TTN

Add this payload formatter in your TTN application for easy decoding of uplink messages:

```javascript
function decodeUplink(input) {
  var data = {};
  var bytes = input.bytes;
  
  data.relay_state = bytes[0] === 1 ? "ON" : "OFF";
  data.timer_active = bytes[1] === 1 ? true : false;
  
  if (data.timer_active && bytes.length >= 4) {
    data.remaining_seconds = (bytes[2] << 8) + bytes[3];
  }
  
  return {
    data: data,
    warnings: [],
    errors: []
  };
}
```

## Troubleshooting

- **Device not joining TTN**
  - Verify your credentials in `config.h`
  - Check that you're using the correct region settings
  - Ensure proper antenna connection

- **Relay not responding**
  - Check that the relay module is properly connected to pin D0
  - Verify the commands are being sent in the correct format

## License

This project is released under the MIT License.

## Credits

- Based on the Seeed Studio XIAO ESP32S3 LoRaWAN example
- Modified to add relay control functionality

---

Created by Daniel Medina (@jdanielcmedina) - April 2025
```
