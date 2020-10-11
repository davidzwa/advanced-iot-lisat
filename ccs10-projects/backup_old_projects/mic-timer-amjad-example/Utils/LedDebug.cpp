/*
 * LedDebug.cpp
 *
 *  Created on: Oct 10, 2020
 *      Author: david
 */

#include "common.h"
#include <Utils/LedDebug.h>

LedDebug::LedDebug()
{
    // TODO Auto-generated constructor stub

}

void LedDebug::setupPort1() {
    P1DIR = 0x01; // set up bit 0 of P1 as output
    P1OUT = 0x00; // intialize bit 0 of P1 to 0
}
void LedDebug::toggle() {
    ledState = !ledState;
    // toggle bit 0 of P1
    P1OUT ^= 0x01;
}

