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

#include "CFFatDriver.h"

#if SPI_DRIVER_SELECT == 3 // Must be set in SdFat/SdFatConfig.h

void HdDrive::activate()
{
#ifdef SDCARD
  SPI.beginTransaction(m_spiSettings);
#endif
}
// Initialize the SPI bus
void HdDrive::begin(SdSpiConfig config)
{
#ifdef SDCARD
  SPI.begin();
#endif
}

// Deactivate SPI hardware.
void HdDrive::deactivate()
{
#ifdef SDCARD
  SPI.endTransaction();
#endif
}

/* Receive command responses from the fake SD Card
and return them to the SDFat library */
uint8_t HdDrive::receive()
{
  //msgout("-->HdDrive::receive(0x%02x))", rcv);
  uint8_t ret;
  static uint8_t count;
  switch (command)
  {
  case CMD0:
    //msgout("%d CMD0: GO_IDLE_STATE (Reset SD Card)", count);
#ifdef SDCARD
    ret = SPI.transfer(0XFF);
    //msgout("response: %x", ret);
    return ret;
#else
    switch (hd_init())
    {
    case 0:
      return cmd_respond(count, cmd0_response_error);
    case 1:
      return cmd_respond(count, cmd0_response);
    case 2:
      //msgout("Error: HD does not support LBA mode");
      return cmd_respond(count, cmd0_response_error);
    }
#endif

  case CMD8:
    //msgout("CMD8: SEND_IF_COND (send interface condition): 0x%08lx", argument);
    return cmd_respond(count, cmd8_response);
#ifdef SDCARD
    return SPI.transfer(0XFF);
#endif
    return ret;

    /*  case CMD9:
    msgout("CMD9: SEND_CSD (Read CSD Register): 0x%08lx", argument);
    msgout("0x%02x", ret=SPI.transfer(0xff));
    return(ret);
    break;*/

  case CMD12:
    //msgout("CMD12: STOP_TRANSMISSION - end multiple sector read sequence");
    return cmd_respond(count, cmd12_response);
#ifdef SDCARD
    return SPI.transfer(0XFF);
#endif

  case CMD18:
    //msgout("\nCMD18: READ_MULTIPLE_SECTOR (read multiple data sectors): 0x%06lx", argument);
#ifdef SDCARD
    ret = SPI.transfer(0XFF);
#else
    if (read_progress)
      ret = cmd_respond(count, cmd18_multi_response);
    else
      ret = cmd_respond(count, cmd18_single_response);
#endif
    //msgout("%d response: %x, progress: %d", count, ret, read_progress);
    return ret;

  case CMD25:
    //msgout("\nCMD25: WRITE_MULTIPLE_SECTOR (write multiple data sectors): 0x%06lx", count, argument);
    if (!write_progress)
    {
      if (sect_written == 0) //Single or multiple sector write started
        ret = cmd_respond(count, cmd25_response_start);
      else if (sect_written == 1) // Single sector transfer ended
      {
        ret = cmd_respond(count, cmd25_response_single_end);
        count == 0 ? sect_written = 0 : sect_written;
      }
      else // Multiple sector transfer ended
      {
        ret = cmd_respond(count, cmd25_response_multi_end);
        count == 0 ? sect_written = 0 : sect_written;
      }
    }
    else //Writing in progress, request next sector
    {
      ret = cmd_respond(count, cmd25_response_next);
    }
#ifdef SDCARD
    ret = SPI.transfer(0XFF);
#endif
    //msgout("%d response: %x Sector Nr: %d", count, ret, sect_written);
    return ret;

  case ACMD41:
    //msgout("%d ACMD41: SD_SEND_OP_COMD (Send operating condition): 0x%08lx", count, argument);
#ifdef SDCARD
    return SPI.transfer(0XFF);
#endif
    return cmd_respond(count, acmd41_response);

  case CMD55:
    //msgout("%d CMD55: APP_CMD (enable application specific command)", count);
#ifdef SDCARD
    return SPI.transfer(0XFF);
#endif
    return cmd_respond(count, cmd55_response);

  case CMD58:
    //msgout("%d CMD58: READ_OCR (read the OCR register)", count);
#ifdef SDCARD
    return SPI.transfer(0XFF);
#endif
    return cmd_respond(count, cmd58_response);

  default:
    msgout("Unknown Command=0x%x, Argument=%08lx", command, argument);
#ifdef SDCARD
    return SPI.transfer(0XFF);
#endif
    return 0;
  }
}

/* return specific fake response */
uint8_t HdDrive::cmd_respond(uint8_t &count, const uint8_t *cmd_response)
{
  uint8_t ret;
  if (count++ < cmd_response[0])
  {
    ret = cmd_response[count];
    count == cmd_response[0] ? count = 0 : count;
  }
  else
    count = 0;
  return ret;
}

/* Process SD card command that comes from the SDFat library 
This extracts the command code and argument and stores them to class variables */
void HdDrive::send(uint8_t data)
{
  //msgout("HdDrive::send(data = %d)", data);
#ifdef SDCARD
  SPI.transfer(data);
#endif
  static int8_t arg_pos;
  if ((arg_pos == 0) && ((data & 0xf0) == 0xf0)) //Ignore the 10 0xFF on init and process write tokens
  {
    switch (data)
    {
    case 0xfc: //Multiple write token
      sect_written++;
      write_progress = true;
      //msgout("Sector Number: %d", sect_written);
      break;
    case 0xfd: //Transfer end token
      write_progress = false;
      //msgout("Write finished (0xfd)");
      break;
    case 0xff:
      if (command == CMD12)
      {
        read_progress = false;
        //msgout("Read finished"); //Single or multiple sector read complete
      }
      else
      {
        //msgout("Ignore: 0xff");
      }

      break;

    default:
      msgout("Ignored: 0x%02x", data);
    }
    return;
  }
  if ((arg_pos == 0) && ((data & 0xC0) == 0x40)) //Extract SD Card command
  {
    argument = 0;
    command = data & 0x3f;
    arg_pos++;
    //msgout("Command: %d", command);
  }
  else if (arg_pos <= 4) //Extract argument
  {
    argument <<= 8;
    argument += data;
    arg_pos++;
  }
  else //Ignore Checksum
  {
    arg_pos = 0;
    //msgout("Argument: 0x%08x", argument);
  }
  //msgout("data: %02x, argument: %08lx\n", data, argument);
}

// Write file data to disk.
void HdDrive::send(const uint8_t *buf, size_t count)
{
  //msgout("-->HdDrive::send(sector=0x%06lx, count=%d)", argument, count);
  //hexdump(buf, count);
#ifdef SDCARD
  for (size_t i = 0; i < count; i++)
  {
    SPI.transfer(buf[i]);
  }
#else
  hd_write_multiple(argument, buf);
#endif
}

// Read file data from disk.
uint8_t HdDrive::receive(uint8_t *buf, size_t count)
{
  //msgout("-->HdDrive::receive(sector=0x%06x)", argument);
#ifdef SDCARD
  for (size_t i = 0; i < count; i++)
  {
    buf[i] = SPI.transfer(0XFF);
  }
#else
  read_progress = 1;
  hd_read_multiple(argument, buf);
#endif
  //hexdump(buf, count);
  return 0;
}

// Save SPISettings for new max SCK frequency
void HdDrive::setSckSpeed(uint32_t maxSck)
{
#ifdef SDCARD
  m_spiSettings = SPISettings(maxSck, MSBFIRST, SPI_MODE0);
#endif
}

#endif // SPI_DRIVER_SELECT
