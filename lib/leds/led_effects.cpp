#include "led_effects.h"


// --- Параметры эффекта ---
// #define MAX_DROPS 15
// #define DROP_EXPANSION_SPEED 0.1 // Скорость движения капли (пикселей в мс)
// #define MIN_TRIGGER_INTERVAL 300 // Мин. интервал создания капель (мс)
// #define CENTER_ZONE_SIZE 20 // Размер центральной зоны
// #define DROP_WIDTH 15    // ДЛИНА капли/хвоста (в светодиодах) - можно настроить

// --- Стиль затухания хвоста ---
// 0: Линейное затухание (просто)
// 1: Квадратичное затухание (хвост затухает быстрее)
// 2: Корень квадратный (хвост затухает медленнее)
// #define FADE_STYLE 1
#define MAX_DROPS_CONST 30           // Максимальное количество капель



struct Drop {
    bool active = false;        // Активна ли капля?
    CRGB color;                 // Цвет капли
    unsigned long startTime;    // Время создания (millis())
    float initialBrightness;    // Начальная яркость (может зависеть от амплитуды)
};

Drop drops[MAX_DROPS_CONST];         // Массив для хранения капель
unsigned long lastDropTriggerTime = 0; // Время последнего срабатывания триггера для любой частоты


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

// // Gemini 2.5
// void centerDropEffect() {
//     unsigned long currentTime = millis();

//     // 1. Общее затухание всех светодиодов
//     fadeToBlackBy(leds, NUM_LEDS, DROP_FADE_RATE);

//     // 2. Проверка триггеров и создание новых капель
//     int amplitudes[] = {lowFreqAmp, midFreqAmp, highFreqAmp};
//     CRGB colors[] = {lowColor, midColor, highColor};
//     bool triggered = false; // Флаг, что хотя бы один триггер сработал в этом цикле

//     // Проверяем каждую частоту
//     for (int i = 0; i < 3; ++i) {
//         if (amplitudes[i] * sensitivity > amplitudeThreshold) {
//             triggered = true; // Отмечаем срабатывание

//             // Ищем свободный слот для новой капли
//             int freeSlot = -1;
//             for (int j = 0; j < MAX_DROPS; ++j) {
//                 if (!drops[j].active) {
//                     freeSlot = j;
//                     break;
//                 }
//             }

//             // Если нашли свободный слот и прошел интервал
//             if (freeSlot != -1 && (currentTime - lastDropTriggerTime > MIN_TRIGGER_INTERVAL)) {
//                 drops[freeSlot].active = true;
//                 drops[freeSlot].color = colors[i];
//                 drops[freeSlot].startTime = currentTime;
//                 // Можно сделать начальную яркость зависимой от амплитуды:
//                 drops[freeSlot].initialBrightness = map(amplitudes[i] * sensitivity, amplitudeThreshold, 255, 150, 255); // Примерная карта
//                 drops[freeSlot].initialBrightness = constrain(drops[freeSlot].initialBrightness, 150, 255); // Ограничим мин/макс
//                 lastDropTriggerTime = currentTime; // Обновляем время последнего *успешного* создания капли
//                 break; // Создаем только одну каплю за цикл от одного типа частоты (можно убрать, если нужно несколько сразу)
//             }
//         }
//     }


//     // 3. Обновление и отрисовка активных капель
//     int center = NUM_LEDS / 2;
//     for (int i = 0; i < MAX_DROPS; ++i) {
//         if (drops[i].active) {
//             unsigned long age = currentTime - drops[i].startTime;

//             // Деактивировать старые капли
//             if (age > DROP_LIFETIME) {
//                 drops[i].active = false;
//                 continue; // Переходим к следующей капле
//             }

//             // Расчет текущего радиуса "полусферы"
//             float currentRadius = age * DROP_EXPANSION_SPEED;

//             // Расчет текущей максимальной яркости капли (затухание со временем)
//             // Плавное затухание от initialBrightness до 0 за DROP_LIFETIME
//             uint8_t currentMaxBrightness = map(age, 0, DROP_LIFETIME, drops[i].initialBrightness, 0);

//             // Отрисовка полусферы
//             for (int j = 0; j <= ceil(currentRadius); ++j) { // Идем от центра до текущего радиуса
//                 int distance = j;
//                 int leftPos = center - distance;
//                 int rightPos = center + distance;

//                 // Рассчитываем яркость для этой дистанции (макс в центре, падает к радиусу)
//                 // Используем квадратичное падение для более плавного вида "сферы"
//                 // float brightnessFactor = pow(1.0 - (float)distance / currentRadius, 2);
//                 // Или линейное падение:
//                 float brightnessFactor = 1.0;
//                 if (currentRadius > 0) { // Избегаем деления на ноль
//                     brightnessFactor = 1.0 - constrain((float)distance / currentRadius, 0.0, 1.0);
//                 }


//                 uint8_t pixelBrightness = currentMaxBrightness * brightnessFactor;

//                 // Применяем цвет с расчитанной яркостью к левому и правому пикселю
//                 if (pixelBrightness > 0) { // Оптимизация - не считать для черных пикселей
//                     CRGB pixelColor = drops[i].color;
//                     pixelColor.nscale8(pixelBrightness);

//                     // Левый пиксель (если в пределах ленты)
//                     if (leftPos >= 0 && leftPos < NUM_LEDS) {
//                         // Используем addtive blending или max blending
//                         // Additive: leds[leftPos] += pixelColor; (может привести к белому цвету)
//                         // Max: (сохраняет цвет, берет самый яркий компонент)
//                         leds[leftPos].r = max(leds[leftPos].r, pixelColor.r);
//                         leds[leftPos].g = max(leds[leftPos].g, pixelColor.g);
//                         leds[leftPos].b = max(leds[leftPos].b, pixelColor.b);
//                     }
//                     // Правый пиксель (если в пределах ленты и не центральный)
//                     if (rightPos < NUM_LEDS && rightPos != leftPos) {
//                         leds[rightPos].r = max(leds[rightPos].r, pixelColor.r);
//                         leds[rightPos].g = max(leds[rightPos].g, pixelColor.g);
//                         leds[rightPos].b = max(leds[rightPos].b, pixelColor.b);
//                     }
//                 }
//                  // Если яркость упала почти до 0 на этой дистанции, дальше можно не считать
//                 if (pixelBrightness < 5 && distance > 0) break;
//             }
//         }
//     }

//     // 4. Показать результат (если твоя структура предполагает вызов show() внутри эффекта)
//     FastLED.show();
// }

// Gemini 2.5
void centerDropEffect() {
    unsigned long currentTime = millis();

    // Очистка ленты перед отрисовкой
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // 2. Проверка триггеров и создание новых капель (код из твоей версии)
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

    // 3. Обновление и отрисовка активных капель
    int effectCenter = (NUM_LEDS + LED_START) / 2;
    int zoneHalfSize = CENTER_ZONE_SIZE / 2;
    int zoneStart = max(LED_START, effectCenter - zoneHalfSize);
    int zoneEndCalc = effectCenter + zoneHalfSize - ((CENTER_ZONE_SIZE % 2 == 0) ? 1 : 0);
    int zoneEnd = min(NUM_LEDS - 1, zoneEndCalc);
    zoneStart = min(zoneStart, zoneEnd); // на всякий случай


    for (int i = 0; i < MAX_DROPS; ++i) {
        if (drops[i].active) {
            unsigned long age = currentTime - drops[i].startTime;
            int offset = round((float)age * DROP_EXPANSION_SPEED);

            // Определяем позиции НАЧАЛА и КОНЦА отрезка, где ДОЛЖНА быть капля
            // Левая капля: самый левый пиксель (острие) до самого правого (хвост)
            int left_leading_edge = zoneStart - 1 - offset - (DROP_WIDTH - 1); // Самый левый
            int left_trailing_edge = zoneStart - 1 - offset;                // Самый правый
            // Правая капля: самый левый пиксель (хвост) до самого правого (острие)
            int right_trailing_edge = zoneEnd + 1 + offset;                 // Самый левый
            int right_leading_edge = zoneEnd + 1 + offset + (DROP_WIDTH - 1); // Самый правый

             // --- УСЛОВИЕ ДЕАКТИВАЦИИ ---
             // Деактивируем, когда хвост (самый близкий к центру пиксель)
             // полностью ушел за край ленты.
            if ( (left_trailing_edge < 0) && (right_trailing_edge >= NUM_LEDS) ) {
                drops[i].active = false;
                continue; // Эта капля больше не рисуется
            }

            // --- Отрисовка ЛЕВОЙ капли С ГРАДИЕНТОМ ---
            for (int k = left_leading_edge; k <= left_trailing_edge; ++k) {
                // Проверяем, находится ли пиксель в допустимой зоне (ВНУТРИ ленты и ЛЕВЕЕ центральной зоны)
                if (k >= 0 && k < zoneStart) {
                    // Рассчитываем позицию пикселя k внутри капли (0=острие, DROP_WIDTH-1=хвост)
                    int pos_in_drop = k - left_leading_edge;
                    // Рассчитываем фактор яркости (0.0 у хвоста, 1.0 на острие)
                    float relativePos = (float)pos_in_drop / (DROP_WIDTH > 1 ? (DROP_WIDTH - 1) : 1); // от 0 до 1
                    float brightnessFactor;

                    // #if FADE_STYLE == 0 // Линейный
                    // brightnessFactor = 1.0 - relativePos;
                    // #elif FADE_STYLE == 1 // Квадратичный (быстрый спад)
                    // brightnessFactor = pow(1.0 - relativePos, 2);
                    // #elif FADE_STYLE == 2 // Корень (медленный спад)
                    // brightnessFactor = sqrt(1.0 - relativePos);
                    // #else // По умолчанию линейный
                    // brightnessFactor = 1.0 - relativePos;
                    // #endif

                    if(FADE_STYLE == 0) // Линейный
                        brightnessFactor = 1.0 - relativePos;
                    else if(FADE_STYLE == 1) // Квадратичный (быстрый спад)
                        brightnessFactor = pow(1.0 - relativePos, 2);
                    else if(FADE_STYLE == 2) // Корень (медленный спад)
                        brightnessFactor = sqrt(1.0 - relativePos);
                    else // По умолчанию линейный
                        brightnessFactor = 1.0 - relativePos;

                    brightnessFactor = constrain(brightnessFactor, 0.0, 1.0);
                    uint8_t pixelBrightness = drops[i].initialBrightness * brightnessFactor;

                    if (pixelBrightness > 3) { // Рисуем только если яркость заметна
                        CRGB finalPixelColor = drops[i].color;
                        finalPixelColor.nscale8(pixelBrightness);
                        leds[k] = finalPixelColor; // Присваиваем рассчитанный цвет
                    }
                }
            } // Конец отрисовки левой капли

            // --- Отрисовка ПРАВОЙ капли С ГРАДИЕНТОМ ---
            for (int k = right_trailing_edge; k <= right_leading_edge; ++k) {
                 // Проверяем, находится ли пиксель в допустимой зоне (ВНУТРИ ленты и ПРАВЕЕ центральной зоны)
                if (k < NUM_LEDS && k > zoneEnd) {
                    // Рассчитываем позицию пикселя k внутри капли (0=острие, DROP_WIDTH-1=хвост)
                    // Острие здесь right_leading_edge
                    int pos_in_drop = right_leading_edge - k;
                    // Рассчитываем фактор яркости (0.0 у хвоста, 1.0 на острие)
                    float relativePos = (float)pos_in_drop / (DROP_WIDTH > 1 ? (DROP_WIDTH - 1) : 1); // от 0 до 1
                    float brightnessFactor;

                    // #if FADE_STYLE == 0 // Линейный
                    // brightnessFactor = 1.0 - relativePos;
                    // #elif FADE_STYLE == 1 // Квадратичный
                    // brightnessFactor = pow(1.0 - relativePos, 2);
                    // #elif FADE_STYLE == 2 // Корень
                    // brightnessFactor = sqrt(1.0 - relativePos);
                    // #else // Линейный
                    // brightnessFactor = 1.0 - relativePos;
                    // #endif

                    if(FADE_STYLE == 0) // Линейный
                        brightnessFactor = 1.0 - relativePos;
                    else if(FADE_STYLE == 1) // Квадратичный (быстрый спад)
                        brightnessFactor = pow(1.0 - relativePos, 2);
                    else if(FADE_STYLE == 2) // Корень (медленный спад)
                        brightnessFactor = sqrt(1.0 - relativePos);
                    else // По умолчанию линейный
                        brightnessFactor = 1.0 - relativePos;

                    brightnessFactor = constrain(brightnessFactor, 0.0, 1.0);
                    uint8_t pixelBrightness = drops[i].initialBrightness * brightnessFactor;

                     if (pixelBrightness > 3) { // Рисуем только если яркость заметна
                        CRGB finalPixelColor = drops[i].color;
                        finalPixelColor.nscale8(pixelBrightness);
                        leds[k] = finalPixelColor; // Присваиваем рассчитанный цвет
                    }
                }
            } // Конец отрисовки правой капли

            // --- Зажигание центральной зоны ПРИ СОЗДАНИИ (код из твоей версии) ---
            if (age < 35) {
                 // Цвет с максимальной яркостью для вспышки
                CRGB flashColor = drops[i].color;
                flashColor.nscale8(drops[i].initialBrightness);
                for (int k = zoneStart; k < zoneEnd; ++k) {
                    if(k >= 0 && k < NUM_LEDS) {
                        leds[k] = flashColor;
                    }
                }
            }

        } // конец if (drops[i].active)
    } // конец цикла по каплям (i)

    // 4. Показать результат
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