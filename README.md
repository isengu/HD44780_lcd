# HD44780 Dot Matrix LCD Controller Library

This is a weekend project naive arduino library made for HD44780 controller based lcd's. It abstracts the complexity of directly interacting with the LCD's hardware, making it simple to display text, numbers, and custom characters.

## Usage

```C++
#include "lcd.h"

LCD* lcd;

void setup() {
  Serial.begin(9600);

  int rs = 12;
  int rw = 13;
  int en = 11;
  lcd = new LCD(en, rs, rw, 5, 4, 3, 2);
}

void loop() {
  if ((millis() % 1000) == 0) {
    lcd->set_cursor(2, 1);
    lcd->write(String(millis() / 1000));
    delay(1);
  }
}

```

## Features

### Simple Text Printing:

With the `write(String str)` method, given text can be printed on the lcd.

```C++
lcd->write("HELLO WORLD");
```

### Custom Charactes:

These type of lcd's are dot matrix displays and each character represented in a 5x8 (or 5x10) pixel group. So we can define a custom character in the form of a nested array consisting of zeros and ones. Ones represents lighting pixels.

Define and print a custom character:
```C++
int custom_char[8][5] = {
      {0, 1, 1, 1, 0},
      {1, 1, 1, 1, 1},
      {1, 1, 1, 1, 0},
      {1, 1, 1, 0, 0},
      {1, 1, 0, 0, 0},
      {1, 1, 1, 1, 1},
      {1, 1, 1, 1, 1},
      {0, 1, 1, 1, 0}
  };
lcd->build_custom_char(0, custom_char);
lcd->write_custom_char(0);
```

### Compatibility:

Supports a wide range of lcd screens based on the HD44780U controller (LCD 1602). Also supports both 8 bit or 4 bit modes. (in 4 bit mode only d4, d5, d6, d7 data lines of the lcd used, otherwise all 8 data lines are used)

## Reference

[HD44780 Datasheet](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)