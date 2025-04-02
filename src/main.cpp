// #include "../lib/files/config.h"
// #include "../lib/files/led_effects.h"
// #include "../lib/files/network.h"
#include "config.h"
#include "led_effects.h"
#include "network.h"

void setup() {
    Serial.begin(115200);

    // WiFi Configuration
    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("Failed to configure static IP");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.print("Connected to WiFi with IP ");
    Serial.println(WiFi.localIP());

    // mDNS Configuration
    if (!MDNS.begin("colormusicesp32")) {  // Domain will be esp32.local
        Serial.println("Error starting mDNS");
    } else {
        Serial.println("mDNS responder started: colormusicesp32.local");
    }

    // WebSocket Configuration
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started.");

    // UDP Configuration
    udp.begin(localUdpPort);
    Serial.printf("Now listening for UDP packets on port %d\n", localUdpPort);

    // OTA Configuration
    setupOTA();

    // LED Strip Configuration
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.setBrightness(brightness);
    Serial.println("Strip started.");

    // Initialize color states
    for(int i = 0; i < 3; i++) {
        zoneColors[i].currentColor = CRGB::Black;
        zoneColors[i].targetColor = CRGB::Black;
        zoneColors[i].brightness = 0;
    }

    singleZoneColor.currentColor = CRGB::Black;
    singleZoneColor.targetColor = CRGB::Black;
    singleZoneColor.brightness = 0;
}

void loop() {
    // Handle OTA requests
    ArduinoOTA.handle();
    
    // Handle network events
    webSocket.loop();
    handleUdpData();

    // Handle LED effects
    switch (currentMode) {
        case STATIC:
            showStaticColor(staticColor);
            break;
        case RAINBOW_wave:
            rainbowWave();
            break;
        case FLICKER_color:
            flickerColor();
            break;
        case STROBE_mode:
            strobeMode();
            break;
        case SINGLE_ZONE_freq:
            singleZoneFrequencyVisualization();
            break;
        case THREE_ZONE_freq:
            threeZoneFrequencyVisualization();
            break;
        case FIVE_ZONE_freq:
            fiveZoneFrequencyVisualization();
            break;
        case CENTER_DROP_eff:
            centerDropEffect();
            break;
        case CENTER_LINE_eff:
            centerLineEffect();
            break;
        case RAINBOW_LINE_eff:
            rainbowLineEffect();
            break;
        default:
            break;
    }
    
    FastLED.show();
}