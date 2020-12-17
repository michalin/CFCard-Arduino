#include "Arduino.h"
#include "CFCard.h"

#ifdef USE_FAT
#include "SdFat.h"
#endif

#if SPI_DRIVER_SELECT == 3 // Must be set in SdFat/SdFatConfig.h
#ifndef HDDriver_h
#define HDDriver_h

//#define SDCARD

#ifdef SDCARD
#include "SPI.h" // Only required if you use features in the SPI library.
#endif

// SD chip select pin.
#define SD_CS_PIN SS

typedef class HdDrive : public SdSpiBaseClass
{
public:
  void activate();
  void begin(SdSpiConfig config);
  void deactivate();
  // Receive a byte.
  uint8_t receive();
  // Receive multiple bytes.
  // Replace this function if your board has multiple byte receive.
  uint8_t receive(uint8_t *buf, size_t count);
  // Send a byte.
  void send(uint8_t data);
  // Send multiple bytes.
  // Replace this function if your board has multiple byte send.
  void send(const uint8_t *buf, size_t count);
  // Save SPISettings for new max SCK frequency
  void setSckSpeed(uint32_t maxSck);

private:
  //Command responses: First byte response length
  #ifdef SDCARD
  SPISettings m_spiSettings;
  #endif
  uint8_t command; 
  uint32_t argument; 
  bool read_progress = false; //True: (multiple) Read in progress
  uint32_t sect_written; // Number of sectors written 
  bool write_progress = false; //True: (multiple) Write in progress

  //uint8_t cmd_send_response(uint8_t, uint32_t);
  uint8_t cmd_respond(uint8_t &, const uint8_t *);
  const uint8_t cmd0_response[4] = {3, 0xff, R1_IDLE_STATE, 0xff};
  const uint8_t cmd0_response_error[4] = {3, 0xff, 0x00, 0xff};
  const uint8_t cmd8_response[8] = {7, 0xff, R1_IDLE_STATE, 0x00, 0x00, 0x01, 0xaa, 0xff};
  const uint8_t cmd12_response[4] = {3, 0x7f, R1_READY_STATE, 0xff};
  const uint8_t cmd18_single_response[9] = {8, 0xff, R1_READY_STATE, 0xff, 0xff, 0xfe, 0xab, 0xcd, 0xff}; //R1 Resonse - read token (0xfe) - Dummy Checksum (0xabcd) 
  const uint8_t cmd18_multi_response[6] = {5, 0xff, 0xfe, 0x12, 0x34, 0xff};
  const uint8_t cmd25_response_start[4] = {3, 0xff, R1_READY_STATE, 0xff};
  const uint8_t cmd25_response_next[4] = {3, 0xe5, 0x03, 0xff};
  const uint8_t cmd25_response_multi_end[4] = {3, 0x80, 0x03, 0xff};
  const uint8_t cmd25_response_single_end[3] = {2, 0x83, 0xff};
  const uint8_t *cmd55_response = cmd0_response;
  const uint8_t acmd41_response[4] = {3, 0xff, R1_READY_STATE, 0xff};
  const uint8_t cmd58_response[8] = {7, 0xff, R1_READY_STATE, 0xc0, 0xff, 0x80, 0x00, 0xff}; //OCR Register
} HdDriver;


#endif
#endif