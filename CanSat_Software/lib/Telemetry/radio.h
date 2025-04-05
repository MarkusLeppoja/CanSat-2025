#ifndef __RADIO__
#define __RADIO__

#include <Arduino.h>
#include <defines.h>
#include <RFM69_ATC.h>
#include <SPI.h>
#include <flash.h>

#define NETWORKID     100                   //the same on all nodes that talk to each other
#define ENCRYPTKEY    "Wdymkasseesobib?"    //exactly the same 16 characters/bytes on all nodes!


RFM69_ATC radio(IO::SPI1_CS_RADIO, IO::SERVO, true, &SPI1); // Initialise radio lib using Automatic Transmission Control


// Function declarations
void checkForTelemetryPackages();
void transmitRadio(String packet);
void packetFiltering(String packet);
void initRadio();



bool radioInitStatus;
void initRadio()
{
    // Configure custom SPI usage
    SPI1.setMOSI(11);
    SPI1.setMISO(12);
    SPI1.setSCK(10);
    SPI1.begin(13);
    

    // Begin radio
    uint8_t status = radio.initialize(RF69_433MHZ,NODEID,NETWORKID);

    if (status != 1)
    {
        Serial.println("[A] Radio initialisation FAILED! Code: " + String(status));
        radioInitStatus = false;
        return;
    }

    //Auto Transmission Control - dials down transmit power to save battery
    radio.enableAutoPower(-110);
    
    // Configure for RFM69HCW
    radio.setHighPower();
    radio.set300KBPS();
    radio.encrypt(ENCRYPTKEY);

    // Receiver paramteres
    radio.spyMode(false);
    radio.receiveDone(); //put radio in RX mode
    

    Serial.printf("[A] Radio initialisation SUCCESS!\n\r");
    Serial.printf("[A] Operating at %d Mhz...\r\n", radio.getFrequency() / 1000000);
    Serial.printf("[A] MY ID: %d Receiver ID: %d\n\r", NODEID, NODEID_RECEIVER);
    Serial.flush();

    radioInitStatus = true;
}


void transmitRadio(String packet)
{
    if (!radioInitStatus)
        return;

    radio.sendWithRetry(NODEID_RECEIVER, packet.c_str(), packet.length(), 1);  // Transmit package with 1 retry
    debug("Transmitted packet (" + String(packet.length()) + "): " + packet.c_str());
    radio.receiveDone();
}


uint64_t prev;
void checkForTelemetryPackages()
{
    if (millis() - prev < 150 || !radioInitStatus)
        return;
    prev = millis();

    if (radio.receiveDone())
    {
        //print message received to serial
        Serial.print("[ID:");Serial.print(radio.SENDERID);Serial.print("]");
        Serial.print("[RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
        Serial.println((char*)radio.DATA);

        #ifndef IS_GROUND_STATION
        packetFiltering((char*)radio.DATA);
        #endif
    }
        
    //check if sender wanted an ACK
    if (radio.ACKRequested())
    {
        radio.sendACK();
        debug("ACK sent");
    }
    radio.receiveDone(); //put radio in RX mode
}

/* Examples:
    [A]Board configured in GROUND STATION MODE
    [R,A]CanSat configured for test
    [T,1]11.04,5,25.42,1004.25,10.5
    [C,F]
*/
void packetFiltering(String packet)
{
    // Find the location index of [] if not present then return
    uint8_t firstBraketIndex, secondBraketIndex;
    firstBraketIndex = packet.indexOf('[');
    secondBraketIndex = packet.indexOf(']', firstBraketIndex);
    if (firstBraketIndex == -1 || secondBraketIndex == -1) return;


    // Get the packet payload
    String payload;
    payload = packet.substring(firstBraketIndex+1, secondBraketIndex);
    if (payload.length() < 1) return;
    
    
    // Get packet type and extra arguments
    String ID, designator;
    uint8_t commaIndex; 

    commaIndex = payload.indexOf(',');
    if (commaIndex > -1)
    {
        designator = payload.substring(commaIndex+1);
    }
    ID = payload.substring(0, commaIndex);


    // If designator is present find it
    debug("ID: " + ID);
    debug("Designator: " + designator);

    if (ID == "C")
    {
        switch (designator.charAt(0))
        {
            /* TOGGLE TELEMETRY STREAM */
            case '1':
                telemetryMSTStream = true;
                transmitRadio("[R,1]MST telemetry stream: ENABLED");
            break;

            case '2':
                telemetryMSTStream = false;
                transmitRadio("[R,2]MST telemetry stream: DISABLED");
            break;

            case '3':
            CanSatState = IDLE;

            toggleBuzzer = false;
            toggleSwitchBATT(false);
            toggleSwitch5V(false);

            telemetryMSTFrequency = 1;

            transmitRadio("[R,3]State set to IDLE");
            break;

            case '4':
            CanSatState = FLIGHT;
            toggleBuzzer = true;
            toggleSwitch5V(true);
            
            // Setup data logging
            allTelemetryRecording = true;
            createAllTelemetryFile();

            telemetryMSTFrequency = 4;
            telemetryMSTStream = true;

            BARO_DATA.deltaAltitudeSetpoint = BARO_DATA.aglAltitude;

            transmitRadio("[R,4]State set to FLIGHT");
            break;

            case '5':
            CanSatState = LANDED;
            toggleBuzzer = false;
            toggleSwitchBATT(false);
            toggleSwitch5V(false);

            closeFile();
            telemetryMSTFrequency = 1;
            allTelemetryRecording = true;

            transmitRadio("[R,5]State set to LANDED");
            break;


            /* TOGGLE BUZZER AND CAMERA */
            case '6':
                toggleBuzzer = true;
                transmitRadio("[R,6]Enable indicator buzzer beep");
            break;

            case '7':
                toggleBuzzer = false;
                toggleSwitchBATT(false);
                transmitRadio("[R,7]Disable indicator buzzer beep");
            break;

            case '8':
                toggleSwitch5V(true);
                transmitRadio("[R,8]Enable camera");
            break;

            case '9':
                toggleSwitch5V(false);
                transmitRadio("[R,9]Disable camera");
            break;

            case 'A':
                BARO_DATA.deltaAltitudeSetpoint = BARO_DATA.aglAltitude;
                transmitRadio("[R,A]Delta altitude setpoint configured");
            break;

            case 'B':
                telemetryMSTFrequency = 4;
                transmitRadio("[R,B]MST telemetry frequency set to 4Hz");
            break;


            case 'C':
                telemetryMSTFrequency = 1;
                transmitRadio("[R,C]MST telemetry frequency set to 1Hz");
            break;

            case 'D':
                readFileAndPrint(TELEMETRY_FILE_NAME, 5);
                transmitRadio("[R,D]Printing all file data");
            break;

            case 'E':
                clearFlash();
                transmitRadio("[R,E]Clearing flash");
            break;

        default:
            break;
        }
    }
}
#endif