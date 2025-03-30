#include "config.h"

// WiFi && WebSocket Settings
const char* ssid = "Balti";
const char* password = "987654321";
const int webSocketPort = 81;

// IP Settings
IPAddress local_IP(192, 168, 85, 51);
IPAddress gateway(192, 168, 85, 1);
IPAddress subnet(255, 255, 255, 0);

// UDP Settings
const unsigned int localUdpPort = 8888;
// const int maxPacketSize = 50;

// LED Settings
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
int amplitudeThreshold = 3000;

// Frequency Amplitudes
int lowFreqAmp = 0;
int midFreqAmp = 0;
int highFreqAmp = 0;

// Static Color
CRGB staticColor = CRGB::Blue;

// --- Параметры эффекта ---
int MAX_DROPS = 15;
float DROP_EXPANSION_SPEED = 0.1; // Скорость движения капли (пикселей в мс)
int MIN_TRIGGER_INTERVAL = 300; // Мин. интервал создания капель (мс)
int CENTER_ZONE_SIZE = 20; // Размер центральной зоны
int DROP_WIDTH = 15;       // ДЛИНА капли/хвоста (в светодиодах) - можно настроить

// --- Стиль затухания хвоста ---
// 0: Линейное затухание (просто)
// 1: Квадратичное затухание (хвост затухает быстрее)
// 2: Корень квадратный (хвост затухает медленнее)
int FADE_STYLE = 1;