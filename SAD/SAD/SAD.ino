// Core library for code-sense - IDE-based
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(LITTLEROBOTFRIENDS) // LittleRobotFriends specific
#include "LRF.h"
#elif defined(MICRODUINO) // Microduino specific
#include "Arduino.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(SPARK) // Spark specific
#include "application.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE


#include <StandardCplusplus.h>
#include <vector>

#include <SoftwareSerial.h>
#include <TimedAction.h>
#include "CommStation.h"
#include "Motor.h"
#include "Rangefinder.h"
#include "Packet.h"
#include "Magnetometer.h"
#include <Wire.h>
#include <HMC5883L.h>


using namespace std;

// Bluetooth LE (KEDSUM)
#define BT_TX_PIN 12
#define BT_RX_PIN 11
#define BAUD_RATE 9600

// Right motor
#define H_ENABLE_PIN_1   3
#define H_CONTROL_PIN_1A 2
#define H_CONTROL_PIN_1B 1

// Left motor
#define H_ENABLE_PIN_2   6
#define H_CONTROL_PIN_2A 4
#define H_CONTROL_PIN_2B 5

// Rangefinders (SunFounder HC-SR04)
#define FRONT_RF_TRIG_PIN A0
#define FRONT_RF_ECHO_PIN A1
#define RIGHT_RF_TRIG_PIN A2
#define RIGHT_RF_ECHO_PIN A3
#define LEFT_RF_TRIG_PIN  A4
#define LEFT_RF_ECHO_PIN  A5

CommStation* comm;
Motor* rMotor;
Motor* lMotor;
Rangefinder* fRangefinder;
Rangefinder* rRangefinder;
Rangefinder* lRangefinder;
Magnetometer* magnetometer;

void brake() {
    rMotor->stop();
    lMotor->stop();
}

void moveForward() {
    rMotor->accelerate();
    lMotor->accelerate();
}

void moveBackward() {
    rMotor->reverse();
    lMotor->reverse();
}

void turnRight() {
    rMotor->reverse();
    lMotor->accelerate();
}

void turnLeft() {
    rMotor->accelerate();
    lMotor->reverse();
}


Packet scan() {
    int totalScans = 10; // 1ms total time
    vector<Range> RFData;
    
    RFData.push_back(fRangefinder->avgPing(totalScans));
    //RFData.push_back(lRangefinder->avgPing(totalScans));
    //RFData.push_back(rRangefinder->avgPing(totalScans));
    
    int heading = magnetometer->getHeading();
    
    return Packet(heading, RFData);
}


void bgScan() {
    Packet p = scan();
    
    if (!comm->isBusy()) {
        comm->sendPacket(p);
    }
    
}

void setup() {
    // Establish connection with base
    SoftwareSerial* bluetoothSerial;
    bluetoothSerial = new SoftwareSerial(BT_TX_PIN, BT_RX_PIN);
    bluetoothSerial->begin(BAUD_RATE);
    
    comm = new CommStation(bluetoothSerial);
    
    int initialSpeed = 255;
    // Ready motors
    rMotor = new Motor(H_ENABLE_PIN_1, H_CONTROL_PIN_1A, H_CONTROL_PIN_1B);
    rMotor->setSpeed(initialSpeed);
    
    lMotor = new Motor(H_ENABLE_PIN_2, H_CONTROL_PIN_2A, H_CONTROL_PIN_2B);
    lMotor->setSpeed(initialSpeed);
    
    // Initialize rangefinders
    fRangefinder = new Rangefinder(FRONT_RF_TRIG_PIN, FRONT_RF_ECHO_PIN, 90);
    //rRangefinder = new Rangefinder(RIGHT_RF_TRIG_PIN, RIGHT_RF_ECHO_PIN, 180);
    //lRangefinder = new Rangefinder(LEFT_RF_TRIG_PIN, LEFT_RF_ECHO_PIN, 0);
    
    magnetometer = new Magnetometer();
}

TimedAction scanAction = TimedAction(100, bgScan);

void loop() {
    if (comm->isAvailable()) {
        // Execute any available action
        cmdFuncPtr cmd = comm->getCmd();
        if (cmd != NULL) {
            cmd();
        }
        
        scanAction.check();
    }
    
}
