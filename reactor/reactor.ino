/*
 * Capacitive proximity sensors and individually addressable RGB LEDs
 */

#include <CapacitiveSensor.h>
#include <FastLED.h>

// This makes things really slow.
const bool debug = false;

// Sensing
const uint8_t SENSOR1_SEND_PIN = 2;
const uint8_t SENSOR1_RECV_PIN = 3;

const uint8_t SENSOR2_SEND_PIN = 2;
const uint8_t SENSOR2_RECV_PIN = 4;

const uint8_t SENSOR3_SEND_PIN = 2;
const uint8_t SENSOR3_RECV_PIN = 5;

class SensorState {
public:
  // Proximity score from 0 to 1.
  // 1 = closest.
  float prox;

  // Difference between current and previous proximity score
  float dprox;

  SensorState::SensorState(uint8_t send_pin, uint8_t recv_pin);
  void SensorState::sense();

private:
  CapacitiveSensor &sensor;
  float sample;
  bool initialized_median;
  float median;
  float median_step;
  float avg;

  void SensorState::update_proximity_score();
  void SensorState::update_median();
  void SensorState::update_avg();
};

// Initialization
SensorState::SensorState(uint8_t send_pin, uint8_t recv_pin) {
  sensor = CapacitiveSensor::CapacitiveSensor(send_pin, recv_pin);

  // Turn off auto-calibration otherwise occurring every 20 s
  sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);

  sample = 0;
  initialized_median = 0;
  median = 0;
  median_step = 0;
  avg = 0;
  prox = 0;
  dprox = 0;
}

void SensorState::sense() {
  sample = sensor.capacitiveSensor(50);
  update_proximity_score();
}

SensorState state1 = SensorState(SENSOR1_SEND_PIN, SENSOR1_RECV_PIN);
SensorState state2 = SensorState(SENSOR2_SEND_PIN, SENSOR2_RECV_PIN);
SensorState state3 = SensorState(SENSOR3_SEND_PIN, SENSOR3_RECV_PIN);

// Duration of the last time step in ms
float t;
float dt;

// LED strip
const uint8_t LED_PIN = 13;
const uint8_t NUM_LEDS = 50;
CRGB leds[NUM_LEDS];

void setup()
{
  if (debug)
    Serial.begin(9600);
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
}

/*
 * Moving averages and scoring used for proximity sensing
 */

long iterations = 0;

// Estimate the long-term median (moving percentile)
void SensorState::update_median() {
  if (!initialized_median) {
    if (iterations == 50) {
      median = avg;
      median_step = abs(median) / 100;
      initialized_median = true;
    }
    else
      median = avg;
  }
  else {
    if (sample > median) {
      median += median_step;
    } else if (sample < median) {
      median -= median_step;
    }
  }
}

// Estimate the short-term mean (exponential moving average)
void SensorState::update_avg() {
  avg = 0.2 * sample + 0.8 * avg;
}

// Values of the deviation mapping to a proximity score range of [0, 1]
const float min_dev = 0.1;
const float max_dev = 1;

float logdeviation(float deviation) {
  return log(max(deviation, 0)) / log(2);
}

const float min_logdev = logdeviation(min_dev);
const float max_logdev = logdeviation(max_dev);

// Set the prox and dprox globals.
void SensorState::update_proximity_score() {
  update_median();
  update_avg();
  float deviation = median > 0 ? max(0, avg-median)/max(0, median) : 0;
  float logdev = logdeviation(deviation);
  float new_prox = min(1, max(0, (logdev-min_logdev)/(max_logdev-min_logdev)));
  dprox = new_prox - prox;
  // Avoid big jumps
  if (dprox > 0.1) {
    prox += 0.1;
  }
  else if (dprox < -0.1) {
    prox -= 0.1;
  }
  else {
    prox = new_prox;
  }
  if (debug) {
    /*
    Serial.print("sample: ");
    Serial.print(sample);
    Serial.print("\tmedian: ");
    Serial.print(median);
    Serial.print("\taverage: ");
    Serial.print(avg);
    Serial.print("\tdeviation: ");
    Serial.print(deviation);
    Serial.print("\tscore: ");
    */
    Serial.println(prox);
  }
}

void sense() {
  state1.sense();
  state2.sense();
  state2.sense();
}

// [0.0, 1.0] -> [0, 255]
int color(float x) {
  return max(0, min(255, trunc(256.0 * x)));
}

void output() {
  float red, green, blue = 0;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    // Update only the next LED in the sequence?
    //if ((i + iterations) % NUM_LEDS < 20)
    //if (i < 20)
    {
      float all = (state1.prox + state2.prox + state3.prox) / 3.0;
      red = 0.5 * state1.prox;
      green = 0.5 * state2.prox;
      blue = 0.5 * state3.prox;
      leds[i] = CRGB(color(red), color(green), color(blue));
    }
  }
  FastLED.show();
}

void loop() {
  long t2 = millis();
  dt = t2 - t;
  t = t2;

  sense();
  output();
  iterations += 1;
}
