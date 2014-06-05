/**
 * @file CosaBlinkTemp.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * @section Description
 * Cosa LED blink demonstation. Blink RGB LED according to temperature
 * from 1-wire digital thermometer (DS18B20); RED if above 30 C, 
 * BLUE if below 25 C otherwise GREEN. Demonstrates powerdown to
 * less than 10 uA while idle (and no LED on).
 * 
 *                       DS18B20/sensor
 * (VCC)--[4K7]--+       +------------+
 * (GND)---------)-----1-|GND         |\
 * (D4)----------+-----2-|DQ          | |
 * (VCC/GND)-----------3-|VDD         |/
 *                       +------------+
 *
 * (D0)---------RED->|---+
 *                       |
 * (D1)-------GREEN->|---+----[330]---(GND)
 *                       |
 * (D2)--------BLUE->|---+
 *
 * Connect Arduino to DS18B20 in D4 and GND. May use parasite 
 * powering (connect DS18B20 VCC to GND) otherwise to VCC.
 * 
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Watchdog.hh"
#include "Cosa/OutputPin.hh"
#include "Cosa/OWI/Driver/DS18B20.hh"

// RGB LED pins
OutputPin red(Board::D0);
OutputPin green(Board::D1);
OutputPin blue(Board::D2);

// 1-wire bus and digital thermometer
OutputPin pw(Board::D3);
OWI owi(Board::D4);
DS18B20 sensor(&owi);

// Powerdown on delay
void powerdown(uint16_t ms)
{
  uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
  Power::all_disable();
  Watchdog::delay(ms);
  Power::all_enable();
  Power::set(mode);
}

void setup()
{
  // Start watchdog
  Watchdog::begin();

  // Connect to temperature sensor, and set resolution and triggers
  pw.on();
  sensor.connect(0);
  sensor.set_resolution(10);
  sensor.set_trigger(25,30);
  sensor.write_scratchpad();
  pw.off();
}

void loop()
{
  // Temperature conversion and read result
  pw.on();
  sensor.convert_request();
  sensor.read_scratchpad();
  pw.off();
  int8_t low, high;
  int16_t temp = (sensor.get_temperature() >> 4);
  sensor.get_trigger(low, high);

  // Set LED according to temperature
  if (temp < low) {
    blue.on();
    powerdown(32);
    blue.off();
    powerdown(1024);
  }
  else if (temp > high) {
    red.on();
    powerdown(32);
    red.off();
    powerdown(1000);
  }
  else {
    green.on();
    powerdown(32);
    green.off();
    powerdown(2000);
  }
}
