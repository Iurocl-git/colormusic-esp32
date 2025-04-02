#include "config.h"

// Network Configuration
const char* ssid = "Balti";
const char* password = "987654321";
const int webSocketPort = 81;
const unsigned int localUdpPort = 8888;

// IP Configuration
IPAddress local_IP(192, 168, 85, 51);
IPAddress gateway(192, 168, 85, 1);
IPAddress subnet(255, 255, 255, 0);

// LED Strip Configuration
int LED_START = 100;

// Effect Parameters
int MAX_DROPS = 15;
float DROP_EXPANSION_SPEED = 0.1;    // Drop movement speed (pixels per ms)
int MIN_TRIGGER_INTERVAL = 300;      // Minimum interval between drops (ms)
int CENTER_ZONE_SIZE = 20;           // Size of the center zone
int DROP_WIDTH = 15;                 // Length of drop/tail (in LEDs)

// Fade Style Configuration
// 0: Linear fade (simple)
// 1: Quadratic fade (tail fades faster)
// 2: Square root fade (tail fades slower)
int FADE_STYLE = 1;

// General Effect Settings
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

// Frequency Analysis Settings
float sensitivityLOW = 1.0;
float sensitivityMID = 1.0;
float sensitivityHIGH = 1.0;
int amplitudeThreshold = 3000;

// Frequency Amplitudes
int lowFreqAmp = 0;
int midFreqAmp = 0;
int highFreqAmp = 0;

// Color Settings
const CRGB lowColor = CRGB::Red;
const CRGB midColor = CRGB::Green;
const CRGB highColor = CRGB::Blue;
CRGB staticColor = CRGB::Blue;