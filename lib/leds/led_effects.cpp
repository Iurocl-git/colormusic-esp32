#include "led_effects.h"

#define MAX_DROPS_CONST 30           // Maximum number of drops

struct Drop {
    bool active = false;        // Is the drop active?
    CRGB color;                 // Drop color
    unsigned long startTime;    // Creation time (millis())
    float initialBrightness;    // Initial brightness (can depend on amplitude)
};

Drop drops[MAX_DROPS_CONST];         // Array to store drops
unsigned long lastDropTriggerTime = 0; // Last trigger time for any frequency

// Global variables
CRGB leds[NUM_LEDS];
ColorState zoneColors[3];
ColorState singleZoneColor;

// Helper functions
CRGB blendColors(const CRGB& current, const CRGB& target, float speed) {
    uint8_t r = lerp8by8(current.r, target.r, speed * 255 * 3);
    uint8_t g = lerp8by8(current.g, target.g, speed * 255 * 3);
    uint8_t b = lerp8by8(current.b, target.b, speed * 255 * 3);
    return CRGB(r, g, b);
}

void ledsFillSmooth(CRGB* ledsArray, int numLeds, CRGB target, float speed) {
    for(int i = 0; i < numLeds; i++) {
        ledsArray[i] = blendColors(ledsArray[i], target, speed);
    }
}

// LED Effect Implementations
void showStaticColor(CRGB color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    FastLED.show();
}

void rainbowWave(int width, int startOffset) {
    static float position = 0;
    position += rainbowSpeed;

    int start = (NUM_LEDS - width - startOffset) / 2 + startOffset;
    int end = start + width;

    for (int i = startOffset; i < NUM_LEDS; i++) {
        if (i >= start && i < end) {
            float wave = sin((i + position) / waveWidth);
            wave = map(wave, -1, 1, 0, 0.7) + 0.3;
            uint8_t hue = (i * 255 / NUM_LEDS) + (position * 10) + 0.2;
            CRGB target = CHSV(hue, 255, 255 * (wave * 0.5 + 0.5));
            leds[i] = blendColors(leds[i], target, transitionSpeed);
        } else {
            leds[i] = CRGB::Black;
        }
    }
    FastLED.show();
}

void flickerColor() {
    static float brightness = 0;
    brightness += flickerSpeed * 0.05;

    float flicker = (sin(brightness) * 0.5 + 0.5);
    flicker = flicker * (1.0 - minBrightness) + minBrightness;

    CRGB target = staticColor;
    target.nscale8(255 * flicker);

    ledsFillSmooth(leds, NUM_LEDS, target, transitionSpeed);
}

void strobeMode() {
    static unsigned long lastToggle = 0;
    static bool isOn = false;
    unsigned long interval = 1000 / (strobeFrequency * 2);

    if (millis() - lastToggle > interval) {
        lastToggle = millis();
        isOn = !isOn;
    }

    strobeEffect(leds, NUM_LEDS, CRGB::White, isOn);
}

void strobeEffect(CRGB* ledsArray, int numLeds, CRGB color, bool isOn) {
    if (isOn) {
        for (int i = 0; i < numLeds; i++) {
            ledsArray[i] = color;
        }
    } else {
        for (int i = 0; i < numLeds; i++) {
            ledsArray[i] = CRGB::Black;
        }
    }
    FastLED.show();
}

void singleZoneFrequencyVisualization() {
    static unsigned long lastImpulseTime = 0;
    static CRGB lastColor = CRGB::Black;

    int maxAmp = max(max(lowFreqAmp, midFreqAmp), highFreqAmp);
    CRGB targetColor = CRGB::Black;

    if (maxAmp == lowFreqAmp) {
        targetColor = lowColor;
    } else if (maxAmp == midFreqAmp) {
        targetColor = midColor;
    } else if (maxAmp == highFreqAmp) {
        targetColor = highColor;
    }

    if (maxAmp * sensitivity > amplitudeThreshold &&
        millis() - lastImpulseTime > minPulseInterval) {
        FastLED.clear();
        fill_solid(leds, NUM_LEDS, targetColor);
        lastColor = targetColor;
        lastImpulseTime = millis();
    } else {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));
        }
    }

    FastLED.show();
}

void threeZoneFrequencyVisualization() {
    static unsigned long lastImpulseTime[3] = {0, 0, 0};
    static CRGB lastColors[3] = {CRGB::Black, CRGB::Black, CRGB::Black};

    int zoneSize = (NUM_LEDS - LED_START) / 3;

    int amplitudes[3] = {lowFreqAmp, midFreqAmp, highFreqAmp};
    CRGB colors[3] = {lowColor, midColor, highColor};

    FastLED.clear();

    for (int zone = 0; zone < 3; zone++) {
        int start = LED_START + zone * zoneSize;
        int end = start + zoneSize;

        if (amplitudes[zone] * sensitivity > amplitudeThreshold &&
            millis() - lastImpulseTime[zone] > minPulseInterval) {
            for (int i = start; i < end; i++) {
                leds[i] = colors[zone];
            }
            lastColors[zone] = colors[zone];
            lastImpulseTime[zone] = millis();
        } else {
            for (int i = start; i < end; i++) {
                leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));
            }
        }
    }

    FastLED.show();
}

void fiveZoneFrequencyVisualization() {
    static unsigned long lastImpulseTime[5] = {0, 0, 0, 0, 0};
    static CRGB lastColors[5] = {CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black};

    int zoneSize = (NUM_LEDS - LED_START) / 5;

    int amplitudes[5] = {lowFreqAmp, midFreqAmp, highFreqAmp, midFreqAmp, lowFreqAmp};
    CRGB colors[5] = {lowColor, midColor, highColor, midColor, lowColor};

    FastLED.clear();

    for (int zone = 0; zone < 5; zone++) {
        int start = LED_START + zone * zoneSize;
        int end = start + zoneSize;

        if (amplitudes[zone] * sensitivity > amplitudeThreshold &&
            millis() - lastImpulseTime[zone] > minPulseInterval) {
            for (int i = start; i < end; i++) {
                leds[i] = colors[zone];
            }
            lastColors[zone] = colors[zone];
            lastImpulseTime[zone] = millis();
        } else {
            for (int i = start; i < end; i++) {
                leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));
            }
        }
    }

    FastLED.show();
}

void centerDropEffect() {
    unsigned long currentTime = millis();

    // Clear strip before drawing
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Check triggers and create new drops
    int amplitudes[] = {lowFreqAmp, midFreqAmp, highFreqAmp};
    CRGB colors[] = {lowColor, midColor, highColor};

    for (int i = 0; i < 3; ++i) {
        if (amplitudes[i] * sensitivity > amplitudeThreshold) {
            int freeSlot = -1;
            for (int j = 0; j < MAX_DROPS; ++j) {
                if (!drops[j].active) {
                    freeSlot = j;
                    break;
                }
            }
            if (freeSlot != -1 && (currentTime - lastDropTriggerTime > MIN_TRIGGER_INTERVAL)) {
                drops[freeSlot].active = true;
                drops[freeSlot].color = colors[i];
                drops[freeSlot].startTime = currentTime;
                drops[freeSlot].initialBrightness 
                    = map(amplitudes[i] * sensitivity, amplitudeThreshold, MAX_AMPLITUDE, 180, 255);
                drops[freeSlot].initialBrightness = constrain(drops[freeSlot].initialBrightness, 150, 255);
                lastDropTriggerTime = currentTime;
                break;
            }
        }
    }

    // Update and draw active drops
    int effectCenter = (NUM_LEDS + LED_START) / 2;
    int zoneHalfSize = CENTER_ZONE_SIZE / 2;
    int zoneStart = max(LED_START, effectCenter - zoneHalfSize);
    int zoneEndCalc = effectCenter + zoneHalfSize - ((CENTER_ZONE_SIZE % 2 == 0) ? 1 : 0);
    int zoneEnd = min(NUM_LEDS - 1, zoneEndCalc);
    zoneStart = min(zoneStart, zoneEnd); // Safety check

    for (int i = 0; i < MAX_DROPS; ++i) {
        if (drops[i].active) {
            unsigned long age = currentTime - drops[i].startTime;
            int offset = round((float)age * DROP_EXPANSION_SPEED);

            // Define START and END positions of the drop segment
            // Left drop: from leftmost pixel (tip) to rightmost (tail)
            int left_leading_edge = zoneStart - 1 - offset - (DROP_WIDTH - 1); // Leftmost
            int left_trailing_edge = zoneStart - 1 - offset;                // Rightmost
            // Right drop: from leftmost pixel (tail) to rightmost (tip)
            int right_trailing_edge = zoneEnd + 1 + offset;                 // Leftmost
            int right_leading_edge = zoneEnd + 1 + offset + (DROP_WIDTH - 1); // Rightmost

            // --- DEACTIVATION CONDITION ---
            // Deactivate when tail (pixel closest to center) is completely off the strip
            if ( (left_trailing_edge < 0) && (right_trailing_edge >= NUM_LEDS) ) {
                drops[i].active = false;
                continue; // This drop is no longer drawn
            }

            // --- Draw LEFT drop with GRADIENT ---
            for (int k = left_leading_edge; k <= left_trailing_edge; ++k) {
                // Check if pixel is in valid zone (INSIDE strip and LEFT of center zone)
                if (k >= 0 && k < zoneStart) {
                    // Calculate pixel position within drop (0=tip, DROP_WIDTH-1=tail)
                    int pos_in_drop = k - left_leading_edge;
                    // Calculate brightness factor (0.0 at tail, 1.0 at tip)
                    float relativePos = (float)pos_in_drop / (DROP_WIDTH > 1 ? (DROP_WIDTH - 1) : 1); // from 0 to 1
                    float brightnessFactor;

                    if(FADE_STYLE == 0) // Linear
                        brightnessFactor = 1.0 - relativePos;
                    else if(FADE_STYLE == 1) // Quadratic (fast decay)
                        brightnessFactor = pow(1.0 - relativePos, 2);
                    else if(FADE_STYLE == 2) // Square root (slow decay)
                        brightnessFactor = sqrt(1.0 - relativePos);
                    else // Default to linear
                        brightnessFactor = 1.0 - relativePos;

                    brightnessFactor = constrain(brightnessFactor, 0.0, 1.0);
                    uint8_t pixelBrightness = drops[i].initialBrightness * brightnessFactor;

                    if (pixelBrightness > 3) { // Draw only if brightness is noticeable
                        CRGB finalPixelColor = drops[i].color;
                        finalPixelColor.nscale8(pixelBrightness);
                        leds[k] = finalPixelColor; // Assign calculated color
                    }
                }
            } // End of left drop drawing

            // --- Draw RIGHT drop with GRADIENT ---
            for (int k = right_trailing_edge; k <= right_leading_edge; ++k) {
                // Check if pixel is in valid zone (INSIDE strip and RIGHT of center zone)
                if (k < NUM_LEDS && k > zoneEnd) {
                    // Calculate pixel position within drop (0=tip, DROP_WIDTH-1=tail)
                    // Tip is at right_leading_edge
                    int pos_in_drop = right_leading_edge - k;
                    // Calculate brightness factor (0.0 at tail, 1.0 at tip)
                    float relativePos = (float)pos_in_drop / (DROP_WIDTH > 1 ? (DROP_WIDTH - 1) : 1); // from 0 to 1
                    float brightnessFactor;

                    if(FADE_STYLE == 0) // Linear
                        brightnessFactor = 1.0 - relativePos;
                    else if(FADE_STYLE == 1) // Quadratic (fast decay)
                        brightnessFactor = pow(1.0 - relativePos, 2);
                    else if(FADE_STYLE == 2) // Square root (slow decay)
                        brightnessFactor = sqrt(1.0 - relativePos);
                    else // Default to linear
                        brightnessFactor = 1.0 - relativePos;

                    brightnessFactor = constrain(brightnessFactor, 0.0, 1.0);
                    uint8_t pixelBrightness = drops[i].initialBrightness * brightnessFactor;

                    if (pixelBrightness > 3) { // Draw only if brightness is noticeable
                        CRGB finalPixelColor = drops[i].color;
                        finalPixelColor.nscale8(pixelBrightness);
                        leds[k] = finalPixelColor; // Assign calculated color
                    }
                }
            } // End of right drop drawing

            // --- Light up center zone ON CREATION ---
            if (age < 35) {
                // Color with maximum brightness for flash
                CRGB flashColor = drops[i].color;
                flashColor.nscale8(drops[i].initialBrightness);
                for (int k = zoneStart; k < zoneEnd; ++k) {
                    if(k >= 0 && k < NUM_LEDS) {
                        leds[k] = flashColor;
                    }
                }
            }

        } // end if (drops[i].active)
    } // end drops loop (i)

    // Show result
    FastLED.show();
}

void centerLineEffect() {
    int totalAmp = (lowFreqAmp + midFreqAmp + highFreqAmp) * sensitivity;
    int lineLength = map(totalAmp, 0, MAX_AMPLITUDE * 3, 0, (NUM_LEDS - LED_START) / 2);
    CRGB targetColor = staticColor;
    
    for(int i = LED_START; i < NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(255 - (int)(transitionSpeed * 255));
    }
    
    for(int i = 0; i < lineLength; i++) {
        int left = ((NUM_LEDS + LED_START) / 2) - i;
        int right = ((NUM_LEDS + LED_START) / 2) + i;
        if(left >= LED_START) {
            leds[left] = blendColors(leds[left], targetColor, transitionSpeed);
        }
        if(right < NUM_LEDS) {
            leds[right] = blendColors(leds[right], targetColor, transitionSpeed);
        }
    }
}

void rainbowLineEffect() {
    int activeLength = NUM_LEDS - LED_START;
    int totalAmp = (lowFreqAmp + midFreqAmp + highFreqAmp) * sensitivity;
    int rainbowWidth = map(totalAmp, 0, MAX_AMPLITUDE * 3, 0, activeLength);
    rainbowWave(rainbowWidth, LED_START);
} 