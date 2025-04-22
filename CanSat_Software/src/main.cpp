#include <Arduino.h>
#include <defines.h>
#include <sensors.h> 
#include <packet_constructor.h>
#include <radio.h>
#include <flash.h>


void logTelemetry();
void captureSerialInput();
void updateBuzzer();


void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.setTimeout(0);
    initRadio();

    #ifdef IS_GROUND_STATION
    Serial.println("Board configured in GROUND STATION MODE");
    #else
    Serial.println("Board configured in CANSAT MODE");
    initSwitches();
    initSensors();
    initFlash();

    // Setup data logging
    allTelemetryRecording = true;
    createAllTelemetryFile();
    #endif

    CanSatState == IDLE;
}

uint64_t mainLoopMillisPrev;
void loop()
{   
    uint64_t differenceMillis = millis() - mainLoopMillisPrev;

    if (differenceMillis >= 20)
    {
        mainLoopMillisPrev = millis();

        // Ground station mode
        #ifdef IS_GROUND_STATION
        captureSerialInput();
        #endif

        // CanSat mode
        #ifndef IS_GROUND_STATION
        switch (CanSatState)
        {
            case IDLE:
                updateBuzzer();    
                captureSerialInput();
                break;
            case FLIGHT:
                updateBuzzer();
                break;
            case LANDED:
                captureSerialInput();
                break;
            case ERROR:
                break;
        }
        pollSensors();
        logTelemetry();
        #endif

    }
    checkForTelemetryPackages();
}


// Create and transmit different types of telemetry packages based on their configured specifications
uint64_t millis_tlm_MST_prev, millis_tlm_LST_prev, millis_tlm_all_prev;
void logTelemetry()
{
    float durationMST = millis() - millis_tlm_MST_prev;
    float durationLST = millis() - millis_tlm_LST_prev;
    float durationAll = millis() - millis_tlm_all_prev;

    // Most significant telemetry streaming logic
    if (durationMST >= (1000.f / telemetryMSTFrequency) && telemetryMSTStream)
    {
        millis_tlm_MST_prev = millis();
        String packetMST = getMSTPacket();

        transmitRadio(packetMST);
        debug(packetMST);
    }

    #ifdef LEGACY
    // Least significant telemetry streaming logic
    if (durationLST >= (1000.f / telemetryLSTFrequency) && telemetryLSTStream)
    {
        millis_tlm_LST_prev = millis();
        String packetLST = getLSTPacket();

        transmitRadio(packetLST);
        debug(packetLST);
    }
    #endif

    // All telemetry recording logic
    if (allTelemetryRecording && durationAll >= (1000.f / allTelemetryRecordingFrequency))
    {
        millis_tlm_all_prev = millis();
        String packetAll = getAllTelemetryPacket();

        // Write to file
        writeToFile(packetAll.c_str());
        debug(packetAll);
    }
}


String incData;
void captureSerialInput()
{
    while(Serial.available())
    {
        incData.concat((char)Serial.read());   
    }
    if (incData.length() > 0)
    {
        #ifndef IS_GROUND_STATION
        packetFiltering(incData);
        #endif

        transmitRadio(incData);
        incData = "";
    }
}


uint64_t millis_buzzer_prev;
void updateBuzzer()
{
    if (!toggleBuzzer) return;

    float delta = millis() - millis_buzzer_prev;

    if (delta >= 800 && delta < 1000)
    {
        toggleSwitchBATT(true);
    }
    else if (delta >= 1000)
    {
        millis_buzzer_prev = millis();
        toggleSwitchBATT(false);
    }
}

/*  Sections:
        Power    (Current & Voltage monitoring + extra IO monitoring) ADS1115
        Telemetry - (flash) - (radio)
        COMS? or states
*/
