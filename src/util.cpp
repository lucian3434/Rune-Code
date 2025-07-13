#include "util.h"

// because printf() over usb blocks if a serial connection isnt active apparently
void uprintf(const char* format, ...) {
    if (!stdio_usb_connected()) {
        return;  // don't do anything if usb isn't connected
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }