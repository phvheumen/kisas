/*
 * ADE7880Setting.cpp
 *
 *  Created on: 15 Apr 2018
 *      Author: P.H. van Heumen (p.h.v.heumen@gmail.com)
 */

#include "../inc/ADE7880Setting.h"

#include "ADE7880.h"

ADE7880Setting::ADE7880Setting(ADE7880 * parent) {
	this->_parent = parent;
}

void ADE7880Setting::setPGAGain(PGA_t PGA, PGA_Gain_t gain) {

}

ADE7880Setting::PGA_Gain_t ADE7880Setting::getPGAGain(PGA_t PGA) {

	return PGA_Gain1;
}

uint32_t ADE7880Setting::getWHourThreshold() {
	return (0x3 << 27); // Default value ADE7880
}

uint32_t ADE7880Setting::getVARHourThreshold() {
	return (0x3 << 27); // Default value ADE7880
}

uint32_t ADE7880Setting::getVAHourThreshold() {
	return (0x3 << 27); // Default value ADE7880
}

bool ADE7880Setting::wattHourResetRead() {
	return true; // Default value ADE7880
}



