

#include <cmath>
#include <cstdint>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h> // for k_uptime_get()

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/logging\log.h>
#include <zephyr/types.h>

#include "PedalSensors.h"
#include <stdio.h>

// float PedalSensor::pedalPercentage() const
// {
//     float percent = ((float)analogRead(inputPin) - zeroPos) / (fullPos - zeroPos);
//     if (percent < 0.0f)
//     {
//         percent = 0.0f;
//     }
//     else if (percent > 1.0f)
//     {
//         percent = 1.0f;
//     }
//     return percent;
// }

// // Checks for a Short Circuit by reading the value of the sensor and comparing
// // it to the threshold defined above
// bool PedalSensor::checkOpenCircuit() const
// {
//     bool isOpenCircuit = analogRead(inputPin) < ocThreshold;
//     digitalWrite(ocLedPin, isOpenCircuit);
// #ifdef DEBUGGING
//     printf("Pedal %d: Open Circuit %s\n", inputPin, isOpenCircuit ? "detected" : "not detected");
// #endif
//     return isOpenCircuit;
// }

// // Checks for a Short Circuit by reading the value of the sensor and comparing
// // it to the threshold defined above
// bool PedalSensor::checkShortCircuit() const
// {
//     bool isShortCircuit = analogRead(inputPin) > scThreshold;
//     digitalWrite(scLedPin, isShortCircuit);
// #ifdef DEBUGGING
//     printf("Pedal %d: Short Circuit %s\n", inputPin, isShortCircuit ? "detected" : "not detected");
// #endif
//     return isShortCircuit;
// }

// // Checks both faults for a pedal
// bool PedalSensor::checkRangeFaults() const
// {
//     return checkOpenCircuit() || checkShortCircuit();
// }

// // if sensors disagree b y more than 10% for >100ms, fault.
// bool pedalAgreementFault(PedalSensor p1, PedalSensor p2)
// {
//     static uint32_t faultTime = 0;
//     if (abs(p1.pedalPercentage() - p2.pedalPercentage()) > 0.1)
//     { // maybe increase if p2 is being really screwy
//         if (faultTime == 0)
//         {
//             faultTime = k_uptime_get() + 100; // time when apps should start faulting
//         }
//         else if (faultTime <= k_uptime_get())
//         {
//             return true;
//         }
//     }
//     else
//     {
//         faultTime = 0;
//     }
//     return false;
// }

// bool brakeFault(PedalSensor p1, PedalSensor p2)
// {
//     static bool brakeFault = false;
//     float avgPedal = p1.pedalPercentage() + p2.pedalPercentage() / 2;
//     // float avgPedal = p2.pedalPercentage();
//     if (avgPedal > .25 && digitalRead(brake_switch_pin))
//     {
//         brakeFault = true;
//     }
//     if (brakeFault && avgPedal < .05)
//     {
//         brakeFault = false;
//     }
//     return brakeFault;
// }

// bool checkAllFaults(PedalSensor p1, PedalSensor p2)
// {
//     bool fault = false;
//     if (p1.checkRangeFaults() || p2.checkRangeFaults())
//     {
//         fault = true;
//     }
//     if (pedalAgreementFault(p1, p2))
//     {
//         fault = true;
// #ifdef DEBUGGING
//         Serial.println("FAULT: Pedal Agreement");
// #endif
//     }
//     if (brakeFault(p1, p2))
//     {
//         fault = true;
// #ifdef DEBUGGING
//         Serial.println("FAULT: Brake fault");
// #endif
//     }
//     return fault;
// }