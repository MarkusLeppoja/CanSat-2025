#ifndef __FLASH_H__
#define __FLASH_H__

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_FlashTransport.h>
#include <Adafruit_SPIFlash.h>
#include <SdFat.h>
#include <defines.h>


SPIClassRP2040 spi_flash_instance(spi1, IO::SPI1_MISO, IO::SPI1_CS_FLASH, IO::SPI1_SCK, IO::SPI1_MOSI);
Adafruit_FlashTransport_SPI flashTransport(14, &spi_flash_instance);
Adafruit_SPIFlash flash(&flashTransport);


FatVolume fatfs;
File32 allTelemetryFile; 
bool init_flash = false;


// All functions
bool initFlash();
bool doesFileExist(const char* filename);
bool createFile(const char* filename);
bool openFileForRead(const char* filename);
void listFiles();
void readFileAndPrint(const char* filename, unsigned long delayTime);
bool closeFile();
bool writeToFile(const char* data);
bool clearFlash();


bool initFlash()
{
    spi_flash_instance.begin();

    // Initialise flash
    if (!flash.begin())
    {
        Serial.println("[A] Flash initialisation FAILED!");
        init_flash = false;
        return false;
    }

    Serial.println("Flash chip JEDEC ID 0x" + String(flash.getJEDECID(), HEX));
    Serial.println("Flash chip size: " + String(flash.size() / 1024) + "KB");
    
    // Initialise filesystem
    if (!fatfs.begin(&flash))
    {
        Serial.println("[A] File system initialisation FAILED!");
        init_flash = false;
        return false;
    }

    if (doesFileExist(TELEMETRY_FILE_NAME))
    {
        Serial.println("[A] Previous TELEMETRY FILE EXISTS. Please delete it before recording or it will be overwritten");
    }


    init_flash = true;
    Serial.println("[A] Flash initialisation SUCCESS!");
    return true;
}


bool doesFileExist(const char* filename)
{
    return fatfs.exists(filename);
}

bool createFile(const char* filename)
{
    if (doesFileExist(filename))
    {
        debug("[A]File already exists!");
        return false;
    }

    allTelemetryFile = fatfs.open(filename, FILE_WRITE);
    if (!allTelemetryFile)
    {
        debug("[A]Failed to create file!");
        return false;
    }

    debug("[A]File created successfully!");
    return true;
}

bool openFileForRead(const char* filename)
{
    allTelemetryFile = fatfs.open(filename, FILE_READ);
    if (!allTelemetryFile)
    {
        debug("[A]Failed to open file!");
        return false;
    }

    debug("[A]File opened successfully!");
    return true;
}

void listFiles()
{
    File32 root = fatfs.open("/");
    File32 file = root.openNextFile();
    while (file)
    {
        Serial.print("File: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
}

void readFileAndPrint(const char* filename, unsigned long delayTime)
{
    if (!openFileForRead(filename))
        return;

    while (allTelemetryFile.available())
    {
        String line = allTelemetryFile.readStringUntil('\n');
        Serial.println(line);
        delay(delayTime);
    }

    closeFile();
}

bool closeFile()
{
    if (!allTelemetryFile)
    {
        debug("[A]File not open!");
        return false;
    }

    allTelemetryFile.close();
    debug("[A]File closed successfully!");
    return true;
}

void createAllTelemetryFile()
{
    clearFlash();
    createFile(TELEMETRY_FILE_NAME);
    writeToFile("Time s, Vehicle State, AGL Altitude m, Delta Altitude m, Barometric Velocity m/s, Pressure hPa, Temperature C, Acceleration x m/s2, Acceleration y m/s2), Acceleration z m/s2, Angular Velocity x dps, Angular Velocity y dps, Angular Velocity z dps, V5 Switch CMD, V5 Switch Voltage, BATT Switch CMD, BATT Switch Voltage");       
}


bool writeToFile(const char* data)
{
    if (!allTelemetryFile)
    {
        debug("[A]File not open!");
        return false;
    }

    allTelemetryFile.println(data);
    allTelemetryFile.flush(); // Ensure data is written to flash

    debug("[A]Data written to file successfully!");
    return true;
}

bool clearFlash()
{
    if (!init_flash)
    {
        debug("[A]Flash not initialised!");
        return false;
    }

    if (!fatfs.remove(TELEMETRY_FILE_NAME))
    {
        debug("[A]Failed to clear flash!");
        return false;
    }


    debug("[A]File deleted!");
    return true;
}

// Use https://github.com/MarkusLeppoja/Neutron-R3 github repositry and write me code for flash card that has all the nessecary functions like: 
/* 
- creating file with random name
- writing to file
- reading all data from file and serial printing it with a delay
*/
#endif