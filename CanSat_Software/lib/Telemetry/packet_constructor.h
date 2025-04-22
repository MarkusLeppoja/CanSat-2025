#ifndef __PACKET_CONSTRUCTOR__
#define __PACKET_CONSTRUCTOR__


#include <Arduino.h>
#include <sensors.h>
#include <switches.h>

/*  Telemetry Arcidecture

    Station Packages: 
        - Identification code: (0xA)
        - Packet starts and ends with the Identification code to ensure it has been fully received.
    Package types: 
        - Toggle switches
        - Read and return switch voltage
        - Read and return ADC voltage
        - Set delta altitude setpoint to current altitude
        - Set 0x01 Package stream   | STATUS
        - Set 0x02 Package stream   | STATUS
        - Set Software State        | STATE NR
    Example: (BASE ID), (CMD NR), (EXTRA PARAMETER)
    Example: 0xA, 0x48, 8, 0xA

    Telemetry:
        Time (Milliseconds)
        AGL Altitude (m)
        Delta Altitude (m)
        Barometric Velocity
        Air Pressure (hPa)
        Temperature (*C)
        Acceleration (x,y,z) (m/s/s)
        Angular Velocity (x,y,z) (deg/s)
        Switch 1 CMD & Voltage
        Switch 2 CMD & Voltage
        Batt Voltage
        Batt Current
        Secondary Mission ADC Pings
*/


void addDataToPacket(String *packet, String data, bool comma);
bool isPacketOversized(String packet);
String getMSTPacket();
String getLSTPacket();
String getStatusPacket();
String getAllTelemetryPacket();


String getMSTPacket()
{
    String MST_Packet;  // Packet instance

    // Base telemetry
    addDataToPacket(&MST_Packet, "[T,1]", false);
    addDataToPacket(&MST_Packet, String(millis() / 1000.f), true);
    addDataToPacket(&MST_Packet, String(CanSatState), true); 

    // Most significant Telemetry
    addDataToPacket(&MST_Packet, String(BARO_DATA.deltaAltitude), true);
    addDataToPacket(&MST_Packet, String(BARO_DATA.pressure), true);
    addDataToPacket(&MST_Packet, String(BARO_DATA.temperature), false);

    // Return packet provided it is not oversized
    return isPacketOversized(MST_Packet) ? "OVERSIZED" : MST_Packet;
}


String getLSTPacket()
{
    String LST_Packet;  // Packet instance

    // Base telemetry
    addDataToPacket(&LST_Packet, "[T,2]", false);
    addDataToPacket(&LST_Packet, String(millis() / 1000.f), true);
    addDataToPacket(&LST_Packet, String(CanSatState), true); 

    // Most significant Telemetry
    addDataToPacket(&LST_Packet, String(BARO_DATA.deltaAltitude), true);
    addDataToPacket(&LST_Packet, String(IMU_DATA.accX), true);
    addDataToPacket(&LST_Packet, String(IMU_DATA.accY), true);
    addDataToPacket(&LST_Packet, String(IMU_DATA.accZ), true);
    addDataToPacket(&LST_Packet, String(IMU_DATA.gyrX), true);
    addDataToPacket(&LST_Packet, String(IMU_DATA.gyrY), true);
    addDataToPacket(&LST_Packet, String(IMU_DATA.gyrZ), false);

    // Return packet provided it is not oversized
    return isPacketOversized(LST_Packet) ? "OVERSIZED" : LST_Packet;
}


String getStatusPacket()
{
    String Status_Packet;  // Packet instance

    // Base telemetry
    addDataToPacket(&Status_Packet, "[T,3]", false);
    addDataToPacket(&Status_Packet, String(getSwitch5VCMD()), true);
    addDataToPacket(&Status_Packet, String(getSwitch5VVoltage()), true);
    addDataToPacket(&Status_Packet, String(getSwitchBATTCMD()), true);
    addDataToPacket(&Status_Packet, String(getSwitchBATTVoltage()), true);
    addDataToPacket(&Status_Packet, String(NODEID), true);
    addDataToPacket(&Status_Packet, String(NODEID_RECEIVER), true);


    /*
        Batt Voltage
        Batt Current
        */
    // Return packet provided it is not oversized
    return isPacketOversized(Status_Packet) ? "OVERSIZED" : Status_Packet;
}

String getAllTelemetryPacket()
{
    String AllTelemetry_Packet;  // Packet instance

    // Base telemetry
    addDataToPacket(&AllTelemetry_Packet, String(millis() / 1000.f), true);
    addDataToPacket(&AllTelemetry_Packet, String(CanSatState), true);

    // Most significant Telemetry
    addDataToPacket(&AllTelemetry_Packet, String(BARO_DATA.aglAltitude), true);
    addDataToPacket(&AllTelemetry_Packet, String(BARO_DATA.deltaAltitude), true);
    addDataToPacket(&AllTelemetry_Packet, String(BARO_DATA.barometricVelocity), true);
    addDataToPacket(&AllTelemetry_Packet, String(BARO_DATA.pressure), true);
    addDataToPacket(&AllTelemetry_Packet, String(BARO_DATA.temperature), true);
    addDataToPacket(&AllTelemetry_Packet, String(IMU_DATA.accX), true);
    addDataToPacket(&AllTelemetry_Packet, String(IMU_DATA.accY), true);
    addDataToPacket(&AllTelemetry_Packet, String(IMU_DATA.accZ), true);
    addDataToPacket(&AllTelemetry_Packet, String(IMU_DATA.gyrX), true);
    addDataToPacket(&AllTelemetry_Packet, String(IMU_DATA.gyrY), true);
    addDataToPacket(&AllTelemetry_Packet, String(IMU_DATA.gyrZ), true);
    
    // Switch data 
    addDataToPacket(&AllTelemetry_Packet, String(getSwitch5VCMD()), true);
    addDataToPacket(&AllTelemetry_Packet, String(getSwitch5VVoltage()), true);
    addDataToPacket(&AllTelemetry_Packet, String(getSwitchBATTCMD()), true);
    addDataToPacket(&AllTelemetry_Packet, String(getSwitchBATTVoltage()), false);

    // Return packet provided 
    return AllTelemetry_Packet;
}


bool isPacketOversized(String packet)
{
    if (packet.length() > 61) {
        debug("[A]Packet size over 61 bytes: " + packet);
        return true;
    }
    return false;
}


void addDataToPacket(String *packet, String data, bool comma)
{
    packet->concat(data);
    if (comma)
        packet->concat(',');
}
#endif