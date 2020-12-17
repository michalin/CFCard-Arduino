/*  To run this example, you need to install the SDFat Library V2 from Bill Greiman 
 *  https://github.com/greiman/SdFat.git or Arduino Library Manager
 *  Set #define SPI_DRIVER_SELECT 3 in SDFat/src/SdFatConfig.h
 *  Then uncomment "#define USE_FAT" in CFCard.h 
 */

#include "CFCard.h"

HdDriver hdd;
#define HDCONFIG SdSpiConfig(0, DEDICATED_SPI, 0, &hdd)


// Log analog values from A0 into a file
bool log_to_file(const char *filename = "values.csv")
{
  SdFat32 hd;
  if (!hd.begin(HDCONFIG))
  {
    printSdErrorText(&Serial, hd.card()->errorCode());
    return 0;
  }
  msgout("HD successfully initialized");

  

  File32 f_table = hd.open(filename, O_CREAT + O_RDWR + O_APPEND);
  if (!f_table)
  {
    //printSdErrorText(&Serial, hd.card()->errorCode());
    return 0;
  }
  msgout("File open for writing, start measuring");

  f_table.write("Number\tValue\n", 13); //Headline
  for (int i = 0; i < 10; i++)
  {
    const char *line = "%d\t%d\n";
    char buffer[16];
    uint8_t strlen = sprintf(buffer, line, i, analogRead(A0));
    Serial.print(buffer);
    if(!f_table.write(buffer, strlen))
    {
      printSdErrorText(&Serial, hd.card()->errorCode());
      return 0;
    }
    f_table.flush();
    delay(1000);
  }
  f_table.close();
  return 1;
}

// Read whatever file and output on Serial.
bool read_file(const char *filename = "values.csv")
{
  SdFat32 hd;
  if (!hd.begin(HDCONFIG))
  {
    printSdErrorText(&Serial, hd.card()->errorCode());
    return 0;
  }
  msgout("HD successfully initialized");

  File32 f_table = hd.open(filename);
  if(!f_table)
  {
    printSdErrorText(&Serial, hd.card()->errorCode());
    return 0;    
  }
  msgout("File open for reading");

  for (size_t i = 0; i < f_table.size(); i++)
  {
    Serial.print((char)f_table.read());
  }
  f_table.close();
  return 1;
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  log_to_file();
  read_file();
}
void loop()
{
}
