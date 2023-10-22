/*
  Reusable RGB LED effects

  Each LED has unique coordinates. They're not aligned on a grid.
*/

class LEDState {
public:
  const uint8_t id;

  // Coordinates according to gimp:

  // left-to-right axis, unit = 0.1 mm
  const double x;

  // top-to-bottom axis, unit = 0.1 mm
  const double y;

  // Current color intensity within the range [0, 1]
  double red;
  double green;
  double blue;

  LEDState::LEDState(uint8_t id, uint16_t x, uint16_t y);
};

LEDState::LEDState(uint8_t id, uint16_t x, uint16_t y) {
  id = id;
  x = (double) x;
  y = (double) y;
}

/*
  Proposed effects:

  - constant: (rgb)
  - concentric waves: (center, speed, wavelet shape)
  - pulsating spot: (center, radius, frequency, duration, shape)
  - twinkling: (intensity distribution(mean, stdev),
                distribution of means,
                distribution of stdevs)

  To keep the computations affordable:
  - don't allow arbitrary composition of effects
  - define each effect over a time range after which it is removed from
    the active effects
  - allow only the simple superposition of light intensities,
    no other interactions
  - limit the number of simultaneous effects e.g. have 5 channels
    each supporting one effect; starting a new effect is requested on
    a specific channel and ignored if the channel is still in use
    by a previous effect. Channels can correspond to regions of space so
    as to avoid messy superpositions.

  Generic parameters for one effect:
  - subset of leds that are affected, specified either as a range or
    as a predefined set (but not one that's computed along the way)
  - duration (cut-off)
*/
