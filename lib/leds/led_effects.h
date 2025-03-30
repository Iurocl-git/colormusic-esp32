#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

// #include <FastLED.h>
#include <algorithm> // Для std::max и std::min (можно заменить на макросы FastLED если нужно)
#include <cmath>     // Для round()
#include "config.h"

// LED Modes
enum LedMode { 
    STATIC,  
    RAINBOW_wave,  
    FLICKER_color,  
    STROBE_mode, 
    SINGLE_ZONE_freq, 
    THREE_ZONE_freq, 
    FIVE_ZONE_freq,
    CENTER_DROP_eff,  
    CENTER_LINE_eff,  
    RAINBOW_LINE_eff 
};

// Color State Structure
struct ColorState {
    CRGB currentColor;
    CRGB targetColor;
    float brightness;
};

// Global variables
extern CRGB leds[NUM_LEDS];
extern ColorState zoneColors[3];
extern ColorState singleZoneColor;
extern CRGB staticColor;
extern LedMode currentMode;

// Function Declarations
void showStaticColor(CRGB color);
void rainbowWave(int width = NUM_LEDS, int startOffset = 0);
void flickerColor();
void strobeMode();
void strobeEffect(CRGB* ledsArray, int numLeds, CRGB color, bool isOn);
void singleZoneFrequencyVisualization();
void threeZoneFrequencyVisualization();
void fiveZoneFrequencyVisualization();
void centerDropEffect();
void centerLineEffect();
void rainbowLineEffect();

#endif 