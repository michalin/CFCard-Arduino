// Read and write raw data to and from any sector
#include "CFCard.h"

// Write the analog value A0 to a sector
bool raw_write(uint32_t sector = 2)
{
  struct raw_data
  {
    const char str[32] = "Analog Value A0: %d\n";
    uint16_t value;
  } data;

  if (!hd_init())
  {
    msgout("Error, could not find HD");
    return false;
  }
  data.value = analogRead(A0);
  if (hd_write_sector(sector, (const uint8_t *)&data))
  {
    msgout("Written to HD: ");
    msgout(data.str, data.value);
    return false;
  }
  else
  {
    msgout("Error writing to HD");
  }
  return true;
}

// Read sector previously written by raw_write and output on Serial
bool raw_read(uint32_t sector = 2)
{
  if (!hd_init())
  {
    msgout("Error, could not find HD");
    return false;
  }
  struct raw_data
  {
    const char str[32] = "";
    uint16_t value;
  } data;

  if (hd_read_sector(sector, (uint8_t *)&data, sizeof(data)))
  {
    msgout("Read from HD: ");
    msgout(data.str, data.value);
  }
  else
    msgout("Error reading from HD");
}


void setup()
{
  Serial.begin(115200);
  delay(100);
  msgout("WARNING: DON´T RUN THIS EXAMPLE IF THERE IS ANY IMPORTANT DATA ON YOUR DISK!");
  //Uncomment, if you really want to run this 
  //raw_write();
  //raw_read();
}
void loop()
{
}
