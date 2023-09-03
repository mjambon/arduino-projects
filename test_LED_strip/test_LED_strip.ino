// https://github.com/FastLED/FastLED
#include <FastLED.h>

#define LED_PIN     7
#define NUM_LEDS    50

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

// time in milliseconds
float t = 0;
const float dt = 50;
const int period = 2000;
const float pi = acos(-1);
const float omega = 2 * pi / period;

// [0.0, 1.0] -> [0, 255]
int color(float x) {
  return max(0, min(255, trunc(256.0 * x)));
}

void loop() {
  for (int i = 0; i < NUM_LEDS; i++) {
    float led_phase = i * (2 * pi / NUM_LEDS);
    float red = 0.5 + sin(omega * t + led_phase)/2;
    float green = 0;
    float blue = 0.5 + sin(omega * t + pi + led_phase)/2;
    leds[i] = CRGB(color(red), color(green), color(blue));
  }
  FastLED.show();
  
  delay(dt);
  t = t + dt;
}
