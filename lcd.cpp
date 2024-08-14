#include "lcd.h"

/*
 * commands' binary forms represent pin states, each command's value is in the binary form: RS R/W D7 D6 D5 D4 D3 D2 D1 D0
 * e.g. CLEAR_DISPLAY's value is 0x01 hex which is equivalent to 0000000001 in binary, this means only D0 pin's state is HIGH, rest is LOW
 */
#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define ENTRY_MODE_SET(to_right, display_shift) (0x04 | (to_right ? 0x02 : 0x00) | (display_shift ? 0x01 : 0x00))
#define DISPLAY_CONTROL(display_on, cursor_on, blink) (0x08 | (display_on ? 0x04 : 0x00) | (cursor_on ? 0x02 : 0x00) | (blink ? 0x01 : 0x00))
#define CURSOR_OR_DISPLAY_SHIFT(is_display, to_right) (0x10 | (is_display ? 0x08 : 0x00) | (to_right ? 0x04 : 0x00))
#define FUNCTION_SET(is_8bit, is_2line, is_5x8) (0x20 | (is_8bit ? 0x10 : 0x00) | (is_2line ? 0x08 : 0x00) | (is_5x8 ? 0x00 : 0x04))

#define CGRAM_ADDR_SET(addr) (0x40 | addr)
#define DDRAM_ADDR_SET(addr) (0x80 | addr)
#define CGRAM_OR_DDRAM_DATA_WRITE(data) (0x200 | data)

#define BUSYFLAG_ADDRCOUNTER_READ 0x100
#define CGRAM_OR_DDRAM_DATA_READ 0x300

LCD::LCD(int en, int rs, int rw, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7)
{
  en_pin = en;
  rs_pin = rs;
  rw_pin = rw;
  data_pins[0] = d0;
  data_pins[1] = d1;
  data_pins[2] = d2;
  data_pins[3] = d3;
  data_pins[4] = d4;
  data_pins[5] = d5;
  data_pins[6] = d6;
  data_pins[7] = d7;
  row_count = 2;
  col_count = 16;
  is_8_bit_mode = true;
  init();
}

LCD::LCD(int en, int rs, int rw, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7, int row, int col)
{
  en_pin = en;
  rs_pin = rs;
  rw_pin = rw;
  data_pins[0] = d0;
  data_pins[1] = d1;
  data_pins[2] = d2;
  data_pins[3] = d3;
  data_pins[4] = d4;
  data_pins[5] = d5;
  data_pins[6] = d6;
  data_pins[7] = d7;
  row_count = row;
  col_count = col;
  is_8_bit_mode = true;
  init();
}

LCD::LCD(int en, int rs, int rw, int d4, int d5, int d6, int d7)
{
  en_pin = en;
  rs_pin = rs;
  rw_pin = rw;
  data_pins[0] = -1;
  data_pins[1] = -1;
  data_pins[2] = -1;
  data_pins[3] = -1;
  data_pins[4] = d4;
  data_pins[5] = d5;
  data_pins[6] = d6;
  data_pins[7] = d7;
  row_count = 2;
  col_count = 16;
  is_8_bit_mode = false;
  init();
}

LCD::LCD(int en, int rs, int rw, int d4, int d5, int d6, int d7, int row, int col)
{
  en_pin = en;
  rs_pin = rs;
  rw_pin = rw;
  data_pins[0] = -1;
  data_pins[1] = -1;
  data_pins[2] = -1;
  data_pins[3] = -1;
  data_pins[4] = d4;
  data_pins[5] = d5;
  data_pins[6] = d6;
  data_pins[7] = d7;
  row_count = row;
  col_count = col;
  is_8_bit_mode = false;
  init();
}


void LCD::write_4_bit_mode(int cmd)
{
  // set values of rs and rw pins
  digitalWrite(rs_pin, (cmd & (1 << 9)) ? HIGH : LOW); // rs
  digitalWrite(rw_pin, (cmd & (1 << 8)) ? HIGH : LOW); // rw

  // set values of data pins for high nibble
  for (int i = 4; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (cmd & (1 << i)) ? HIGH : LOW);
  }

  // pulse enable pin
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);
  delayMicroseconds(1);

  // set values of data pins for low nibble
  for (int i = 4; i < 8; i++)
  {
    digitalWrite(data_pins[i], (cmd & (1 << (i - 4))) ? HIGH : LOW);
  }

  // pulse enable pin
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);
  delayMicroseconds(50);
}

void LCD::write_8_bit_mode(int cmd)
{
  // set values of rs and rw pins
  digitalWrite(rs_pin, (cmd & (1 << 9)) ? HIGH : LOW); // rs
  digitalWrite(rw_pin, (cmd & (1 << 8)) ? HIGH : LOW); // rw

  // set values of data pins
  for (int i = 0; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (cmd & (1 << i)) ? HIGH : LOW);
  }

  // pulse enable pin
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);
  delayMicroseconds(50);
}

void LCD::write_instruction(int cmd)
{
  // wait until busy flag turns 0
  while (read_instruction(BUSYFLAG_ADDRCOUNTER_READ) >= 0x80)
  {
  }

  if (is_8_bit_mode)
  {
    write_8_bit_mode(cmd);
  }
  else
  {
    write_4_bit_mode(cmd);
  }
}

int LCD::read_instruction(int cmd)
{
  if (is_8_bit_mode)
  {
    return read_8_bit_mode(cmd);
  }
  else
  {
    return read_4_bit_mode(cmd);
  }
}

int LCD::read_4_bit_mode(int cmd)
{
  int read_value = 0x0;
  
  // set values of rs and rw pins
  digitalWrite(rs_pin, (cmd & (1 << 9)) ? HIGH : LOW); // rs
  digitalWrite(rw_pin, (cmd & (1 << 8)) ? HIGH : LOW); // rw

  // read high nibble
  digitalWrite(en_pin, HIGH);
  for (int i = 4; i < 8; i++)
  {
    pinMode(data_pins[i], INPUT);
    read_value |= digitalRead(data_pins[i]) << i;
  }
  digitalWrite(en_pin, LOW);

  delayMicroseconds(1);

  // read low nibble
  digitalWrite(en_pin, HIGH);
  for (int i = 4; i < 8; i++)
  {
    read_value |= digitalRead(data_pins[i]) << (i - 4);
  }
  digitalWrite(en_pin, LOW);

  return read_value;
}

int LCD::read_8_bit_mode(int cmd)
{
  int read_value = 0x0;

  digitalWrite(rs_pin, (cmd & (1 << 9)) ? HIGH : LOW);
  digitalWrite(rw_pin, (cmd & (1 << 8)) ? HIGH : LOW);

  // read data pins
  digitalWrite(en_pin, HIGH);
  for (int i = 0; i < 8; i++)
  {
    pinMode(data_pins[i], INPUT);
    read_value |= digitalRead(data_pins[i]) << i;
  }
  digitalWrite(en_pin, LOW);

  return read_value;
}

void LCD::init()
{
  pinMode(en_pin, OUTPUT);
  pinMode(rs_pin, OUTPUT);
  pinMode(rw_pin, OUTPUT);
  digitalWrite(en_pin, LOW);
  digitalWrite(rs_pin, LOW);
  digitalWrite(rw_pin, LOW);

  // wait for internal reset of lcd
  delay(100);

  // intialization instructions (datasheet pg. 45-46)
  if (is_8_bit_mode)
  {
    init_by_instruction_8_bit();
  }
  else
  {
    init_by_instruction_4_bit();
  }
  write_instruction(FUNCTION_SET(is_8_bit_mode, (row_count > 1 ? true : false), true));
  write_instruction(DISPLAY_CONTROL(0, 0, 0));
  write_instruction(CLEAR_DISPLAY);
  write_instruction(ENTRY_MODE_SET(1, 0));

  write_instruction(DISPLAY_CONTROL(1, 0, 1));
}

void LCD::init_by_instruction_4_bit()
{
  digitalWrite(rs_pin, LOW); // rs
  digitalWrite(rw_pin, LOW); // rw

  for (int i = 4; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x03 & (1 << (i - 4))) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);

  // wait more than 4.1ms
  delay(5);

  for (int i = 4; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x03 & (1 << (i - 4))) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);

  // wait more than 100us
  delayMicroseconds(150);

  for (int i = 4; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x03 & (1 << (i - 4))) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);

  delayMicroseconds(1);

  for (int i = 4; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x02 & (1 << (i - 4))) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);
  delayMicroseconds(1);
}

void LCD::init_by_instruction_8_bit()
{
  digitalWrite(rs_pin, LOW); // rs
  digitalWrite(rw_pin, LOW); // rw

  for (int i = 0; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x30 & (1 << i)) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);

  // wait more than 4.1ms
  delay(5);

  for (int i = 0; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x30 & (1 << i)) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);

  // wait more than 100us
  delayMicroseconds(150);

  for (int i = 0; i < 8; i++)
  {
    pinMode(data_pins[i], OUTPUT);
    digitalWrite(data_pins[i], (0x30 & (1 << i)) ? HIGH : LOW);
  }
  digitalWrite(en_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(en_pin, LOW);
  delayMicroseconds(1);
}

void LCD::display_config(bool display_on, bool cursor_on, bool cursor_blink)
{
  write_instruction(DISPLAY_CONTROL(display_on, cursor_on, cursor_blink));
}

void LCD::clear()
{
  write_instruction(CLEAR_DISPLAY);
}

void LCD::home()
{
  write_instruction(RETURN_HOME);
}

void LCD::write(String str)
{
  for (char c : str)
  {
    if (c == '\n' && row_count > 1)
    {
      write_instruction(DDRAM_ADDR_SET(0x40));
      continue;
    }
    else if (c == '\n' && row_count == 1)
    {
      continue;
    }
    write_instruction(CGRAM_OR_DDRAM_DATA_WRITE((int)c));
  }
}

void LCD::set_cursor(int x, int y)
{
  int row_start_addr = x == 1 ? 0x00 : (row_count > 1 ? 0x40 : 0x00);
  int addr = row_start_addr + (y - 1);

  write_instruction(DDRAM_ADDR_SET(addr));
}

void LCD::write_custom_char(int custom_char_no)
{
  if (custom_char_no > 8)
  {
    Serial.println("ERROR: There are maximum 8 custom characters.");
    return;
  }
  write_instruction(CGRAM_OR_DDRAM_DATA_WRITE(custom_char_no));
}

void LCD::build_custom_char(int custom_char_no, int bitmap[8][5])
{
  if (custom_char_no > 8)
  {
    Serial.println("ERROR: maximum 8 custom characters can be defined.");
    return;
  }

  // save current cursor position (ddram address) to recover after setting cgram data
  int saved_ddram_addr;
  do
  {
    saved_ddram_addr = read_instruction(BUSYFLAG_ADDRCOUNTER_READ);
  } while (saved_ddram_addr & (1 << 7)); // read until busy flag gets 0

  // set cgram data (datasheet pg.19)
  for (int i = 0; i < 8; i++)
  {
    write_instruction(CGRAM_ADDR_SET(((custom_char_no << 3) | (0x00 + i))));
    int data = 0x00;
    for (int j = 0; j < 5; j++)
    {
      data |= bitmap[i][j] << 4 - j;
    }
    write_instruction(CGRAM_OR_DDRAM_DATA_WRITE(data));
  }

  // recover cursor position
  write_instruction(DDRAM_ADDR_SET(saved_ddram_addr));
}