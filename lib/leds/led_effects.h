#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

#include <algorithm> // For std::max and std::min
#include <cmath>     // For round()
#include "config.h"

// LED Modes Enum
enum LedMode { 
    STATIC,              // Static color mode
    RAINBOW_wave,        // Rainbow wave effect
    FLICKER_color,       // Color flicker effect
    STROBE_mode,         // Strobe light effect
    SINGLE_ZONE_freq,    // Single zone frequency visualization
    THREE_ZONE_freq,     // Three zone frequency visualization
    FIVE_ZONE_freq,      // Five zone frequency visualization
    CENTER_DROP_eff,     // Center drop effect
    CENTER_LINE_eff,     // Center line effect
    RAINBOW_LINE_eff     // Rainbow line effect
};

// Color State Structure for smooth transitions
struct ColorState {
    CRGB currentColor;   // Current color of the zone
    CRGB targetColor;    // Target color to transition to
    float brightness;    // Current brightness level
};

// Global Variables
extern CRGB leds[NUM_LEDS];           // LED strip array
extern ColorState zoneColors[3];       // Color states for three zones
extern ColorState singleZoneColor;     // Color state for single zone
extern CRGB staticColor;               // Static color for static mode
extern LedMode currentMode;            // Current display mode

// Function Declarations
void showStaticColor(CRGB color);      // Display static color
void rainbowWave(int width = NUM_LEDS, int startOffset = 0);  // Rainbow wave effect
void flickerColor();                   // Color flicker effect
void strobeMode();                     // Strobe light effect
void strobeEffect(CRGB* ledsArray, int numLeds, CRGB color, bool isOn);  // Helper function for strobe
void singleZoneFrequencyVisualization();  // Single zone frequency visualization
void threeZoneFrequencyVisualization();   // Three zone frequency visualization
void fiveZoneFrequencyVisualization();    // Five zone frequency visualization
void centerDropEffect();                  // Center drop effect
void centerLineEffect();                  // Center line effect
void rainbowLineEffect();                 // Rainbow line effect

#endif 