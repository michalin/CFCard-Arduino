#include "debug.h"

/* output sprintf formatted string */
void dbgout(const char *msg, ...)
{
    char str[255];
    va_list args;
    va_start(args, msg);
    vsprintf(str, msg, args);
    Serial.print(str);
}


void hexdump(const byte *buf, size_t len)
{
    uint16_t addr = 0;
    //Serial.println();
    while (addr < len)
    {
        dbgout("0x%04x\t", addr);
        do
        {
            dbgout("%02x ", buf[addr]);
        } while (++addr % 16 != 0);
        addr -= 16;
        Serial.print("\t");
        do
        {   if(buf[addr] >= 32 && buf[addr] < 127)
                dbgout("%c", buf[addr]);
            else
                Serial.print(".");
        } while (++addr % 16 != 0);

        Serial.println("");
    }
}
