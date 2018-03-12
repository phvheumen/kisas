/*
 * ADE7880Settings.cpp
 *
 *  Created on: 15 Apr 2018
 *      Author: P.H. van Heumen (p.h.v.heumen@gmail.com)
 */

#include "ADE7880Settings.h"
#include "ADE7880.h"

ADE7880Settings::ADE7880Settings(ADE7880 * parent) {
	this->parent = parent;
}

void ADE7880Settings::setPGAGain(PGA_t PGA, PGA_Gain_t gain) {

}

ADE7880Settings::PGA_Gain_t ADE7880Settings::getPGAGain(PGA_t PGA) {

	return PGA_Gain1;
}




