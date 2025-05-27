// ads1115.cpp
#include "ads1115.h"

ADS1115::ADS1115()
  : _wire(nullptr), _address(0x48), _gain(GAIN_TWOTHIRDS), _rate(DR_128SPS) {}

bool ADS1115::begin(TwoWire &wirePort, uint8_t address, Gain gain, DataRate rate) {
  _wire = &wirePort;
  _address = address;
  _gain = gain;
  _rate = rate;
  _wire->begin();
  return true;
}

uint16_t ADS1115::configReg(uint8_t channel) {
  uint16_t config = 0x8000; // OS: start single conversion
  switch (channel) {
    case 0: config |= 0x4000; break;
    case 1: config |= 0x5000; break;
    case 2: config |= 0x6000; break;
    case 3: config |= 0x7000; break;
    default: config |= 0x4000; break;
  }
  config |= _gain;
  config |= 0x0100; // MODE: single-shot
  config |= _rate;
  config |= 0x0003; // Comparator disabled
  return config;
}

int16_t ADS1115::readRaw(uint8_t channel) {
  uint16_t cfg = configReg(channel);
  // Write config to pointer register 1
  _wire->beginTransmission(_address);
  _wire->write(0x01);
  _wire->write((cfg >> 8) & 0xFF);
  _wire->write(cfg & 0xFF);
  _wire->endTransmission();
  // Wait for conversion (approximate)
  delay(8);
  // Read conversion register 0
  _wire->beginTransmission(_address);
  _wire->write(0x00);
  _wire->endTransmission();
  _wire->requestFrom(_address, (uint8_t)2);
  if (_wire->available() < 2) return 0;
  uint16_t msb = _wire->read();
  uint16_t lsb = _wire->read();
  return (int16_t)((msb << 8) | lsb);
}

float ADS1115::readVoltage(uint8_t channel) {
  int16_t raw = readRaw(channel);
  // LSB size = 187.5 uV for default gain +/-6.144V
  const float lsb = 0.0001875;
  return raw * lsb;
}
