#include "network.h"

// Global variables definitions
WebSocketsServer webSocket(webSocketPort);
WiFiUDP udp;
char incomingPacket[MAX_PACKET_SIZE];
LedMode currentMode = RAINBOW_wave;


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
        int len = udp.read(incomingPacket, MAX_PACKET_SIZE);
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
        // if (cmd.indexOf("0") != -1) {
        //     currentMode = STATIC;
        // } else if (cmd.indexOf("2") != -1) {
        //     currentMode = RAINBOW_wave;
        // } else if (cmd.indexOf("1") != -1) {
        //     currentMode = FLICKER_color;
        // } else if (cmd.indexOf("3") != -1) {
        //     currentMode = STROBE_mode;
        // } else if (cmd.indexOf("4") != -1) {
        //     currentMode = THREE_ZONE_freq;
        // } else if (cmd.indexOf("5") != -1) {
        //     currentMode = SINGLE_ZONE_freq;
        // } else if (cmd.indexOf("6") != -1) {
        //     currentMode = CENTER_DROP_eff;
        // } else if (cmd.indexOf("8") != -1) {
        //     currentMode = RAINBOW_LINE_eff;
        // } else if (cmd.indexOf("7") != -1) {
        //     currentMode = CENTER_LINE_eff;
        // } else if (cmd.indexOf("9") != -1) {
        //     currentMode = FIVE_ZONE_freq;
        // }
        int mode = getValueFromJSON(cmd, "mode");
        switch(mode) {
            case MODE_STATIC:
                currentMode = STATIC;
                break;
            case MODE_FLICKER:
                currentMode = FLICKER_color;
                break;
            case MODE_RAINBOW_WAVE:
                currentMode = RAINBOW_wave;
                break;
            case MODE_STROBE:
                currentMode = STROBE_mode;
                break;
            case MODE_THREE_ZONE:
                currentMode = THREE_ZONE_freq;
                break;
            case MODE_SINGLE_ZONE:
                currentMode = SINGLE_ZONE_freq;
                break;
            case MODE_CENTER_DROP:
                currentMode = CENTER_DROP_eff;
                break;
            case MODE_CENTER_LINE:
                currentMode = CENTER_LINE_eff;
                break;
            case MODE_RAINBOW_LINE:
                currentMode = RAINBOW_LINE_eff;
                break;
            case MODE_FIVE_ZONE:
                currentMode = FIVE_ZONE_freq;
                break;
            default:
                break;
        }
    } else if (cmd.indexOf("parameterChange") != -1) {
        int name = getValueFromJSON(cmd, "name");
        switch(name) {
            case PARAM_SENSITIVITY:
                sensitivity = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_TRANSITION_SPEED:
                transitionSpeed = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_PULSE_DECAY:
                pulseDecay = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_RAINBOW_SPEED:
                rainbowSpeed = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_WAVE_WIDTH:
                waveWidth = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_FLICKER_SPEED:
                flickerSpeed = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_STROBE_FREQUENCY:
                strobeFrequency = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_BRIGHTNESS:
                brightness = getValueFromJSON(cmd,"value");
                FastLED.setBrightness(brightness);
                break;
            case PARAM_LED_START:
                LED_START = getValueFromJSON(cmd,"value");
                break;
            case PARAM_SENSITIVITY_LOW:
                sensitivityLOW = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_SENSITIVITY_MID:
                sensitivityMID = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_SENSITIVITY_HIGH:
                sensitivityHIGH = getFloatFromJSON(cmd,"value");
                break;
            case PARAM_MAX_DROPS:
                MAX_DROPS = getValueFromJSON(cmd, "value");
                break;
            case PARAM_DROP_EXPANSION_SPEED:
                DROP_EXPANSION_SPEED = getFloatFromJSON(cmd, "value");
                break;
            case PARAM_MIN_TRIGGER_INTERVAL:
                MIN_TRIGGER_INTERVAL = getValueFromJSON(cmd, "value");
                break;
            case PARAM_CENTER_ZONE_SIZE:
                CENTER_ZONE_SIZE = getValueFromJSON(cmd, "value");
                break;
            case PARAM_DROP_WIDTH:
                DROP_WIDTH = getValueFromJSON(cmd, "value");
                break;
            case PARAM_FADE_STYLE:
                FADE_STYLE = getValueFromJSON(cmd, "value");
                break;
            case PARAM_AMPLITUDE_THRESHOLD:
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
    highFreqAmp = getValueFromJSON(data, "HFA") * sensitivityHIGH;
} 