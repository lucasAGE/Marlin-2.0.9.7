#ifndef THERMISTOR_3000_H
#define THERMISTOR_3000_H

/**
 * ID 3000: Custom NTC 100 kΩ @25°C, Beta ≈3950 K, pull-up 4.7 kΩ
 * Divider em 5V, ADC interno 10-bit (0–1023)
 * Inserida via thermistors.h quando #define TEMP_SENSOR_* == 3000
 */

 #pragma once

 
constexpr temp_entry_t temptable_3000[] PROGMEM = {
  { OV(1023),  -40 },
  { OV(1019),  -25 },
  { OV(1007),    0 },
  { OV( 997),   25 },
  { OV( 981),   50 },
  { OV( 960),   75 },
  { OV( 933),  100 },
  { OV( 900),  125 },
  { OV( 859),  150 },
  { OV( 809),  175 },
  { OV( 748),  200 },
  { OV( 676),  225 },
  { OV( 589),  250 },
  { OV( 484),  275 },
  { OV( 352),  300 }
};

#define TT_3000  { OV(3000), temptable_3000, COUNT(temptable_3000), 0 }

#endif // THERMISTOR_3000_H
