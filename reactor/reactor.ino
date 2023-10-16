/*
 * Capacitive proximity sensors and individually addressable RGB LEDs
 */

#include <CapacitiveSensor.h>
#include <FastLED.h>

// This makes things really slow.
const bool debug = false;
                                               
// Sensing
const int SENSOR1_SEND_PIN = 2;
const int SENSOR1_RECV_PIN = 3;
CapacitiveSensor capsensor1 = CapacitiveSensor(SENSOR1_SEND_PIN, SENSOR1_RECV_PIN);

const int SENSOR2_SEND_PIN = 2;
const int SENSOR2_RECV_PIN = 4;
CapacitiveSensor capsensor2 = CapacitiveSensor(SENSOR2_SEND_PIN, SENSOR2_RECV_PIN);

const int SENSOR3_SEND_PIN = 2;
const int SENSOR3_RECV_PIN = 5;
CapacitiveSensor capsensor3 = CapacitiveSensor(SENSOR3_SEND_PIN, SENSOR3_RECV_PIN);

typedef struct SensorState {
  CapacitiveSensor sensor;
  float sample;
  bool initialized_median;
  float median;
  float median_step;
  float avg;

  // Proximity score from 0 to 1.
  // 1 = closest.
  float prox;

  // Difference between current and previous proximity score
  float dprox;
} SensorState;

void init_sensor_state(SensorState &x, CapacitiveSensor &sensor) {
  x.sensor = sensor;
  x.sample = 0;
  x.initialized_median = 0;
  x.median = 0;
  x.median_step = 0;
  x.avg = 0;
  x.prox = 0;
  x.dprox = 0;
  return;
}

SensorState sensor1 = { .sensor = capsensor1 };
SensorState sensor2 = { .sensor = capsensor2 };
SensorState sensor3 = { .sensor = capsensor3 };

// Duration of the last time step in ms
float t;
float dt;

// LED strip
const int LED_PIN = 13;
const int NUM_LEDS = 50;
CRGB leds[NUM_LEDS];

void setup()                    
{
  // turn off autocalibrate on channel 1 - just as an example
  //capsensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
  if (debug)
    Serial.begin(9600);

  init_sensor_state(sensor1, capsensor1);
  init_sensor_state(sensor2, capsensor2);
  init_sensor_state(sensor3, capsensor3);
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
}

/*
 * Moving averages and scoring used for proximity sensing
 */

long iterations = 0;


// Estimate the long-term median (moving percentile)
void update_median(SensorState &x) {
  if (!x.initialized_median) {
    if (iterations == 50) {
      x.median = x.avg;
      x.median_step = abs(x.median) / 100;
      x.initialized_median = true;
    }
    else
      x.median = x.avg;
  }
  else {
    if (x.sample > x.median) {
      x.median += x.median_step;
    } else if (x.sample < x.median) {
      x.median -= x.median_step;
    }
  }
}

// Estimate the short-term mean (exponential moving average)
void update_avg(SensorState &x) {
  x.avg = 0.2 * x.sample + 0.8 * x.avg;
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
float update_proximity_score(SensorState &x) {
  update_median(x);
  update_avg(x);
  float deviation = x.median > 0 ? max(0, x.avg-x.median)/max(0, x.median) : 0;
  float logdev = logdeviation(deviation);
  float new_prox = min(1, max(0, (logdev-min_logdev)/(max_logdev-min_logdev)));
  x.dprox = new_prox - x.prox;
  // Avoid big jumps
  if (x.dprox > 0.1) {
    x.prox += 0.1;
  }
  else if (x.dprox < -0.1) {
    x.prox -= 0.1;
  }
  else {
    x.prox = new_prox;
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
    Serial.println(x.prox);
  }
}

void sense() {
  sensor1.sample = sensor1.sensor.capacitiveSensor(30);
  sensor2.sample = sensor2.sensor.capacitiveSensor(30);
  sensor3.sample = sensor3.sensor.capacitiveSensor(30);
  update_proximity_score(sensor1);
  update_proximity_score(sensor2);
  update_proximity_score(sensor3);
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
      float all = (sensor1.prox + sensor2.prox + sensor3.prox) / 3.0;
      red = 0.01 + 0.5 * sensor1.prox + 0.1 * all;
      green = 0.01 + 0.5 * sensor2.prox + 0.1 * all;
      blue = 0.01 + 0.5 * sensor3.prox + 0.1 * all;
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
