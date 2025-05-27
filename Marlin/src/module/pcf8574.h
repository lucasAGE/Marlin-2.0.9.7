// pcf8574.h
#ifndef PCF8574_H
#define PCF8574_H

#include <Arduino.h>
#include <Wire.h>

/**
 * PCF8574 I2C I/O Expander Driver
 * Provides basic reading and writing of 8-bit ports and individual pins.
 */
class PCF8574 {
public:
  PCF8574();
  /**
   * Initialize the expander with the given TwoWire instance and I2C address.
   * Returns true if initialization succeeds.
   */
  bool begin(TwoWire &wirePort = Wire, uint8_t address = 0x20);

  /**
   * Write a byte to the port, updating all pins.
   */
  void writePort(uint8_t value);

  /**
   * Read the current state of the port.
   */
  uint8_t readPort();

  /**
   * Write a single pin (0-7) to value (HIGH/LOW).
   */
  void writePin(uint8_t pin, bool value);

  /**
   * Read a single pin (0-7), returns HIGH (true) or LOW (false).
   */
  bool readPin(uint8_t pin);

  /**
   * Get the cached port state (last written or read value).
   */
  uint8_t getPortState() const;

private:
  TwoWire *_wire;
  uint8_t _address;
  uint8_t _portState;
};

#endif // PCF8574_H


