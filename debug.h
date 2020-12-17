#ifndef DEBUG_H
#define DEBUG_H
#include "Arduino.h"

/* output sprintf formatted string */
void dbgout(const char*, ...);
void hexdump(const byte*, size_t);

#endif