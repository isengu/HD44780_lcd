#ifndef _LCD_
#define _LCD_

#include <Arduino.h>

class LCD
{
  int en_pin;
  int rs_pin;
  int rw_pin;
  int data_pins[8]; // d0, d1, ..., d7

  int row_count = 2;
  int col_count = 16;

  bool is_8_bit_mode = true;

public:
  // ctors
  LCD(int en, int rs, int rw, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7);
  LCD(int en, int rs, int rw, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7, int row, int col);
  LCD(int en, int rs, int rw, int d4, int d5, int d6, int d7);
  LCD(int en, int rs, int rw, int d4, int d5, int d6, int d7, int row, int col);

  void clear(); // clears all of the screen
  void home(); // puts cursor at the begining
  void display_config(bool display_on, bool cursor_on, bool cursor_blink); // configures display properties: display on/off - cursor on/off - cursor blinking on/off
  void set_cursor(int x, int y); // sets the cursor to the given x, y coordinate (x and y is 1 indexed)
  void write(String str); // writes the given text to the screen from where the cursor is
  void build_custom_char(int custom_char_no, int bitmap[8][5]); // maximum 8 custom character can be defined and to define the character define a nested array 8x5 this represent a cell in the lcd and fill it with ones and zeros, ones represents the active dots zeros represents inactive dots
  void write_custom_char(int custom_char_no); // writes the custom character of the given number from where the cursor is

private:
  void init(); // initializes the lcd
  void init_by_instruction_8_bit(); // in case of internal reset circut didn't work this executes all the necessary instructions for initialization of the lcd
  void init_by_instruction_4_bit(); // same as above but for 4 bit mode
  void write_instruction(int cmd); // executes given write instruction, instruction must be the representation of pin values in the binary format that each bit represents a pin value. From msb to lsb bit represents pins: RS R/W D7 D6 D5 D4 D3 D2 D1 D0 
  int read_instruction(int cmd); // executes given read instruction and returns pin values in the binary format where each bit represents a pin value. From msb to lsb bits represents pins: D7 D6 D5 D4 D3 D2 D1 D0
  void write_4_bit_mode(int cmd); // exectues write instruction in 4 bit mode where it sends in two cycles
  void write_8_bit_mode(int cmd); // executes write instruction in 8 bit mode
  int read_4_bit_mode(int cmd); // executes read instruction in 4 bit mode
  int read_8_bit_mode(int cmd); // executes read instruction in 8 bit mode
};

#endif