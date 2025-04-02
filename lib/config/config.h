#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>
#include <Arduino.h>

// Display Modes Enum
enum DisplayMode {
    MODE_STATIC = 0,
    MODE_FLICKER = 1,
    MODE_RAINBOW_WAVE = 2,
    MODE_STROBE = 3,
    MODE_THREE_ZONE = 4,
    MODE_SINGLE_ZONE = 5,
    MODE_CENTER_DROP = 6,
    MODE_CENTER_LINE = 7,
    MODE_RAINBOW_LINE = 8,
    MODE_FIVE_ZONE = 9
};

// Effect Parameters Enum
enum EffectParameter {
    PARAM_SENSITIVITY = 0,
    PARAM_TRANSITION_SPEED = 1,
    PARAM_PULSE_DECAY = 2,
    PARAM_RAINBOW_SPEED = 3,
    PARAM_WAVE_WIDTH = 4,
    PARAM_FLICKER_SPEED = 5,
    PARAM_STROBE_FREQUENCY = 6,
    PARAM_BRIGHTNESS = 7,
    PARAM_LED_START = 8,
    PARAM_SENSITIVITY_LOW = 9,
    PARAM_SENSITIVITY_MID = 10,
    PARAM_SENSITIVITY_HIGH = 11,
    PARAM_MAX_DROPS = 12,
    PARAM_DROP_EXPANSION_SPEED = 13,
    PARAM_MIN_TRIGGER_INTERVAL = 14,
    PARAM_CENTER_ZONE_SIZE = 15,
    PARAM_DROP_WIDTH = 16,
    PARAM_FADE_STYLE = 17,
    PARAM_AMPLITUDE_THRESHOLD = 18
};

// Network Configuration
extern const char* ssid;
extern const char* password;
extern const int webSocketPort;
extern const unsigned int localUdpPort;
#define MAX_PACKET_SIZE 50

// IP Configuration
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;

// LED Strip Configuration
#define LED_PIN    4
#define NUM_LEDS  296
#define NOISE 500
extern int LED_START;

// Effect Parameters
extern int MAX_DROPS;
extern float DROP_EXPANSION_SPEED;    // Drop movement speed (pixels per ms)
extern int MIN_TRIGGER_INTERVAL;      // Minimum interval between drops (ms)
extern int CENTER_ZONE_SIZE;          // Size of the center zone
extern int DROP_WIDTH;                // Length of drop/tail (in LEDs)

// Fade Style Configuration
// 0: Linear fade (simple)
// 1: Quadratic fade (tail fades faster)
// 2: Square root fade (tail fades slower)
extern int FADE_STYLE;

// General Effect Settings
extern float transitionSpeed;
extern float sensitivity;
extern float pulseDecay;
extern float rainbowSpeed;
extern float waveWidth;
extern float flickerSpeed;
extern float strobeFrequency;
extern int brightness;
extern float minBrightness;
extern float minBrightnessZones;
extern float minPulseInterval;

// Frequency Analysis Settings
extern float sensitivityLOW;
extern float sensitivityMID;
extern float sensitivityHIGH;
extern int amplitudeThreshold;
#define MAX_AMPLITUDE 4095

// Frequency Amplitudes
extern int lowFreqAmp;
extern int midFreqAmp;
extern int highFreqAmp;

// Color Settings
extern const CRGB lowColor;
extern const CRGB midColor;
extern const CRGB highColor;
extern CRGB staticColor;

#endif