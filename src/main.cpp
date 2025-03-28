#include <WiFi.h>
#include <WebSocketsServer.h>
#include <FastLED.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h> // Библиотека для OTA загрузки
#include <math.h>

////!  WiFi && WebSocket Settings                         *!
////!                                                     *!
const char* ssid = "Pixel_7";
const char* password = "987654321";
WebSocketsServer webSocket = WebSocketsServer(81);


IPAddress local_IP(192, 168, 190, 120);   // Желаемый IP-адрес
IPAddress gateway(192, 168, 190, 1);
    // Адрес шлюза (Pixel 7)
IPAddress subnet(255, 255, 255, 0);    // Маска подсети
// Настройки UDP
WiFiUDP udp;
unsigned int localUdpPort = 8888;  // Порт для UDP
char incomingPacket[255];    

////!  LEDS Settings                                      *!
////!                                                     *!
#define LED_PIN    4       // Пин, к которому подключена светодиодная лента
#define NUM_LEDS  296     //Количество светодиодов в ленте
#define NOISE 500
CRGB leds[NUM_LEDS];
int LED_START = 100;


////!  General Settings                                   *!
////!                                                     *!
float sensitivity = 1.0;      // Чувствительность к аудио сигналу
float transitionSpeed = 0.05; // Скорость перехода цвета (0.0 - 1.0)
float pulseDecay = 0.95;      // Коэффициент затухания импульса (0.0 - 1.0)
float rainbowSpeed = 0.1;     // Скорость перемещения радуги
float waveWidth = 100.0;       // Ширина волны радуги
CRGB staticColor = CRGB::Blue; // Цвет для статических режимов
float flickerSpeed = 0.05;     // Скорость мерцания
float strobeFrequency = 10.0;  // Частота стробоскопа
int brightness = 50;
float minBrightness = 0.1;     //Минимальная яркость при мерцании
float minBrightnessZones = 20; 
float minPulseInterval = 200;   //Минимальное время между импульсами в музыке в ms


float sensitivityLOW = 1.0;
float sensitivityMID = 1.0;
float sensitivityHIGH = 1.0;


////!  Colors for frequency mode                          *!
////!                                                     *!
CRGB lowColor = CRGB::Red;
CRGB midColor = CRGB::Green;
CRGB highColor = CRGB::Blue;


////! Magnitude Setup                                     *!
////!                                                     *!
const int amplitudeThreshold = 150;


////! Audio Settings                                      *!
////!                                                     *!
int lowFreqAmp = 0;
int midFreqAmp = 0;
int highFreqAmp = 0;


////  Number of frequency slices                           *!
////                                                      *!
int freqAmplitudes[32];


////  Modes settings                                      *!
////                                                      *!
enum LedMode { 
  STATIC,  RAINBOW_wave,  FLICKER_color,  STROBE_mode, 
  SINGLE_ZONE_freq, THREE_ZONE_freq, FIVE_ZONE_freq,
  CENTER_DROP_eff,  CENTER_LINE_eff,  RAINBOW_LINE_eff 
  };
const int TOTAL_MODES = 8;
LedMode currentMode = RAINBOW_wave;


//// Settings for smoothness of the pulse                 *!
////                                                      *!
struct ColorState {
  CRGB currentColor;
  CRGB targetColor;
  float brightness;
};

ColorState zoneColors[3]; // color = 3 zones
ColorState singleZoneColor; // color = single zone



//// Functions 
////
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleUdpData();
void handleTextCommand(const char* message);
float getFloatFromJSON(String json, String key);
int getValueFromJSON(String json, String key);
String getStringFromJSON(String json, String key);
void processAudioData(String data);
CRGB blendColors(const CRGB& current, const CRGB& target, float speed);
void ledsFillSmooth(CRGB* ledsArray, int numLeds, CRGB target, float speed);
void switchMode();
void setMode(LedMode mode);
void showStaticColor(CRGB color);
// void rainbowWave();
void rainbowWave(int rainbowWidth = NUM_LEDS, int startOffset = 0);
void flickerColor();
void strobeMode();
void strobeEffect(CRGB* ledsArray, int numLeds, CRGB color, bool isOn);
void singleZoneFrequencyVisualization();
void threeZoneFrequencyVisualization();
void fiveZoneFrequencyVisualization();
void fadeWithMinBrightness(CRGB& led, uint8_t minBrightness, float pulseDecay);
void centerDropEffect();
void centerLineEffect();
void rainbowLineEffect();



void setupOTA() {
  ArduinoOTA.setHostname("ESP32-OTA"); // Устанавливаем имя устройства
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
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
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


////////%%%%%%%%      SETUP     ////////%%%%%%%%
////////%%%%%%%%                ////////%%%%%%%%

void setup() {
  Serial.begin(115200);

//////     Setting up WiFI connection
//////
  // Настройка статического IP
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to configure static IP");
  }

  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Вывод информации о подключении
  Serial.print("Connected to WiFi with IP ");
  Serial.println(WiFi.localIP());


//////     Setting up webSocket connection for Fetching data
//////
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started.");


  // Инициализируем UDP
  udp.begin(localUdpPort);
  Serial.printf("Now listening for UDP packets on port %d\n", localUdpPort);

  setupOTA();


//////     Setting up LEDs
//////
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);  // Изменили на GRB
  FastLED.clear();
  Serial.println("Strip started.");

  for(int i = 0; i < 3; i++) {
    zoneColors[i].currentColor = CRGB::Black;
    zoneColors[i].targetColor = CRGB::Black;
    zoneColors[i].brightness = 0;
  }

  singleZoneColor.currentColor = CRGB::Black;
  singleZoneColor.targetColor = CRGB::Black;
  singleZoneColor.brightness = 0;
}


////////%%%%%%%%      LOOP      ////////%%%%%%%%
////////%%%%%%%%                ////////%%%%%%%%
void loop(){
  ArduinoOTA.handle(); // Обработка OTA запросов
  webSocket.loop();
  handleUdpData();


  //if ( WiFi.status() == WL_DISCONNECTED)

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
      // По умолчанию ничего не делаем или устанавливаем режим по умолчанию
      break;
  }
  FastLED.show();

  // delay(1);
}
// Обработчик WebSocket сообщений
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      // Получаем текстовую команду
      handleTextCommand((char*)payload);
      break;
    case WStype_BIN:
      // Обрабатываем бинарные аудиоданные
      break;
  }
}

void handleUdpData() {
  int packetSize = udp.parsePacket();  // Проверяем, есть ли входящий пакет
  if (packetSize) {
    // Чтение данных из UDP пакета
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Завершаем строку

      // Пример полученного сообщения: "low:120, mid:180, high:250"
      String receivedMessage = String(incomingPacket);

      // Парсим строку и извлекаем значения
      int lowIndex = receivedMessage.indexOf("low:") + 4;
      int midIndex = receivedMessage.indexOf("mid:") + 4;
      int highIndex = receivedMessage.indexOf("high:") + 5;

      // Извлекаем и конвертируем значения в целые числа
      lowFreqAmp = receivedMessage.substring(lowIndex, receivedMessage.indexOf(",", lowIndex)).toInt();
      midFreqAmp = receivedMessage.substring(midIndex, receivedMessage.indexOf(",", midIndex)).toInt();
      highFreqAmp = receivedMessage.substring(highIndex).toInt();

      // Выводим значения для отладки
      // Serial.printf("Received audio data - Low: %d, Mid: %d, High: %d\n", lowFreqAmp, midFreqAmp, highFreqAmp);
    }
  }
}

// Обработчик текстовых команд
void handleTextCommand(const char* message) {
  String cmd = String(message);
  // int name = getValueFromJSON(cmd, "name");
  // Serial.println(cmd);


  // Парсим JSON команду
  if (cmd.indexOf("changeMode") != -1) 
  {
    int mode = getValueFromJSON(cmd, "mode");
    FastLED.clear();
    if (mode == 0) 
      Serial.println("Static Color");
    if (cmd.indexOf("0") != -1) 
      Serial.println("Static Color index");

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
      currentMode =  RAINBOW_LINE_eff;
    } else if (cmd.indexOf("7") != -1) {
      currentMode =  CENTER_LINE_eff;
    } else if (cmd.indexOf("9") != -1) {
      currentMode =  FIVE_ZONE_freq;
    } 

  } else if (cmd.indexOf("parameterChange") != -1)
  {
    int name = getValueFromJSON(cmd, "name");
    switch(name)
    {
      case 0:
        sensitivity = getFloatFromJSON(cmd,"value");
        // Serial.println("Sensitivity " + String(sensitivity) );
        break;
      case 1: // 1 "transitionSpeed"
        transitionSpeed = getFloatFromJSON(cmd,"value");
        //Serial.println("transitionSpeed " + String(transitionSpeed) );
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
        FastLED.setBrightness(brightness);
        // FastLED.show();
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
        break;

    }
  }
  // Обработка команды изменения цвета (RGB)
  else if (cmd.indexOf("changeColor") != -1) {
    int r = getValueFromJSON(cmd, "r");
    int g = getValueFromJSON(cmd, "g");
    int b = getValueFromJSON(cmd, "b");
    staticColor = CRGB(r, g, b);
    // showStaticColor(staticColor);
  }
  // processAudioData(cmd);
}

// Функция для получения значения из JSON
float getFloatFromJSON(String json, String key) {
  // Serial.println(json);
  int startIndex = json.indexOf("\"" + key + "\":") + key.length() + 3;
  int endIndex = json.indexOf(",", startIndex);
  if (endIndex == -1) {
    endIndex = json.indexOf("}", startIndex);
  }

  // Serial.println(startIndex);
  // Serial.println(endIndex);

  return json.substring(startIndex, endIndex).toFloat();
}

int getValueFromJSON(String json, String key) {
  // Serial.println(json);
  int startIndex = json.indexOf("\"" + key + "\":") + key.length() + 3;
  int endIndex = json.indexOf(",", startIndex);
  if (endIndex == -1) {
    endIndex = json.indexOf("}", startIndex);
  }

  // Serial.println(startIndex);
  // Serial.println(endIndex);

  return json.substring(startIndex, endIndex).toInt();
}
String getStringFromJSON(String json, String key) {
  // Serial.println(json);
  int startIndex = json.indexOf("\"" + key + "\":") + key.length() + 3;
  int endIndex = json.indexOf(",", startIndex);
  if (endIndex == -1) {
    endIndex = json.indexOf("}", startIndex);
  }

  // Serial.println(startIndex);
  // Serial.println(endIndex);

  return String(json.substring(startIndex, endIndex));
}



// Пример обработки аудиоданных для светомузыки
void processAudioData(String data) 
{
  lowFreqAmp = getValueFromJSON(data, "LFA") * sensitivityLOW;
  midFreqAmp = getValueFromJSON(data, "MFA") * sensitivityMID;
  highFreqAmp = getValueFromJSON(data, "HFA") * sensitivityHIGH;
  // Serial.println("Current mode " + String(currentMode));
}


//////////////////    Sub_Functions    //////////////////
//////////////////                     //////////////////

////---  Mode_Functions  ---////
////---                  ---////
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

////---  Changing_Mode_Functions  ---////
////---                           ---////
void switchMode() {
  currentMode = static_cast<LedMode>((currentMode + 1) % TOTAL_MODES);
}

void setMode(LedMode  mode) {

  if(mode >= RAINBOW_wave && mode < TOTAL_MODES) {
    currentMode = mode;
  }
}


//////////////////    DRAWING MODES    //////////////////
//////////////////                     //////////////////


////---  Статичный режим  ---////
////---                   ---////
void showStaticColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

////---  Режим 1: Радуга с волнообразной формой  ---////
////---                                          ---////
// void rainbowWave() {
//   static float position = 0;
//   position += rainbowSpeed;
//   for (int i = 0; i < NUM_LEDS; i++) {
//     float wave = sin((i + position ) / waveWidth);
//     wave = map(wave, -1, 1, 0, 0.7) + 0.3;
//     uint8_t hue = (i * 255 / NUM_LEDS) + (position * 10) + 0.2; // Увеличение изменения оттенка
//     CRGB target = CHSV(hue, 255, 255 * (wave * 0.5 + 0.5));
//     leds[i] = blendColors(leds[i], target, transitionSpeed);
//   }
// }

void rainbowWave(int width, int startOffset) {
  static float position = 0;
  position += rainbowSpeed;

  // Вычисляем начало и конец области действия эффекта
  int start = (NUM_LEDS - width - startOffset) / 2 + startOffset;
  int end = start + width;

  for (int i = startOffset; i < NUM_LEDS; i++) {
    if (i >= start && i < end) {
      // Применяем радугу только в заданной области
      float wave = sin((i + position) / waveWidth);
      wave = map(wave, -1, 1, 0, 0.7) + 0.3;
      uint8_t hue = (i * 255 / NUM_LEDS) + (position * 10) + 0.2;
      CRGB target = CHSV(hue, 255, 255 * (wave * 0.5 + 0.5));
      leds[i] = blendColors(leds[i], target, transitionSpeed);
    } else {
      // Гасим светодиоды за пределами области
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}
// void rainbowWave(int rainbowWidth, int startOffset) {
//   static float position = 0;  // Для переливающегося эффекта
//   position += rainbowSpeed;   // Движение радуги

//   // Начало и конец области действия радуги
//   int start = startOffset;                  // Начало радуги
//   int end = min(startOffset + rainbowWidth, NUM_LEDS);  // Конец радуги (учитываем длину)

//   // Центр области радуги
//   int center = start + (end - start) / 2;

//   // Очистка светодиодов
//   FastLED.clear();

//   for (int i = 0; i < NUM_LEDS; i++) {
//     if (i >= start && i < end) {
//       // Цвет внутри области радуги
//       uint8_t hue = (i * 255 / rainbowWidth) + position * 10;  // Цвет зависит от позиции
//       leds[i] = CHSV(hue % 255, 255, 255);
//     } else {
//       // Гасим светодиоды за пределами области
//       leds[i] = CRGB::Black;
//     }
//   }

//   FastLED.show();
// }


////---  Режим 2: Плавно мерцающий цвет  ---////
////---                                  ---////
void flickerColor() {
  static float brightness = 0;
  brightness += flickerSpeed * 0.05;

  // Расчёт текущей яркости с учётом минимального значения
  float flicker = (sin(brightness) * 0.5 + 0.5);  // Диапазон [0, 1]
  flicker = flicker * (1.0 - minBrightness) + minBrightness;  // Диапазон [minBrightness, 1.0]

  // Применяем яркость к целевому цвету
  CRGB target = staticColor;
  target.nscale8(255 * flicker);

  // Заполняем ленту
  ledsFillSmooth(leds, NUM_LEDS, target, transitionSpeed);
}


////---  Режим 3: Стробоскоп  ---////
////---                       ---////

void strobeMode() {
  static unsigned long lastToggle = 0;  // Время последнего переключения
  static bool isOn = false;             // Состояние стробоскопа (включен/выключен)
  unsigned long interval = 1000 / (strobeFrequency * 2);  // Интервал переключения

  if (millis() - lastToggle > interval) {
    lastToggle = millis();  // Обновляем время переключения
    isOn = !isOn;           // Переключаем состояние
  }

  // Резкое переключение цветов
  strobeEffect(leds, NUM_LEDS, CRGB::White, isOn);
}

void strobeEffect(CRGB* ledsArray, int numLeds, CRGB color, bool isOn) {
  if (isOn) {
    for (int i = 0; i < numLeds; i++) {
      ledsArray[i] = color; // Устанавливаем заданный цвет
    }
  } else {
    for (int i = 0; i < numLeds; i++) {
      ledsArray[i] = CRGB::Black; // Гасим светодиоды
    }
  }
  FastLED.show();
}

////---  Режим 5: Одна зона, меняющая цвет в зависимости от частоты с плавностью и порогом  ---////
////---                                                                                     ---////
// void singleZoneFrequencyVisualization() {
//   int maxAmp = max(max(lowFreqAmp, midFreqAmp), highFreqAmp);
//   CRGB target = CRGB::Black;

//   if(maxAmp * sensitivity > amplitudeThreshold) {
//     if (maxAmp == lowFreqAmp) {
//       target = lowColor;
//     } else if (maxAmp == midFreqAmp) {
//       target = midColor;
//     } else {
//       target = highColor;
//     }
//     // singleZoneColor.brightness = constrain(maxAmp * sensitivity, 0, 255);
//     singleZoneColor.brightness = brightness;

//   } else {
//     singleZoneColor.brightness = max((int)(singleZoneColor.brightness * pulseDecay), 0);
//   }

//   // Плавный переход цвета
//   // singleZoneColor.currentColor = blendColors(singleZoneColor.currentColor, target, transitionSpeed);

//   // Применение цвета ко всей ленте
//   CRGB color = singleZoneColor.currentColor;
//   color.nscale8(singleZoneColor.brightness);
//   ledsFillSmooth(leds, NUM_LEDS, color, transitionSpeed);
//   for (int i = 0; i < NUM_LEDS; i++){
//     leds[i] = target;
//   }
// }
// void singleZoneFrequencyVisualization() {
//   static unsigned long lastImpulseTime = 0;  // Время последнего импульса
//   static CRGB lastColor = CRGB::Black;       // Последний цвет для плавного затухания

//   int maxAmp = max(max(lowFreqAmp, midFreqAmp), highFreqAmp);  // Максимальная амплитуда среди всех частот
//   CRGB targetColor = CRGB::Black;

//   // Выбор цвета на основе максимальной амплитуды
//   if (maxAmp == lowFreqAmp) {
//     targetColor = lowColor;
//   } else if (maxAmp == midFreqAmp) {
//     targetColor = midColor;
//   } else if (maxAmp == highFreqAmp) {
//     targetColor = highColor;
//   }

//   // Проверяем порог и интервал между импульсами
//   if (maxAmp * sensitivity > amplitudeThreshold &&
//       millis() - lastImpulseTime > minPulseInterval) {
//     // Новый импульс: устанавливаем цвет
//     FastLED.clear();  // Полностью очищаем ленту
//     fill_solid(leds, NUM_LEDS, targetColor);
//     lastColor = targetColor;  // Запоминаем последний цвет
//     lastImpulseTime = millis();  // Обновляем время последнего импульса
//   } else {
//     // Постепенное затухание предыдущего цвета
//     for (int i = 0; i < NUM_LEDS; i++) {
//       leds[i] = lastColor;  // Сохраняем цвет последнего импульса
//       // fadeWithMinBrightness(leds[i], minBrightnessZones, pulseDecay);
//       // for (int i = 0; i < end; i++) {
//         leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));
      
//     }
//   }

//   FastLED.show();  // Отображаем изменения
// }

void singleZoneFrequencyVisualization() {
  static unsigned long lastImpulseTime = 0;  // Время последнего импульса
  static CRGB lastColor = CRGB::Black;       // Последний цвет для плавного затухания

  int maxAmp = max(max(lowFreqAmp, midFreqAmp), highFreqAmp);  // Максимальная амплитуда среди всех частот
  CRGB targetColor = CRGB::Black;

  // Выбор цвета на основе максимальной амплитуды
  if (maxAmp == lowFreqAmp) {
    targetColor = lowColor;
  } else if (maxAmp == midFreqAmp) {
    targetColor = midColor;
  } else if (maxAmp == highFreqAmp) {
    targetColor = highColor;
  }

  // Проверяем порог и интервал между импульсами
  if (maxAmp * sensitivity > amplitudeThreshold &&
      millis() - lastImpulseTime > minPulseInterval) {
    // Новый импульс: устанавливаем цвет
    FastLED.clear();  // Полностью очищаем ленту
    fill_solid(leds, NUM_LEDS, targetColor);
    lastColor = targetColor;  // Запоминаем последний цвет
    lastImpulseTime = millis();  // Обновляем время последнего импульса
  } else {
    // Постепенное затухание предыдущего цвета
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));
    }
  }

  FastLED.show();  // Отображаем изменения
}


////---  Режим 4: Лента разделена на три зоны по частотам с плавностью и порогом  ---////
////---                                                                           ---////
// void threeZoneFrequencyVisualization() {
//   int zoneSize = (NUM_LEDS - LED_START) / 3;
//   for(int i = 0; i < 3; i++) {
//     int amp;
//     CRGB color;
//     if(i == 0) {
//       amp = lowFreqAmp;
//       color = lowColor;
//     } else if(i == 1) {
//       amp = midFreqAmp;
//       color = midColor;
//     } else {
//       amp = highFreqAmp;
//       color = highColor;
//     }
//     if(amp * sensitivity > amplitudeThreshold) {
//       zoneColors[i].targetColor = color;
//       // zoneColors[i].brightness = constrain(amp * sensitivity, 0, 255);
//       zoneColors[i].brightness = brightness;
//     } else {
//       // Плавное затухание
//       zoneColors[i].brightness = max((int)(zoneColors[i].brightness * pulseDecay), 0);
//       if(zoneColors[i].brightness == 0) {
//         zoneColors[i].targetColor = CRGB::Black;
//       }
//     }

//     // Плавный переход цвета
//     zoneColors[i].currentColor = blendColors(zoneColors[i].currentColor, zoneColors[i].targetColor, transitionSpeed);
//   }

//   // Применение цветов к зонам
//   FastLED.clear();
//   for(int i = LED_START; i < NUM_LEDS; i++) {
//     int zone = (i - LED_START) / ((NUM_LEDS - LED_START) / 3);
//     if(zone > 2) zone = 2;
//     CRGB color = zoneColors[zone].currentColor;
//     color.nscale8(zoneColors[zone].brightness);
//     leds[i] = blendColors(leds[i], color, transitionSpeed);
//     leds[i] = zoneColors[zone].currentColor;
//   }
// }

void threeZoneFrequencyVisualization() {
  static unsigned long lastImpulseTime[3] = {0, 0, 0};  // Время последнего импульса для каждой зоны
  static CRGB lastColors[3] = {CRGB::Black, CRGB::Black, CRGB::Black};  // Последние цвета для плавного затухания

  int zoneSize = (NUM_LEDS - LED_START) / 3;

  // Цвета и амплитуды для зон
  int amplitudes[3] = {lowFreqAmp, midFreqAmp, highFreqAmp};
  CRGB colors[3] = {lowColor, midColor, highColor};

  FastLED.clear();

  for (int zone = 0; zone < 3; zone++) {
    int start = LED_START + zone * zoneSize;
    int end = start + zoneSize;

    if (amplitudes[zone] * sensitivity > amplitudeThreshold &&
        millis() - lastImpulseTime[zone] > minPulseInterval) {
      // Новый импульс: устанавливаем цвет
      for (int i = start; i < end; i++) {
        leds[i] = colors[zone];
      }
      lastColors[zone] = colors[zone];
      lastImpulseTime[zone] = millis();
    } else {
      // Плавное затухание
      for (int i = start; i < end; i++) {
        // leds[i] = lastColors[zone];  // Сохраняем цвет последнего импульса
        // fadeWithMinBrightness(leds[i], minBrightnessZones, pulseDecay);
              leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));

      }
    }

    
  }

  FastLED.show();
}

void fiveZoneFrequencyVisualization() {
  static unsigned long lastImpulseTime[5] = {0, 0, 0, 0, 0};  // Время последнего импульса для каждой зоны
  static CRGB lastColors[5] = {CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black};  // Последние цвета

  int zoneSize = (NUM_LEDS - LED_START) / 5;

  // Цвета и амплитуды для зон
  int amplitudes[5] = {lowFreqAmp, midFreqAmp, highFreqAmp, midFreqAmp, lowFreqAmp};
  CRGB colors[5] = {lowColor, midColor, highColor, midColor, lowColor};

  FastLED.clear();

  for (int zone = 0; zone < 5; zone++) {
    int start = LED_START + zone * zoneSize;
    int end = start + zoneSize;

    if (amplitudes[zone] * sensitivity > amplitudeThreshold &&
        millis() - lastImpulseTime[zone] > minPulseInterval) {
      // Новый импульс: устанавливаем цвет
      for (int i = start; i < end; i++) {
        leds[i] = colors[zone];
      }
      lastColors[zone] = colors[zone];
      lastImpulseTime[zone] = millis();
    } else {
      // Плавное затухание
      for (int i = start; i < end; i++) {
        // leds[i] = lastColors[zone];  // Сохраняем цвет последнего импульса
        // fadeWithMinBrightness(leds[i], minBrightnessZones, pulseDecay);
              leds[i].fadeToBlackBy(255 - (int)(pulseDecay * 255));

      }
    }
  }

  FastLED.show();
}

// void fadeWithMinBrightness(CRGB& led, uint8_t minBrightness, float pulseDecay) {
//   // Получаем текущую яркость каждого канала
//   uint8_t currentR = led.r;
//   uint8_t currentG = led.g;
//   uint8_t currentB = led.b;

//   // Рассчитываем новую яркость с учётом pulseDecay
//   uint8_t newR = max((int)(currentR * pulseDecay), minBrightnessZones);
//   uint8_t newG = max((int)(currentG * pulseDecay), minBrightnessZones);
//   uint8_t newB = max((int)(currentB * pulseDecay), minBrightnessZones);

//   // Применяем рассчитанную яркость
//   led.setRGB(newR, newG, newB);
// }

////---  Режим 6: Капля из центра при появлении частоты с плавностью и порогом  ---////
////---                                                                         ---////
void centerDropEffect() {
  static unsigned long lastTrigger = 0;
  unsigned long currentTime = millis();
  
  // Определяем, если превышен порог для любой частоты
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
  
  // Затухание
  float fadeFactor = pulseDecay;
  
  // Создание капли
  int center = NUM_LEDS / 2;
  for(int i = 0; i < NUM_LEDS; i++) {
    int distance = abs(i - center);
    float brightness = 255.0 / (distance + 1); // Простая модель затухания
    CRGB target = dropColor;
    target.nscale8((uint8_t)(brightness * fadeFactor));
    leds[i] = blendColors(leds[i], target, transitionSpeed);
  }
}

////---  Режим 7: Линия из центра, длина зависит от амплитуды с плавностью  ---////
////---                                                                     ---////
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
////---  Режим 8: Линия из центра с переливом радуги и плавностью  ---////
////---                                                            ---////
// void rainbowLineEffect() {
//   int totalAmp = (lowFreqAmp + midFreqAmp + highFreqAmp) * sensitivity;
//   int lineLength = map(totalAmp, 0, 255 * 3, 0, (NUM_LEDS - LED_START) / 2);
  
//   for(int i = LED_START; i < NUM_LEDS; i++) {
//     leds[i].fadeToBlackBy(255 - (int)(transitionSpeed * 255));
//   }
  
//   for(int i = 0; i < lineLength; i++) {
//     int left = ((NUM_LEDS + LED_START) / 2) - i;
//     int right = ((NUM_LEDS + LED_START) / 2) + i;
//     uint8_t hue = (i * 255) / ((NUM_LEDS - LED_START) / 2);
//     CRGB rainbowColor = CHSV(hue, 255, 255);
//     if(left >= LED_START) {
//       leds[left] = blendColors(leds[left], rainbowColor, transitionSpeed);
//     }
//     if(right < NUM_LEDS) {
//       leds[right] = blendColors(leds[right], rainbowColor, transitionSpeed);
//     }
//   }
// }
void rainbowLineEffect() {
  // Рассчитываем длину активного участка
  int activeLength = NUM_LEDS - LED_START;

  // Рассчитываем ширину радуги на основе амплитуды
  int totalAmp = (lowFreqAmp + midFreqAmp + highFreqAmp) * sensitivity;
  int rainbowWidth = map(totalAmp, 0, 255 * 3, 0, activeLength);

  // Вызываем радугу с заданным началом и длиной
  rainbowWave(rainbowWidth, LED_START);
}





