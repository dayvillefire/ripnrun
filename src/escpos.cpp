#include <Arduino.h>
#include "escpos.hpp"

String EscPos::initialize()
{
    String x = String();
    x += "\x1B@";
    return x;
}

String EscPos::align(int align)
{
    String x = String();
    x += "\x1B\x61";
    x += char(align);
    return x;
}

String EscPos::feed(int lines)
{
    String x = String();
    x += "\x1B\x64";
    x += char(lines);
    return x;
}

String EscPos::char_width_height(int width, int height)
{
    String x = String();
    x += "\x1D\x21";
    x += char(width | height);
    return x;
}

String EscPos::reverse_printing(bool on)
{
    String x = String();
    x += "\x1D\x42";
    x += char(on ? 1 : 0);
    return x;
}

String EscPos::set_printmode(int printmode)
{
    String x = String();
    x += "\x1B\x21";
    x += char(printmode);
    return x;
}

String EscPos::printimage(uint8_t *buffer, int width, int height)
{
    int pitch = (width + 7) >> 3;
    uint8_t *x;

    String b = String();
    b += "\x1Dv00";

    b += char((width + 7) >> 3);
    b += char(0);
    b += char((uint8_t)height);
    b += char((uint8_t)(height >> 8));

    // Print the graphics
    x = buffer;
    for (int y = 0; y < height; y++)
    {
        for (int i = 0; i < pitch; i++)
        {
            b += char(*(x + i));
        }
        x += pitch;
    } // for y

    return b;
}
