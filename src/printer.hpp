#ifndef PRINTER_HPP
#define PRINTER_HPP 1

#include "usb/usb_host.h"
#include "usbhelp.hpp"

static void print_to_printer(String aLine);
void show_config_desc_full(const usb_config_desc_t *config_desc);
void prepare_endpoints(const void *p);
void check_interface_desc_printer(const void *p);
static void printer_transfer_cb(usb_transfer_t *transfer);
void printPayload(String aLine);

#endif /* PRINTER_HPP */