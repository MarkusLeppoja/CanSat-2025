#ifndef __SENSORS_H__
#define __SENSORS_H__


#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include "ICM42688.h"
#include "defines.h"


SPIClassRP2040 Inst_SPI(spi0, IO::SPI0_MISO, IO::SPI0_CS_BARO, IO::SPI0_SCK, IO::SPI0_MOSI);

// Sensors instances
// Both sensors are used in RP2040 HardwareSPI mode
Adafruit_BMP3XX Inst_Barometer;
ICM42688 Inst_IMU(Inst_SPI, IO::SPI0_CS_IMU);


// Misc variables
BARO BARO_DATA;
IMU IMU_DATA;
uint64_t millis_prev_sensors;
float dT;
bool sensorsInitStatus;


void initSensors()
{
    // Log alert
    Serial.println("[A] Beginning sensor initialisation.");

    // Barometer (BMP388)
    {
        if (!Inst_Barometer.begin_SPI(IO::SPI0_CS_BARO, &Inst_SPI))
        {
            Serial.println("[A] Barometer initialisation FAILED!");
            sensorsInitStatus = false;
            return;
        }


        // Set up oversampling and filter initialization
        Inst_Barometer.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
        Inst_Barometer.setPressureOversampling(BMP3_OVERSAMPLING_4X);
        Inst_Barometer.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_7);
        Inst_Barometer.setOutputDataRate(BMP3_ODR_200_HZ);

        Serial.println("[A] Barometer initialisation & configuration SUCCESS.");
    }




    // IMU (ICM42688)
    {
        if (Inst_IMU.begin() < 0)
        {
            Serial.println("[A] IMU initialisation FAILED!");
            sensorsInitStatus = false;
            return;
        }


        // Set up oversampling and filter initialization
        Inst_IMU.setGyroFS(Inst_IMU.dps2000);
        Inst_IMU.setAccelFS(Inst_IMU.gpm16);
        Inst_IMU.setAccelODR(Inst_IMU.odr200);
        Inst_IMU.setGyroODR(Inst_IMU.odr200);
        Inst_IMU.setFilters(true, true);


        Serial.println("[A] IMU initialisation & configuration SUCCESS.");
    }    


    sensorsInitStatus = true;
    Serial.println("[A] Sensors initialisation SUCCESS!");
}

void pollSensors()
{
    // Ensure consistent update rate
    float difference = millis() - millis_prev_sensors;
    if (difference < 80 || !sensorsInitStatus)
        return;
    millis_prev_sensors = millis();
    dT = difference / 1000.f;

    
    // Barometer
    {
        if (!Inst_Barometer.performReading())
        {
            debug("Failed to perform Barometer reading.");
            return;
        }        


        // Update variables
        BARO_DATA.pressure = Inst_Barometer.pressure / 100.f;
        BARO_DATA.temperature = Inst_Barometer.temperature;
        BARO_DATA.aglAltitude = Inst_Barometer.readAltitude(SEALEVELPRESSURE_HPA);

        // Calculate barometric velocity based on altitude delta (defenatly not the best option to do this)
        BARO_DATA.barometricVelocity = float(BARO_DATA.aglAltitude - BARO_DATA.barometricAltitudePrev) / dT;
        BARO_DATA.barometricAltitudePrev = BARO_DATA.aglAltitude;


        // Calculate altitude based on a setpoint
        BARO_DATA.deltaAltitude = float(BARO_DATA.aglAltitude - BARO_DATA.deltaAltitudeSetpoint);
    }


    // IMU
    {
        if (Inst_IMU.getAGT() < 0)
        {
            debug("Failed to perform IMU reading.");
            return;
        }
        // Update variables
        IMU_DATA.accX = Inst_IMU.accX();
        IMU_DATA.accY = Inst_IMU.accY();
        IMU_DATA.accZ = Inst_IMU.accZ();

        IMU_DATA.gyrX = Inst_IMU.gyrX();
        IMU_DATA.gyrY = Inst_IMU.gyrY();
        IMU_DATA.gyrZ = Inst_IMU.gyrZ();

        IMU_DATA.temperature = Inst_IMU.temp();
    }
}
#endif