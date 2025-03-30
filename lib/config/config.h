#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>
#include <Arduino.h>

// WiFi && WebSocket Settings
extern const char* ssid;
extern const char* password;
extern const int webSocketPort;

// IP Settings
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;

// UDP Settings
extern const unsigned int localUdpPort;
const int maxPacketSize = 50;

// LED Settings
#define LED_PIN    4
#define NUM_LEDS  296
#define NOISE 500
extern int LED_START;

// General Settings
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

// Frequency Sensitivity
extern float sensitivityLOW;
extern float sensitivityMID;
extern float sensitivityHIGH;

// Colors for frequency mode
extern const CRGB lowColor;
extern const CRGB midColor;
extern const CRGB highColor;

// Magnitude Setup
extern int amplitudeThreshold;
#define MAX_AMPLITUDE 4095

// Frequency Amplitudes
extern int lowFreqAmp;
extern int midFreqAmp;
extern int highFreqAmp;

// Static Color
extern CRGB staticColor;


// --- Параметры эффекта ---
extern int MAX_DROPS;
extern float DROP_EXPANSION_SPEED; // Скорость движения капли (пикселей в мс)
extern int MIN_TRIGGER_INTERVAL;// Мин. интервал создания капель (мс)
extern int CENTER_ZONE_SIZE;// Размер центральной зоны
extern int DROP_WIDTH;        // ДЛИНА капли/хвоста (в светодиодах) - можно настроить

// --- Стиль затухания хвоста ---
// 0: Линейное затухание (просто)
// 1: Квадратичное затухание (хвост затухает быстрее)
// 2: Корень квадратный (хвост затухает медленнее)
extern int FADE_STYLE;

#endif