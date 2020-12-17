/*Arduino Library for CF Cards and PATA hard disks
Copyright (C) 2020  Michael Linsenmeier (michalin70@gmail.com)
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

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
