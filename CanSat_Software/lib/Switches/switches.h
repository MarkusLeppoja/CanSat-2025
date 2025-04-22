#ifndef __SWITCHES_H__
#define __SWITCHES_H__


#include <Arduino.h>
#include <defines.h>


bool Switch5VCMD, SwitchBATTCMD, isInit;
void toggleSwitch5V(bool state);
void toggleSwitchBATT(bool state);
float getSwitchBATTVoltage();
float getSwitch5VVoltage();
bool getSwitchBATTCMD();
bool getSwitch5VCMD();


void initSwitches()
{
    if (isInit) 
        return;
    isInit = true;
    pinMode(IO::GATE2_BATT_VOLTAGE, INPUT);
    pinMode(IO::GATE1_5V_VOLTAGE, INPUT);
    pinMode(IO::GATE2_BATT_COMMAND, OUTPUT);
    pinMode(IO::GATE1_5V_COMMAND, OUTPUT);
    digitalWrite(IO::GATE1_5V_COMMAND, LOW);
    digitalWrite(IO::GATE2_BATT_COMMAND, LOW);
}


void toggleSwitchBATT(bool state)
{
    if (SwitchBATTCMD == state) return;

    SwitchBATTCMD = state;
    
    // Log alert
    Serial.println("[A]Switch BATT state set to: " + String(state));

    // Toggle switch
    initSwitches();
    digitalWrite(IO::GATE2_BATT_COMMAND, state ? HIGH : LOW);
}


void toggleSwitch5V(bool state)
{
    if (SwitchBATTCMD == state) return;

    Switch5VCMD = state;

    // Log alert
    Serial.println("[A]Switch 5V state set to: " + String(state));

    // Toggle switch
    initSwitches();
    digitalWrite(IO::GATE1_5V_COMMAND, state ? HIGH : LOW);
}


float getSwitchBATTVoltage()
{
    return analogRead(IO::GATE2_BATT_VOLTAGE) * 3.3f / 4096.f * 10.f;
}


float getSwitch5VVoltage()
{
    return analogRead(IO::GATE1_5V_VOLTAGE) * 3.3f / 4096.f * 10.f;
}


bool getSwitchBATTCMD()
{
    return SwitchBATTCMD;
}


bool getSwitch5VCMD()
{
    return Switch5VCMD;
}
#endif