#include <Arduino.h>
#include "escpos.hpp"

extern usb_transfer_t *PrinterOut;

EscPos::EscPos()
{
    buf = "";
}

void EscPos::initialize()
{
    buf += "\x1B@";
}

void EscPos::align(int align)
{
    buf += "\x1B\x61";
    buf += char(align);
}

void EscPos::feed(int lines)
{
    buf += "\x1B\x64";
    buf += char(lines);
}

void EscPos::text(String text)
{
    buf += text;
}

void EscPos::char_width_height(int width, int height)
{
    buf += "\x1D\x21";
    buf += char(width | height);
}

void EscPos::reverse_printing(bool on)
{
    buf += "\x1D\x42";
    buf += char(on ? 1 : 0);
}

void EscPos::set_printmode(int printmode)
{
    buf += "\x1B\x21";
    buf += char(printmode);
}

void EscPos::printimage(uint8_t *buffer, int width, int height)
{
    int pitch = (width + 7) >> 3;
    uint8_t *x;

    buf += "\x1Dv00";

    buf += char((width + 7) >> 3);
    buf += char(0);
    buf += char((uint8_t)height);
    buf += char((uint8_t)(height >> 8));

    // Print the graphics
    x = buffer;
    for (int y = 0; y < height; y++)
    {
        for (int i = 0; i < pitch; i++)
        {
            buf += char(*(x + i));
        }
        x += pitch;
    } // for y
}

void EscPos::flush()
{
    if (PrinterOut == NULL)
    {
        Serial.print("Waiting for PrinterOut initialization ...");
        while (PrinterOut == NULL)
        {
            Serial.print(".");
            delay(100);
            usbh_task();
        }
        Serial.println("");
    }
    PrinterOut->num_bytes = buf.length();
    memcpy(PrinterOut->data_buffer, buf.c_str(), PrinterOut->num_bytes);
    esp_err_t err = usb_host_transfer_submit(PrinterOut);
    buf = "";
}