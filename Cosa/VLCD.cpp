/**
 * @file Cosa/VLCD.cpp
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013, Mikael Patel
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Board.hh"
#include "Cosa/VLCD.hh"

void
VLCD::Slave::on_request(void* buf, size_t size)
{
  char c = (char) m_buf[0];
  if (c != 0) {
    m_lcd->putchar(c);
    for (uint8_t i = 1; i < size; i++)
      m_lcd->putchar(m_buf[i]);
    return;
  }
  if (size == 2) {
    uint8_t cmd = m_buf[1];
    switch (cmd) {
    case BACKLIGHT_OFF_CMD: m_lcd->backlight_off(); return;
    case BACKLIGHT_ON_CMD: m_lcd->backlight_on(); return;
    case DISPLAY_OFF_CMD: m_lcd->display_off(); return;
    case DISPLAY_ON_CMD: m_lcd->display_on(); return;
    }
  }
  else if (size == 3) {
    uint8_t x = m_buf[1];
    uint8_t y = m_buf[2];
    m_lcd->set_cursor(x, y);
  }
}

#if !defined(__ARDUINO_TINY__)

#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"

void 
VLCD::write(uint8_t cmd)
{
  uint8_t buf[2];
  buf[0] = Slave::COMMAND;
  buf[1] = cmd;
  if (!twi.begin()) return;
  uint8_t retry = 3; 
  do {
    int res = twi.write(ADDR, buf, sizeof(buf));
    if (res == sizeof(buf)) break;
  } while (--retry);
  twi.end();
}

bool 
VLCD::begin()
{
  SLEEP(1);
  display_clear();
  display_on();
  backlight_on();
  return (true);
}

bool 
VLCD::end()
{
  display_off();
  return (true);
}

void 
VLCD::backlight_off() 
{ 
  write(Slave::BACKLIGHT_OFF_CMD);
}

void 
VLCD::backlight_on() 
{ 
  write(Slave::BACKLIGHT_ON_CMD);
}

void 
VLCD::display_off() 
{ 
  write(Slave::DISPLAY_OFF_CMD);
}

void 
VLCD::display_on() 
{ 
  write(Slave::DISPLAY_ON_CMD);
}

void 
VLCD::display_clear() 
{
  putchar('\f');
}

void 
VLCD::set_cursor(uint8_t x, uint8_t y)
{
  uint8_t buf[3];
  buf[0] = 0;
  buf[1] = x;
  buf[2] = y;  
  if (!twi.begin()) return;
  twi.write(ADDR, buf, sizeof(buf));
  twi.end();
  m_x = x;
  m_y = y;
}

int 
VLCD::putchar(char c)
{
  if (!twi.begin()) return (-1);
  int n = twi.write(ADDR, &c, sizeof(c));
  twi.end();
  if (n != 1) return (-1);
  if (c >= ' ') return (c & 0xff);
  switch (c) {
  case '\b':
    if (m_x > 0) m_x -= 1;
    break;
  case '\f':
    m_x = 0;
    m_y = 0;
    break;
  case '\n':
    m_x = 0;
    m_y += 1;
    if (m_y > HEIGHT) m_y = 0; 
    break;
  case '\t':
    m_x += m_tab - (m_x % m_tab);
    m_y += (m_x >= WIDTH);
    if (m_x >= WIDTH) m_x = 0;
    break;
  }
  return (c & 0xff);
}

int 
VLCD::puts(char* s)
{
  if (!twi.begin()) return (-1);
  size_t len = strlen(s);
  int n = twi.write(ADDR, s, len);
  twi.end();
  if (n != len) return (-1);
  m_x += len;
  return (0);
}

int 
VLCD::puts_P(const char* s)
{
  uint8_t buf[BUF_MAX];
  size_t len = 0;
  char c;
  while (((c = pgm_read_byte(s++)) != 0) && len != BUF_MAX)
    buf[len++] = c;
  if (!twi.begin()) return (-1);
  int n = twi.write(ADDR, buf, len);
  twi.end();
  if (n != len) return (-1);
  m_x += len;
  return (0);
}

int 
VLCD::write(void* buf, size_t size)
{
  if (!twi.begin()) return (-1);
  int n = twi.write(ADDR, buf, size);
  twi.end();
  if (n < 0) return (-1);
  m_x += size;
  return (n);
}
#endif