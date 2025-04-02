# ESP32 Music Visualization System

A music visualization system based on ESP32 that creates dynamic lighting effects on LED strips in response to audio input. The system supports various visualization modes and can be controlled via a web interface or UDP commands.

## Main Features

### Display Modes
- Static color mode
- Rainbow wave effect
- Color flicker effect
- Strobe mode
- Single zone frequency visualization
- Three-zone frequency visualization
- Five-zone frequency visualization
- Center drop effect
- Center line effect
- Rainbow line effect

### Network Capabilities
- WebSocket server for real-time control
- UDP server for data reception
- mDNS support (accessible via colormusicesp32.local)
- OTA (Over-The-Air) updates
- Static IP configuration
- WiFi connectivity

### Customizable Parameters
- LED strip brightness
- Effect speed
- Color settings
- Zone configurations
- Network settings

### Application
You can find the application for this project at the following link [`Color-Music`](https://github.com/Iurocl-git/Color-Music.git)

## Technical Specifications

### Hardware
- ESP32 microcontroller
- LED strip (compatible with FastLED library)
- Audio input module (configurable)
- Power supply

### Software
- PlatformIO development environment
- FastLED library for LED control
- WebSocket and UDP protocols
- WiFi connectivity

### LED Strip Support
- Supports all LED strips compatible with the FastLED library
- Configurable number of LEDs
- Adjustable brightness and color settings

## Project Structure

```
lib/
├── config/         # Configuration files
├── leds/          # LED control library
├── network/       # Network functionality
└── files/         # Core project files
    ├── config.h   # Global configuration
    ├── led_effects.h/cpp  # LED effects implementation
    └── network.h/cpp      # Network functionality
```

## Project Management

### Configuration
- Network settings (WiFi, IP, ports)
- LED strip parameters
- Effect settings
- Zone configurations

### Control Methods
- Web interface
- UDP commands
- OTA updates
- Serial debugging

## Requirements

### Hardware
- ESP32 development board
- LED strip compatible with FastLED
- Audio input module
- Power supply

### Software
- PlatformIO IDE
- FastLED library
- Arduino core for ESP32 
