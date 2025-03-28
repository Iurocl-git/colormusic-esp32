#include "led_effects.h"

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
    static unsigned long lastTrigger = 0;
    unsigned long currentTime = millis();
    
    bool triggered = false;
    CRGB dropColor = CRGB::Black;
    
    if(lowFreqAmp * sensitivity > amplitudeThreshold) {
        triggered = true;
        dropColor = lowColor;
    }
    if(midFreqAmp * sensitivity > amplitudeThreshold) {
        triggered = true;
        dropColor = midColor;
    }
    if(highFreqAmp * sensitivity > amplitudeThreshold) {
        triggered = true;
        dropColor = highColor;
    }
    
    if(triggered) {
        lastTrigger = currentTime;
    }
    
    float fadeFactor = pulseDecay;
    
    int center = NUM_LEDS / 2;
    for(int i = 0; i < NUM_LEDS; i++) {
        int distance = abs(i - center);
        float brightness = 255.0 / (distance + 1);
        CRGB target = dropColor;
        target.nscale8((uint8_t)(brightness * fadeFactor));
        leds[i] = blendColors(leds[i], target, transitionSpeed);
    }
}

void centerLineEffect() {
    int totalAmp = (lowFreqAmp + midFreqAmp + highFreqAmp) * sensitivity;
    int lineLength = map(totalAmp, 0, 255 * 3, 0, (NUM_LEDS - LED_START) / 2);
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
    int rainbowWidth = map(totalAmp, 0, 255 * 3, 0, activeLength);
    rainbowWave(rainbowWidth, LED_START);
} 