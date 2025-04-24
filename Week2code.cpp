#include "mbed.h"
#include "arm_book_lib.h"
#define BLINKING_RATE 200ms
#define BLINKING_RATE2 500ms
#define LOCKOUT_TIME 60s

int main()
{
    DigitalIn enterButton(BUTTON1);
    DigitalIn gasDetector(D2);
    DigitalIn overTempDetector(D3);
    DigitalIn aButton(D4);
    DigitalIn bButton(D5);
    DigitalIn cButton(D6);
    DigitalIn dButton(D7);

    DigitalOut alarmLed(LED1);
    DigitalOut emergencyLed(LED3);
    DigitalOut systemBlockedLed(LED2);

    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
    aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);

    alarmLed = OFF;
    emergencyLed = OFF;
    systemBlockedLed = OFF;

    bool alarmState = OFF;
    int numberOfIncorrectCodes = 0;
    bool emergencyMode = false;
    bool lockout = false;
    bool emergencyBlinkState = false;
    bool lockoutBlinkState = false;

    Timer emergencyTimer;
    Timer lockoutBlinkTimer;
    Timer lockoutTimer;

    emergencyTimer.start();
    lockoutBlinkTimer.start();

    while (true) {
        if (gasDetector || overTempDetector) {
            alarmState = ON;
        }

        alarmLed = alarmState;

        if (gasDetector && overTempDetector) {
            emergencyMode = true;
        }

        if (emergencyMode && !lockout) {
            if (emergencyTimer.elapsed_time() >= BLINKING_RATE) {
                emergencyBlinkState = !emergencyBlinkState;
                emergencyLed = emergencyBlinkState;
                emergencyTimer.reset();
            }
        } else {
            emergencyLed = OFF;
        }

        if (!lockout && numberOfIncorrectCodes < 5) {
            if (aButton && bButton && cButton && dButton && !enterButton) {
                emergencyMode = false;
                emergencyLed = OFF;
            }
        }

        if (!lockout && enterButton.read() == 1 && emergencyMode) {
            if (aButton && bButton && !cButton && !dButton) {
                emergencyMode = false;
                emergencyLed = OFF;
                numberOfIncorrectCodes = 0;
                systemBlockedLed = OFF;
            } else {
                numberOfIncorrectCodes++;
                if (numberOfIncorrectCodes >= 5) {
                    lockout = true;
                    lockoutTimer.reset();
                    lockoutTimer.start();
                    lockoutBlinkTimer.reset();
                    lockoutBlinkTimer.start();
                } else {
                    systemBlockedLed = ON;
                    ThisThread::sleep_for(500ms);
                    systemBlockedLed = OFF;
                }
            }
            ThisThread::sleep_for(500ms);
        }

        if (lockout) {
            if (lockoutTimer.elapsed_time() >= LOCKOUT_TIME) {
                lockout = false;
                numberOfIncorrectCodes = 0;
                systemBlockedLed = OFF;
                lockoutTimer.stop();
                lockoutTimer.reset();
            } else {
                if (lockoutBlinkTimer.elapsed_time() >= BLINKING_RATE2) {
                    lockoutBlinkState = !lockoutBlinkState;
                    systemBlockedLed = lockoutBlinkState;
                    lockoutBlinkTimer.reset();
                }
            }
        }

        ThisThread::sleep_for(10ms);
    }
}