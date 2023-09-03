#include <CapacitiveSensor.h>

/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10 megohm between send pin and receive pin
 * Resistor affects sensitivity, experiment with values, 50 kilohm - 50 megohm. 
 * Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 * Best results are obtained if sensor foil and wire is covered with an 
 * insulator such as paper or plastic sheet
 */

CapacitiveSensor capsensor = CapacitiveSensor(4,2);

const int LED_PIN = 9;
const bool debug = false;
    
void setup()                    
{
   // turn off autocalibrate on channel 1 - just as an example
   capsensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
   if (debug)
     Serial.begin(9600);

   pinMode(LED_PIN, OUTPUT);
}

long iterations = 0;
bool initialized_median = false;
float median = 0;
float median_step = 0;
float avg = 0;

// Estimate the long-term median (moving percentile)
void update_median(float sample) {
  if (!initialized_median) {
    if (iterations == 50) {
      median = avg;
      median_step = abs(median) / 100;
      initialized_median = true;
    }
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
void update_avg(float sample) {
  avg = 0.2 * sample + 0.8 * avg;
}

void loop()                    
{
    long start = millis();
    long sample = capsensor.capacitiveSensor(100);
    update_median(sample);
    update_avg(sample);
    float deviation = median > 0 ? max(0, avg-median)/max(0, median) : 0;
    // proximity score from 0 to 5:
    float score = max(0, min(5, log(deviation * 5)/log(2)));

    analogWrite(LED_PIN, score * 255 / 5);

    if (debug) {
      // check on performance in milliseconds
     Serial.print("step duration (ms): ");
      Serial.print(millis() - start);        
      Serial.print("\tsample: ");
      Serial.print(sample);
      Serial.print("\tmedian: ");
      Serial.print(median);
      Serial.print("\taverage: ");
      Serial.print(avg);
      Serial.print("\tdeviation: ");
      Serial.print(deviation);
      Serial.print("\tscore: ");
      Serial.println(score);

      // arbitrary delay to limit data to serial port 
      delay(10);
    }
    iterations += 1;
}
