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

class EscPos
{
public:
    EscPos();
    String initialize();
    String align(int align);
    String feed(int lines);
    String char_width_height(int width, int height);
    String reverse_printing(bool on);
    String set_printmode(int printmode);
    String printimage(uint8_t *buffer, int width, int height);
};

#endif /* ESCPOS_HPP */