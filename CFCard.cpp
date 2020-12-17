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

#include "CFCard.h"

#define MODE_NOTINIT 0
#define MODE_LBA 1
#define MODE_CHS 2

uint8_t disk_mode;
uint8_t mode = MODE_NOTINIT;

uint8_t dior_mask, diow_mask;
volatile uint8_t *dior_mode, *dior_out;
volatile uint8_t *diow_mode, *diow_out;

inline void dior(uint8_t status)
{
    status == HI_Z ? (*dior_mode &= ~dior_mask) : (*dior_mode |= dior_mask);
    status == HIGH ? (*dior_out |= dior_mask) : (*dior_out &= ~dior_mask);
}
inline void diow(uint8_t status)
{
    status == HI_Z ? (*diow_mode &= ~diow_mask) : (*diow_mode |= diow_mask);
    status == HIGH ? (*diow_out |= diow_mask) : (*diow_out &= ~diow_mask);
}

/* Put register address on data bus */
void inline put_register(uint8_t addr)
{
    REG_ADDR_MODE |= REG_ADDR_MASK;
    REG_ADDR_OUT &= ~REG_ADDR_MASK;
    REG_ADDR_OUT |= REG_ADDR_MASK & addr;
}

/* Read register */
uint8_t register_read(uint8_t addr)
{
    dior(NEGATE);
    DD_LSB_MODE = 0; //Data pins as Input hd -> Arduino
    DD_LSB_OUT = 0;
    put_register(addr);
    dior(ASSERT);
    uint8_t ret = DD_LSB_IN;
    dior(NEGATE);
    return ret;
}

uint8_t register_write(uint8_t addr, uint8_t value)
{
    diow(NEGATE);
    DD_LSB_MODE = 0xFF; //Data pins as output Arduino -> hd
    DD_LSB_OUT = value;
    put_register(addr);
    diow(ASSERT);
    diow(NEGATE);
}


// Helper functions for Diagnosis 
/* output sprintf formatted string */
void msgout(const char *msg, ...)
{
    char str[255];
    va_list args;
    va_start(args, msg);
    vsprintf(str, msg, args);
    Serial.println(str);
}

/* waits until BSY is reset and optional flags in status register are set */
uint8_t status_wait(uint8_t flags = 0)
{
    uint16_t retries = 0;
    uint8_t regval = register_read(REG_STATUS);
    while ((regval & BSY) || ((regval & flags) != flags))
    {
        if (retries++ >= STATUS_TIMEOUT)
        {
            msgout("ERROR: status_wait() timeout. Increasing STATUS_TIMEOUT might help");
            return retries;
        }
        delayMicroseconds(1000);
        regval = register_read(REG_STATUS);
    }
    //dbgout("status_wait() loops: %d", retries);
    return 0;
}

void dump_status()
{
    uint8_t s = register_read(REG_STATUS);
    msgout("BSY | DRDY | DWF | DSC | DRQ | CORR | IDX | ERR");
    msgout("%d   | %d    | %d   | %d   | %d   | %d    | %d   | %d",
           bitRead(s, 7), bitRead(s, 6), bitRead(s, 5), bitRead(s, 4),
           bitRead(s, 3), bitRead(s, 2), bitRead(s, 1), bitRead(s, 0));
}

void dump_error()
{
    uint8_t s = register_read(REG_ERR);
    msgout("BBK | UNC | MC | IDNF | MCR | ABRT | TK0NF | AMNF");
    msgout("%d   | %d   | %d  | %d    | %d   | %d    | %d     | %d",
           bitRead(s, 7), bitRead(s, 6), bitRead(s, 5), bitRead(s, 4),
           bitRead(s, 3), bitRead(s, 2), bitRead(s, 1), bitRead(s, 0));
}

void dump_regs()
{
    if (register_read(REG_DH) & LBA)
    {
        msgout("Data | Sec Cnt | LBA/CHS  ");
        msgout("0x%02x | %03d     | 0x%02x%02x%02x%02x",
               register_read(REG_D), register_read(REG_SC), register_read(REG_DH), register_read(REG_CY_HI), register_read(REG_CY_LO), register_read(REG_SN));
    }
    else
    {
        msgout("Data | Sec Cnt | Drv/Hd | Cyl Hi | Cyl Lo | Sec Nr. ");
        msgout("0x%02x | %03d     | 0x%02x   | 0x%02x   | 0x%02x   | 0x%02x",
               register_read(REG_D), register_read(REG_SC), register_read(REG_DH), register_read(REG_CY_HI), register_read(REG_CY_LO), register_read(REG_SN));
    }
}

#if defined IRQPIN
void irqfunc()
{
    dbgout("-------Interrupt---------");
    dump_status();
    dbgout("-------------------------");
}
#endif

uint8_t hd_init(bool lba)
{
    if(mode)
        return mode;
    Serial.print("Init disk");
    uint8_t dior_port = digitalPinToPort(DIOR_PIN);
    dior_mask = digitalPinToBitMask(DIOR_PIN);
    dior_mode = portModeRegister(dior_port);
    dior_out = portOutputRegister(dior_port);
    uint8_t diow_port = digitalPinToPort(DIOW_PIN);
    diow_mask = digitalPinToBitMask(DIOW_PIN);
    diow_mode = portModeRegister(diow_port);
    diow_out = portOutputRegister(diow_port);
    diow(NEGATE);
    dior(NEGATE);

    uint8_t init_timeout = 0;

    if(!(register_read(REG_STATUS) & BSY))
    {
        msgout(" Error: Drive not found");
        delay(1000);
        return MODE_NOTINIT;
    }

    //Wait max. 30 seconds until drive is ready
    while ((register_read(REG_STATUS) & 0xc0) != DRDY) //until BSY == 0 and DRDY = 1
    {
        delay(1000);
        if (init_timeout++ > 30)
        {
            msgout("ERROR: hd_init() timed out. Try to reset disk (Pull pin 1 to ground)");
            dump_status();
            return MODE_NOTINIT;
        }
        Serial.print(".");
    }
    //Set lba mode
    if(lba)
    {
        mode = MODE_LBA;
        register_write(REG_DH, 0xe0);
        register_write(REG_CMD, CMD_INITPARAMS);
        status_wait();
        if (!(register_read(REG_DH) & LBA))
        {
            msgout("Warning: LBA mode not available.");
            mode = MODE_CHS;
        }
    }
    // Set 8 bit data transfer
    register_write(REG_FR, 0x01);
    register_write(REG_CMD, CMD_SETFR);
    status_wait();
    if (register_read(REG_ERR) & ABRT)
    {
        msgout("Error: 8 Bit transfer mode could not be set");
        return MODE_NOTINIT;
    }

#if defined IRQPIN
    attachInterrupt(digitalPinToInterrupt(IRQPIN), irqfunc, RISING);
#endif
    msgout(" success");
    return mode;
} //hd_init()

uint16_t hd_read_sector(uint32_t sector, uint8_t *buffer, size_t size)
{
    register_write(REG_LBA_24_27, 0xe0 | (sector >> 24) & 0x0f);
    register_write(REG_LBA_16_23, sector >> 16);
    register_write(REG_LBA_8_15, sector >> 8);
    register_write(REG_LBA_0_7, sector);
    register_write(REG_CMD, CMD_READ);
    if (status_wait(DRQ))
    {
        msgout("ERROR: cannot find sector. Maybe it is out of range?");
        return 0;
    }
    for (int i = 0; i < size; i++)
    {
        buffer[i] = register_read(REG_D);
    }
    return size;
}

uint16_t hd_read_multiple(uint32_t sector, uint8_t *buffer)
{
    static uint32_t sector_ = sector;
    static uint32_t current = sector;
    if(sector != sector_)
    {
        sector_ = sector;
        current = sector;
    }
    return hd_read_sector(current++, buffer);
}

uint8_t hd_write_sector(uint32_t sector, const uint8_t *buffer)
{
    uint8_t mode = register_read(REG_LBA_24_27) & 0xf0;
    register_write(REG_LBA_24_27, mode | (sector >> 24) & 0x0f);
    register_write(REG_LBA_16_23, sector >> 16);
    register_write(REG_LBA_8_15, sector >> 8);
    register_write(REG_LBA_0_7, sector);
    register_write(REG_CMD, CMD_WRITE);
    if (status_wait(DRQ))
    {
        msgout("ERROR: Writing to drive failed");
        return 0;
    }
    for (int i = 0; i < SECTOR_LEN; i++)
    {
        register_write(REG_D, buffer[i]);
    }
    return 1;
}

uint8_t hd_write_multiple(uint32_t sector, const uint8_t *buffer)
{
    static uint32_t sector_ = sector;
    static uint32_t current = sector;
    if(sector != sector_)
    {
        sector_ = sector;
        current = sector;
    }
    return hd_write_sector(current++, buffer);
}

bool hd_isInit(){
    return mode;
}
