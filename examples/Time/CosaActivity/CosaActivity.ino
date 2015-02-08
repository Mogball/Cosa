/**
 * @file CosaActivity.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014-2015, Mikael Patel
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
 * Demonstrate of Cosa Activity handling.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Activity.hh"
#include "Cosa/Event.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"

class TraceActivity : public Activity {
public:
  TraceActivity(clock_t start, uint16_t period, uint16_t duration, uint16_t seconds);
  virtual void run();
};

TraceActivity::TraceActivity(clock_t start,
			     uint16_t period,
			     uint16_t duration,
			     uint16_t seconds) :
  Activity()
{
  set_time(start, duration, period);
  set_run_period(seconds);
}

void
TraceActivity::run()
{
  trace << time() << PSTR(":cycles=") << get_cycles() << endl;
}

// Start at 15. Every minute, for five seconds, run every second
TraceActivity activity(15, 1, 5, 1);
Alarm::Scheduler scheduler;

void setup()
{
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaActivity: started"));
  Watchdog::begin(16, Watchdog::push_timeout_events);
  RTC::begin();
  scheduler.begin();
  activity.enable();
}

void loop()
{
  Event event;
  Event::queue.await(&event);
  event.dispatch();
}
