#include "lcd.h"

LCD* lcd;

void setup() {
  Serial.begin(9600);

  int rs = 12;
  int rw = 13;
  int en = 11;
  // lcd = new LCD(en, rs, rw, 9, 8, 7, 6, 5, 4, 3, 2);
  lcd = new LCD(en, rs, rw, 5, 4, 3, 2, 2, 16);

  lcd->display_config(true, true, false);
  
  lcd->write("HELLO WORLD!");

  int custom_char[8][5] = {
      {0, 1, 1, 1, 0},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1}
  };
  int custom_char2[8][5] = {
      {1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1},
      {1, 0, 1, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {0, 1, 1, 1, 0}
  };
  int custom_char3[8][5] = {
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
  lcd->build_custom_char(1, custom_char2);
  lcd->build_custom_char(2, custom_char3);
  
  lcd->set_cursor(1, 13);
  lcd->write_custom_char(0);
  lcd->set_cursor(2, 13);
  lcd->write_custom_char(1);
  lcd->set_cursor(2, 14);
  lcd->write_custom_char(2);
  lcd->write_custom_char(2);
  lcd->write_custom_char(2);
}

void loop() {
  if ((millis() % 1000) == 0) {
    lcd->set_cursor(2, 1);
    lcd->write(String(millis() / 1000));
    delay(1);
  }
}
