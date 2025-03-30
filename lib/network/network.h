#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "led_effects.h"
#include "config.h"
#include <ESPmDNS.h>

// Global variables declarations
extern WebSocketsServer webSocket;
extern WiFiUDP udp;
extern char incomingPacket[maxPacketSize];
extern int lowFreqAmp;
extern int midFreqAmp;
extern int highFreqAmp;
extern int freqAmplitudes[32];
extern String serverURL;  // This will hold the URL
extern IPAddress serverIP;  // This will store the resolved IP address


// Function Declarations
void setupOTA();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleUdpData();
void handleTextCommand(const char* message);
float getFloatFromJSON(String json, String key);
int getValueFromJSON(String json, String key);
String getStringFromJSON(String json, String key);
void processAudioData(String data);
void resolveHostName();

#endif 