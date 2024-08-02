#ifndef ESCPOS_HPP
#define ESCPOS_HPP 1

#include <Arduino.h>

#define ESCPOS_ALIGN_LEFT 48
#define ESCPOS_ALIGN_CENTER 49
#define ESCPOS_ALIGN_RIGHT 50

#define ESCPOS_WIDTH_NORMAL 0
#define ESCPOS_WIDTH_DOUBLE 16

#define ESCPOS_HEIGHT_NORMAL 0
#define ESCPOS_HEIGHT_DOUBLE 1

#define ESCPOS_PRINTMODE_OFF 0
#define ESCPOS_PRINTMODE_FONTB 1
#define ESCPOS_PRINTMODE_BOLD 8
#define ESCPOS_PRINTMODE_DOUBLEHEIGHT 16
#define ESCPOS_PRINTMODE_DOUBLEWIDTH 32
#define ESCPOS_PRINTMODE_UNDERLINE 128

String escpos_initialize();
String escpos_align(int align);
String escpos_feed(int lines);
String escpos_char_width_height(int width, int height);
String escpos_reverse_printing(bool on);
String escpos_set_printmode(int printmode);
String escpos_printimage(uint8_t *buffer, int width, int height);

#endif /* ESCPOS_HPP */