#include "network.h"

// Global variables definitions
WebSocketsServer webSocket(webSocketPort);
WiFiUDP udp;
char incomingPacket[maxPacketSize];
int freqAmplitudes[32];
LedMode currentMode = RAINBOW_wave;

String serverURL = "ColorMusicESP32.com";  // Установите ваш URL
IPAddress serverIP;

void resolveHostName() {
    if (WiFi.hostByName(serverURL.c_str(), serverIP)) {
        Serial.print("Resolved IP for ");
        Serial.print(serverURL);
        Serial.print(": ");
        Serial.println(serverIP);
    } else {
        Serial.println("DNS lookup failed.");
    }
}


void setupOTA() {
    ArduinoOTA.setHostname("ESP32-OTA");
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial.println("OTA Ready");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_TEXT:
            handleTextCommand((char*)payload);
            break;
        case WStype_BIN:
            break;
    }
}

void handleUdpData() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        int len = udp.read(incomingPacket, maxPacketSize);
        if (len > 0) {
            incomingPacket[len] = 0;
            String receivedMessage = String(incomingPacket);

            int lowIndex = receivedMessage.indexOf("low:") + 4;
            int midIndex = receivedMessage.indexOf("mid:") + 4;
            int highIndex = receivedMessage.indexOf("high:") + 5;

            lowFreqAmp = receivedMessage.substring(lowIndex, receivedMessage.indexOf(",", lowIndex)).toInt();
            midFreqAmp = receivedMessage.substring(midIndex, receivedMessage.indexOf(",", midIndex)).toInt();
            highFreqAmp = receivedMessage.substring(highIndex).toInt();
        }
    }
}

void handleTextCommand(const char* message) {
    String cmd = String(message);

    if (cmd.indexOf("changeMode") != -1) {
        FastLED.clear();
        if (cmd.indexOf("0") != -1) {
            currentMode = STATIC;
        } else if (cmd.indexOf("2") != -1) {
            currentMode = RAINBOW_wave;
        } else if (cmd.indexOf("1") != -1) {
            currentMode = FLICKER_color;
        } else if (cmd.indexOf("3") != -1) {
            currentMode = STROBE_mode;
        } else if (cmd.indexOf("4") != -1) {
            currentMode = THREE_ZONE_freq;
        } else if (cmd.indexOf("5") != -1) {
            currentMode = SINGLE_ZONE_freq;
        } else if (cmd.indexOf("6") != -1) {
            currentMode = CENTER_DROP_eff;
        } else if (cmd.indexOf("8") != -1) {
            currentMode = RAINBOW_LINE_eff;
        } else if (cmd.indexOf("7") != -1) {
            currentMode = CENTER_LINE_eff;
        } else if (cmd.indexOf("9") != -1) {
            currentMode = FIVE_ZONE_freq;
        }
    } else if (cmd.indexOf("parameterChange") != -1) {
        int name = getValueFromJSON(cmd, "name");
        switch(name) {
            case 0:
                sensitivity = getFloatFromJSON(cmd,"value");
                break;
            case 1:
                transitionSpeed = getFloatFromJSON(cmd,"value");
                break;
            case 2:
                pulseDecay = getFloatFromJSON(cmd,"value");
                break;
            case 3:
                rainbowSpeed = getFloatFromJSON(cmd,"value");
                break;
            case 4:
                waveWidth = getFloatFromJSON(cmd,"value");
                break;
            case 5:
                flickerSpeed = getFloatFromJSON(cmd,"value");
                break;
            case 6:
                strobeFrequency = getFloatFromJSON(cmd,"value");
                break;
            case 7:
                brightness = getValueFromJSON(cmd,"value");
                // Serial.print("sensitivityHIGH ");
                // Serial.println(brightness);
                FastLED.setBrightness(brightness);
                break;
            case 8:
                LED_START = getValueFromJSON(cmd,"value");
                break;
            case 9:
                sensitivityLOW = getFloatFromJSON(cmd,"value");
                break;
            case 10:
                sensitivityMID = getFloatFromJSON(cmd,"value");
                break;
            case 11:
                sensitivityHIGH = getFloatFromJSON(cmd,"value");
                // Serial.print("sensitivityHIGH ");
                Serial.println(sensitivityHIGH);
                break;
            case 12:
                // Serial.print("MAX_DROPS ");
                // Serial.println(getValueFromJSON(cmd, "value"));
                MAX_DROPS = getValueFromJSON(cmd, "value");
                break;
            case 13:
                DROP_EXPANSION_SPEED = getFloatFromJSON(cmd, "value");
                break;
            case 14:
                // Serial.print("MIN_TRIGGER_INTERVAL ");
                // Serial.println(getValueFromJSON(cmd, "value"));
                MIN_TRIGGER_INTERVAL = getValueFromJSON(cmd, "value");
                break;
            case 15:
                CENTER_ZONE_SIZE = getValueFromJSON(cmd, "value");
                break;
            case 16:
                DROP_WIDTH = getValueFromJSON(cmd, "value");
                break;
            case 17:
                FADE_STYLE = getValueFromJSON(cmd, "value");
                break;
            case 18:
                amplitudeThreshold = getFloatFromJSON(cmd, "value");
                break;
        }
    } else if (cmd.indexOf("changeColor") != -1) {
        int r = getValueFromJSON(cmd, "r");
        int g = getValueFromJSON(cmd, "g");
        int b = getValueFromJSON(cmd, "b");
        staticColor = CRGB(r, g, b);
    }
}

float getFloatFromJSON(String json, String key) {
    int startIndex = json.indexOf("\"" + key + "\":") + key.length() + 3;
    int endIndex = json.indexOf(",", startIndex);
    if (endIndex == -1) {
        endIndex = json.indexOf("}", startIndex);
    }
    return json.substring(startIndex, endIndex).toFloat();
}

int getValueFromJSON(String json, String key) {
    int startIndex = json.indexOf("\"" + key + "\":") + key.length() + 3;
    int endIndex = json.indexOf(",", startIndex);
    if (endIndex == -1) {
        endIndex = json.indexOf("}", startIndex);
    }
    return json.substring(startIndex, endIndex).toInt();
}

String getStringFromJSON(String json, String key) {
    int startIndex = json.indexOf("\"" + key + "\":") + key.length() + 3;
    int endIndex = json.indexOf(",", startIndex);
    if (endIndex == -1) {
        endIndex = json.indexOf("}", startIndex);
    }
    return String(json.substring(startIndex, endIndex));
}

void processAudioData(String data) {
    lowFreqAmp = getValueFromJSON(data, "LFA") * sensitivityLOW;
    midFreqAmp = getValueFromJSON(data, "MFA") * sensitivityMID;
    highFreqAmp = getValueFromJSON(data, "HFA") * sens