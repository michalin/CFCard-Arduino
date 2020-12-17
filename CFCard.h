#ifndef pata_h
#define pata_h
#include <Arduino.h>
//#include "include/debug.h"

//User constants
//#define USE_FAT  //Uncomment to use FAT Filesystems with SDFat Library V2
#define STATUS_TIMEOUT 100 //Time [ms] to wait for status change. Increase for slow disks

#ifdef USE_FAT 
#include "CFFatDriver.h"
#endif

/* 8 Byte Data Bus */
#define DD_LSB_MODE DDRL //Device Pin 17 - 3, odd pin numbers, Ardu Pin 49-42
#define DD_LSB_OUT PORTL //Ardiuno -> hd
#define DD_LSB_IN PINL   //hd -> Arduino

/* Write strobe signal.*/
#define DIOW_PIN 32 //Device Pin 23

/* Read strobe signal. */
#define DIOR_PIN 30 //Device Pin 25

/* 3-bit binary coded address asserted by the host to access a register or data port 
Dev. Pin    |   Bit     | Arduino Pin
35          |   0       | 22 (PA0)
33          |   1       | 23 (PA1)
36          |   2       | 24 (PA2)*/
//Port and mask for Address pins
#define REG_ADDR_MODE DDRA
#define REG_ADDR_MASK 0x07
#define REG_ADDR_OUT PORTA

//Command block registers
#define REG_D 0b000   //Data register
#define REG_ERR 0b001 //Read: Error
#define REG_FR 0b001  //Write: Features
#define REG_SC 0b010  //Sector Count

#define REG_SN 0b011        //Sector Number
#define REG_LBA_0_7 0b011   //LBA Bits 0-7
#define REG_CY_LO 0b100     //Cylinder Low
#define REG_LBA_8_15 0b100  //LBA Bits 8-15
#define REG_CY_HI 0b101     //Cylinder High
#define REG_LBA_16_23 0b101 //LBA Bits 16-23
#define REG_DH 0b110        //Drive/Head
#define REG_LBA_24_27 0b110 //LBA Bits 24-27
#define REG_STATUS 0b111    //Read: Status
#define REG_CMD 0b111       //Write: Command

//Control block registers (not used)
#define REG_IDLE 0b1000     //Data Bus High Imped
#define REG_ALT_STAT 0b1110 //Read: Alternate Status
#define REG_CTRL 0b1110     //Write: Device Control
#define REG_ADDR 0b1111     //Read: Drive Address

//Drive commands
#define CMD_INITPARAMS 0x91 //Init drive parameters
#define CMD_READ 0x21       //Read sector w.o. retry
#define CMD_WRITE 0x31      //Write sector w.o retry
#define CMD_SETFR 0xEF      //
#define CMD_IDENT 0xEC      //Identify drive

//Control flags
#define SRST 0x0c //Reset
#define NIEN 0x0a //Interrupt disable

//Status flags
#define BSY 1 << 7  //Drive busy / not available
#define DRDY 1 << 6 //Drive ready for command
#define DRQ 1 << 3  // Drive ready for R/W

//DH register flags
#define LBA 1 << 6 //Set when LBA enabled

//Error flags
#define ABRT 1 << 2 //Unknown command or parameter

//signal states (Active high)
#define HI_Z 0
#define ASSERT -1
#define NEGATE 1

#define SECTOR_LEN 512

/** Init Harddisk
 * \param[in] mode: false: CHS mode, true: LBA (default)
 * \return 0: error, 1: LBA, 2: CHS
 */
uint8_t hd_init(bool lba = true);

/** 
 * \return true: HD can be used, False: HD is not initialized or error
 * /
bool hd_isinit();

/** Read bytes from a sector
 * \param[in] sector: Sector or CHS to be read
 * \param[out] *buffer: buffer with bytes read
 * \param[in] size: number of bytes to read
 * \return number of bytes read or 0 on error
*/
uint16_t hd_read_sector(uint32_t sector, uint8_t *buffer, size_t size = SECTOR_LEN);

/** Read bytes from multiple sectors: Every subsequent call returns the next sector
 * \param[in] sector: First Sector or CHS to be read
 * \param[out] *buffer: buffer with bytes read
 * \return number of bytes read or 0 on error
*/
uint16_t hd_read_multiple(uint32_t sector, uint8_t *buffer);

/** Write to sector
 * \param[in] sector: Sector or CHS to write
 * \param[in] *buffer: Buffer with bytes to write
 * \return 1 on success, 0 on error
*/
uint8_t hd_write_sector(uint32_t sector, const uint8_t *buffer);

/** Write bytes to multiple sectors: Every subsequent call writes the next sector
 * \param[in] sector: First Sector or CHS to be write
 * \param[in] *buffer: buffer with bytes to be written
 * \return 1 on success, 0 on error
*/
uint8_t hd_write_multiple(uint32_t sector, const uint8_t *buffer);

//** Sprintf formatted message */
void msgout(const char *, ...);

#endif