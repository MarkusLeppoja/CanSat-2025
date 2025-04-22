#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <Arduino.h>




// Configuration
//#define IS_GROUND_STATION           // Uncomment to set it to CanSat mode
//#define DEBUG                       // Uncomment to set it to RELEASE mode




/* RADIO START */
extern bool telemetryMSTStream;
extern bool telemetryLSTStream;
extern uint8_t telemetryMSTFrequency;
extern uint8_t telemetryLSTFrequency;

bool telemetryMSTStream = true;
bool telemetryLSTStream = false;;
uint8_t telemetryMSTFrequency = 1;
uint8_t telemetryLSTFrequency = 1;

#ifdef IS_GROUND_STATION
#define NODEID 0x2F
#define NODEID_RECEIVER 0x1F
#else
#define NODEID 0x1F
#define NODEID_RECEIVER 0x2F
#endif
/* RADIO END */




/* FLASH & DATA RECORDING START */
extern bool allTelemetryRecording;
extern uint8_t allTelemetryRecordingFrequency;

bool allTelemetryRecording = false;
uint8_t allTelemetryRecordingFrequency = 10;

#define MAX_TELEMETRY_FILES 10
/* FLASH & DATA RECORDING END */




/* SECONDARY MISSIONS START */
extern bool toggleBuzzer;
bool toggleBuzzer = false;
/* SECONDARY MISSIONS END */




/* CANSAT SOFTWARE VARIABLES START */
enum CanSatState_t : uint8_t
{
    IDLE = 0,
    FLIGHT = 1,
    LANDED = 2,
    ERROR = 3
};
extern CanSatState_t CanSatState;
CanSatState_t CanSatState;

enum IO : uint8_t
{
    SPI0_CS_BARO        =   0,
    SPI0_CS_IMU         =   1,
    SPI0_SCK            =   2,
    SPI0_MOSI           =   3,
    SPI0_MISO           =   4,
    SERVO               =   5,
    I2C0_SDA            =   6,
    I2C0_SCL            =   7,
    SPI1_SCK            =   10,
    SPI1_MOSI           =   11,
    SPI1_MISO           =   12,
    SPI1_CS_RADIO       =   13,
    SPI1_CS_FLASH       =   14,
    GATE1_5V_VOLTAGE    =   26,
    GATE2_BATT_VOLTAGE  =   27,
    GATE2_BATT_COMMAND  =   28,
    GATE1_5V_COMMAND    =   29
};
/* CANSAT SOFTWARE VARIABLES END */




/* SENSORS START */
struct BARO 
{
    float pressure;             // Air pressure (hPa)
    float temperature;          // Air temperature (*C)
    float aglAltitude;          // Above Ground Level Altitude. (m)
    float deltaAltitude;        // Altitude difference between setpoint. (m)
    float barometricVelocity;   // Velocity calculated based on barometric altitude difference. (m/s)
    
    // Other variables
    float deltaAltitudeSetpoint;
    float barometricAltitudePrev;
};
extern BARO BARO_DATA;

struct IMU 
{
    float accX, accY, accZ;     // Acceleration x,y,z (m/s/s)
    float gyrX, gyrY, gyrZ;     // Angular velocity x,y,z (deg/s)
    float temperature;          // Air temperature (*C)
};
extern IMU IMU_DATA;
/* SENSORS END */




#define ADS1115_I2C_ADDRESS         0x48
#define INA139_CURRENT_GAIN         10
#define MAIN_BUS_VOLTAGE_GAIN       10
#define SWITCH_BUS_VOLTAGE_GAIN     10
#define SEALEVELPRESSURE_HPA        (1013.25)

void debug(String statement)
{
    #ifdef DEBUG
    Serial.println(statement);
    Serial.flush();
    #endif
}

#endif