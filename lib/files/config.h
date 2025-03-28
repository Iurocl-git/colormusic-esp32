#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>

// WiFi && WebSocket Settings
const char* ssid = "Pixel_7";
const char* password = "987654321";
const int webSocketPort = 81;

// IP Settings
IPAddress local_IP(192, 168, 190, 120);
IPAddress gateway(192, 168, 190, 1);
IPAddress subnet(255, 255, 255, 0);

// UDP Settings
const unsigned int localUdpPort = 8888;
const int maxPacketSize = 255;

// LED Settings
#define LED_PIN    4
#define NUM_LEDS  296
#define NOISE 500
int LED_START = 100;

// General Settings
float transitionSpeed = 0.05;
float sensitivity = 1.0;
float pulseDecay = 0.95;
float rainbowSpeed = 0.1;
float waveWidth = 100.0;
float flickerSpeed = 0.05;
float strobeFrequency = 10.0;
int brightness = 50;
float minBrightness = 0.1;
float minBrightnessZones = 20;
float minPulseInterval = 200;

// Frequency Sensitivity
float sensitivityLOW = 1.0;
float sensitivityMID = 1.0;
float sensitivityHIGH = 1.0;

// Colors for frequency mode
const CRGB lowColor = CRGB::Red;
const CRGB midColor = CRGB::Green;
const CRGB highColor = CRGB::Blue;

// Magnitude Setup
int amplitudeThreshold = 150;

// Frequency Amplitudes
int lowFreqAmp = 0;
int midFreqAmp = 0;
int highFreqAmp = 0;

// LED Mode
// LedMode currentMode = RAINBOW_wave;

// Static Color
CRGB staticColor = CRGB::Blue;

#endif 