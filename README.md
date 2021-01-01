# CFCard-Arduino
Library for Arduino Mega that supports CF Cards and PATA drives.
With this library you can save and read data (raw data or files) on a Compact Flash card or PATA hard disk. 

### Installation
Copy the files to your project or Arduino library folder. Wire the Arduino and hard disk as shown below. For CF Cards, PATA adapters are available.
![Connection](wiring.jpg?raw=true "Wiring between Arduino Mega and PATA connector")

### Read and write raw data
Directly read or write sectors of a hard disk or CF Card. See example under [Examples/raw_io](Examples/raw_io/raw_io.ino)

### Read and write files
If the [SDFat library V2](https://github.com/greiman/SdFat.git) from Bill Greiman is installed, data carriers formatted with FAT16 / 32 can also be read or written. 
- Set #define SPI_DRIVER_SELECT 3 in SDFat/src/SdFatConfig.h
- Then uncomment "#define USE_FAT" in CFCard.h 
Most of the functions provided by the SDFat library should work with CF Cards and PATA drives as well.
See [Examples/file-io](Examples/file_io/file_io.ino)
- A downside of the SDFat library is however, that that it can only deal with volumes that have a master boot record (MBR). Compact Flash cards formatted with Windows don´t have an MBR, but Windows users need not despair. There is a free tool, called [Rufus](https://rufus.ie) available that can format the drive so that it is accepted by the SdFat lib.   
