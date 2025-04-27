# XIAO ESP32S3 LoRaWAN Project Repository

![Seeed Studio XIAO ESP32S3](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/img/ESP32S3-Wiki.jpg)

This repository contains various projects for the Seeed Studio XIAO ESP32S3 with LoRaWAN connectivity using the RadioLib 6.6.0 library. Each subfolder contains a specific project for different sensors and applications that can be implemented with this versatile development board.

## Hardware Overview

The Seeed Studio XIAO ESP32S3 is a compact yet powerful development board based on the ESP32-S3 microcontroller. The key features include:

- Dual-core Xtensa LX7 processor up to 240MHz
- 512KB RAM and 8MB Flash
- Wi-Fi and Bluetooth 5 (LE) connectivity
- Rich set of interfaces (I2C, SPI, UART)
- Ultra-small form factor (21×17.5mm)
- USB Type-C for programming and power
- Low power consumption

This repository focuses on combining the XIAO ESP32S3 with LoRaWAN connectivity through the SX1262 module for IoT applications.

## Initial Setup Requirements

### Hardware Requirements

- [Seeed Studio XIAO ESP32S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html)
- [Seeed Studio Grove Shield for XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html) (optional but recommended)
- [LoRa-E5 module with SX1262](https://www.seeedstudio.com/LoRa-E5-Wireless-Module-p-4745.html) or compatible
- USB-C cable for programming and power
- Various sensors (specific to each project in subfolders)

### SX1262 Pin Connections

Default pin configuration for connecting SX1262 to XIAO ESP32S3:

| SX1262 Pin | XIAO ESP32S3 Pin |
|------------|------------------|
| NSS/CS     | 41               |
| DIO1       | 39               |
| RESET      | 42               |
| BUSY       | 40               |
| RF SWITCH  | 38               |
| GND        | GND              |
| 3.3V       | 3.3V             |

**Note**: Always verify these connections with your specific hardware as pin assignments may vary between different SX1262 modules.

## Software Setup

### Install Arduino IDE

1. Download and install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Launch Arduino IDE

### Add ESP32S3 Board Support

1. Go to File > Preferences
2. Add the following URL to the "Additional Boards Manager URLs" field:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to Tools > Board > Boards Manager
4. Search for "esp32" and install "ESP32 by Espressif Systems" (version 2.0.5 or later recommended)

### Install RadioLib 6.6.0

RadioLib version 6.6.0 is specifically required for these projects. To install:

1. Go to Sketch > Include Library > Manage Libraries
2. Search for "RadioLib"
3. Find RadioLib by Jan Gromes
4. Click the version dropdown and select exactly version 6.6.0
5. Click Install

**Important**: If you have another version of RadioLib already installed, you may need to uninstall it first or manually install version 6.6.0 from the [GitHub releases](https://github.com/jgromes/RadioLib/releases/tag/6.6.0).

### Manual Installation of RadioLib 6.6.0

If the library manager doesn't offer version 6.6.0:

1. Download the RadioLib 6.6.0 release from [GitHub](https://github.com/jgromes/RadioLib/releases/tag/6.6.0)
2. Extract the ZIP file
3. Rename the folder to "RadioLib"
4. Move the folder to your Arduino libraries directory:
   - Windows: `Documents\Arduino\libraries\`
   - Mac: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
5. Restart Arduino IDE

### Board Configuration

When ready to upload:

1. Select the correct board: Tools > Board > ESP32 > XIAO_ESP32S3
2. Select the USB CDC On Boot option: Tools > USB CDC On Boot > Enabled
3. Select the USB Mode: Tools > USB Mode > Hardware CDC and JTAG
4. Select the correct port: Tools > Port > (select the port your XIAO is connected to)
5. Select the Upload Speed: Tools > Upload Speed > 921600

## The Things Network Configuration

Each project will have specific TTN configuration requirements, but the general setup process is:

1. Create an account on [The Things Network Console](https://console.cloud.thethings.network/)
2. Create a new application
3. Register a new device using OTAA activation
4. Copy the device credentials (DevEUI, AppEUI/JoinEUI, and AppKey) to your code
5. Configure payload formatters as specified in each project

## Repository Structure

This repository is organized as follows:

- `/` - Main directory with this README and general configuration files
- `/relay/` - Project for controlling relay modules with LoRaWAN
- `/temperature/` - Temperature and humidity sensor projects
- `/soil/` - Soil moisture monitoring projects
- `/gps/` - GPS tracking projects
- `/docs/` - Additional documentation and diagrams

For specific sensor projects, navigate to the corresponding subfolder for detailed instructions.

## Power Management

The XIAO ESP32S3 has several power modes. For LoRaWAN applications, consider these tips:

- Use Deep Sleep between transmissions for battery-powered applications
- The ESP32S3 can be set to light sleep or deep sleep to save power
- Add a good quality capacitor (100μF) to handle power spikes during transmission
- For solar or battery applications, consider using the board's built-in power management features

## Troubleshooting

### Common Issues

- **Cannot upload to board**: 
  - Press the BOOT button while starting the upload
  - If using macOS, ensure you have the CP210x driver installed

- **LoRaWAN join issues**: 
  - Verify your credentials in `config.h`
  - Check antenna connection
  - Confirm region settings match your location

- **RadioLib compilation errors**:
  - Verify you're using exactly RadioLib 6.6.0
  - Check pin definitions match your wiring

- **Board not recognized**:
  - Try a different USB cable (data+power, not just power)
  - On some computers, use a powered USB hub

### Getting Help

If you encounter issues not covered here:
- Check the [Seeed Studio Wiki](https://wiki.seeedstudio.com/XIAO_ESP32S3/)
- Post in the [Seeed Forum](https://forum.seeedstudio.com/)
- Open an issue in this repository

## License

This project is released under the MIT License.

## Credits

- Projects based on Seeed Studio examples and community contributions
- RadioLib library by Jan Gromes

---

*Last updated: April 2025*
