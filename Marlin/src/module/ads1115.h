// ads1115.h
#ifndef ADS1115_H
#define ADS1115_H

#include <Arduino.h>
#include <Wire.h>

/**
 * ADS1115 I2C 16-bit ADC Driver
 * Supports single-shot conversion on 4 channels with configurable gain and data rate.
 */
class ADS1115 {
public:
  enum Gain {
    GAIN_TWOTHIRDS = 0x0000,  // +/-6.144V
    GAIN_ONE       = 0x0200,  // +/-4.096V
    GAIN_TWO       = 0x0400,  // +/-2.048V
    GAIN_FOUR      = 0x0600,  // +/-1.024V
    GAIN_EIGHT     = 0x0800,  // +/-0.512V
    GAIN_SIXTEEN   = 0x0A00   // +/-0.256V
  };
  enum DataRate {
    DR_8SPS   = 0x0000,
    DR_16SPS  = 0x0020,
    DR_32SPS  = 0x0040,
    DR_64SPS  = 0x0060,
    DR_128SPS = 0x0080,  // default
    DR_250SPS = 0x00A0,
    DR_475SPS = 0x00C0,
    DR_860SPS = 0x00E0
  };

  ADS1115();
  /**
   * Initialize with TwoWire instance, I2C address (default 0x48), gain and data rate.
   */
  bool begin(TwoWire &wirePort = Wire, uint8_t address = 0x48,
             Gain gain = GAIN_TWOTHIRDS, DataRate rate = DR_128SPS);

  /**
   * Perform a single-shot conversion on the given channel (0-3).
   * Returns raw 16-bit signed value.
   */
  int16_t readRaw(uint8_t channel);

  /**
   * Read channel and convert to voltage in Volts (float).
   */
  float readVoltage(uint8_t channel);

private:
  TwoWire *_wire;
  uint8_t _address;
  Gain _gain;
  DataRate _rate;

  /**
   * Compose configuration register for channel, gain, rate, single-shot.
   */
  uint16_t configReg(uint8_t channel);
};

#endif // ADS1115_H