// pcf8574.cpp
#include "pcf8574.h"

PCF8574::PCF8574()
  : _wire(nullptr), _address(0x20), _portState(0xFF) {}

bool PCF8574::begin(TwoWire &wirePort, uint8_t address) {
  _wire = &wirePort;
  _address = address;
  _wire->begin();
  // Read initial state to initialize cache
  _portState = readPort();
  return true;
}

void PCF8574::writePort(uint8_t value) {
  _portState = value;
  _wire->beginTransmission(_address);
  _wire->write(_portState);
  _wire->endTransmission();
}

uint8_t PCF8574::readPort() {
  uint8_t value = 0xFF;
  _wire->requestFrom(_address, (uint8_t)1);
  if (_wire->available()) {
    value = _wire->read();
    _portState = value;
  }
  return value;
}

void PCF8574::writePin(uint8_t pin, bool value) {
  if (pin > 7) return;
  uint8_t mask = (1 << pin);
  if (value) _portState |= mask;
  else        _portState &= ~mask;
  writePort(_portState);
}

bool PCF8574::readPin(uint8_t pin) {
  if (pin > 7) return false;
  uint8_t port = readPort();
  return (port & (1 << pin)) != 0;
}

uint8_t PCF8574::getPortState() const {
  return _portState;
}
