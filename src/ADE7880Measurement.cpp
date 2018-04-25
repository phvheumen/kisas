/*
 * ADE7880Measurements.cpp
 *
 *  Created on: 25 Apr 2018
 *      Author: Pasquale
 */

#include "ADE7880Measurement.h"
#include "ADE7880Setting.h"
#include "ADE7880.h"

#include <algorithm>

ADE7880Measurement::ADE7880Measurement(ADE7880 * parent) {
	this->_parent = parent;
	std::fill(std::begin(this->currentSensorGain), std::end(this->currentSensorGain), 1.0f);
	std::fill(std::begin(this->voltageSensorGain), std::end(this->voltageSensorGain), 1.0f);
}

/**
	Get the instantaneous current RMS measurement of a specified phase.

	@param phase The phase to measure.
	@return The rms current [A]. If value is 0.0f, check for errors that may occurred

 */
float ADE7880Measurement::Irms(Phase_t phase) {
	/*
	 * NOTE: According data sheet p.42 (Rev. C) it is recommend to read the rms register synchronous
	 * to the voltage zero crossings to ensure stability. This is not implemented yet
	 */
	uint16_t registerAddress;
	float pgaGain;
	float sensorGain;

	switch (phase) {
	case PHASE_A:
		registerAddress = AIRMS;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		sensorGain = this->currentSensorGain[PHASE_A];
		break;
	case PHASE_B:
		registerAddress = BIRMS;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		sensorGain = this->currentSensorGain[PHASE_B];
		break;
	case PHASE_C:
		registerAddress = CIRMS;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		sensorGain = this->currentSensorGain[PHASE_C];
		break;
	case PHASE_N:
		registerAddress = NIRMS;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA2);
		sensorGain = this->currentSensorGain[PHASE_N];
		break;
	default: // If ever enumerations are added to Phase_t, this functions doesn't handle them and returns 0.0f
		return 0.0f;
	}

	int32_t rms_raw;
	if( !this->_parent->regRead24S(registerAddress, rms_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	/*
	 * ADC produces 24-bit signed two's complement codes.
	 * Extreme codes are then +/- 2^23 = 8,388,608 corresponding with +/- 0.787V
	 * Full scale range is then +/- 5,326,737 corresponding with +/- 0.5V
	 * Full scale RMS measurement corresponds then with 5,326,737/srqt(2) = 3,766,572 (0.5V/sqrt(2) = 0.354Vrms at ADC input)
	 * When integrator enabled @50 Hz FS RMS: 3,759,718
	 * When integrator enabled @60 Hz FS RMS: 3,133,207
	 *
	 * In the calculations of the real world values, xIRMSOS and xIGAIN registers are not used.
	 * These register are considered to be calibration registers for this measurement.
	 */

	// TODO: Adjust calculation if integrator is enabled or not and if we are working in a 50Hz or 60Hz network
	return ( ( 0.5f / SQRT2 ) * ( ((float) rms_raw) / 3759718.0f) * sensorGain ) / pgaGain;
}

/**
	Get the instantaneous voltage RMS measurement of a specified phase.

	@param phase The phase to measure.
	@return The rms voltage [V]. If value is 0.0f, check for errors that may occurred

 */
float ADE7880Measurement::Vrms(Phase_t phase) {
	/*
	 * NOTE: According data sheet p.44 (Rev. C) it is recommend to read the rms register synchronous
	 * to the voltage zero crossings to ensure stability. This is not implemented yet
	 */
	uint16_t registerAddress;
	float pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA3);;
	float sensorGain;

	switch (phase) {
	case PHASE_A:
		registerAddress = AVRMS;
		sensorGain = this->voltageSensorGain[PHASE_A];
		break;
	case PHASE_B:
		registerAddress = BVRMS;
		sensorGain = this->voltageSensorGain[PHASE_B];
		break;
	case PHASE_C:
		registerAddress = CVRMS;
		sensorGain = this->voltageSensorGain[PHASE_C];
		break;
	case PHASE_N:
		return 0.0f; // There is no neutral voltage measurement, return 0.0f
		break;
	default: // If ever enumerations are added to Phase_t, this functions doesn't handle them and returns 0.0f
		return 0.0f;
	}

	int32_t rms_raw;
	if( !this->_parent->regRead24S(registerAddress, rms_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	/*
	 * ADC produces 24-bit signed two's complement codes.
	 * Extreme codes are then +/- 2^23 = 8,388,608 corresponding with +/- 0.787V
	 * Full scale range is then +/- 5,326,737 corresponding with +/- 0.5V
	 * Full scale RMS measurement corresponds then with 5,326,737/srqt(2) = 3,766,572 (0.5V/sqrt(2) = 0.354Vrms at ADC input)
	 *
	 * In the calculations of the real world values, xVRMSOS and xVGAIN registers are not used.
	 * These register are considered to be calibration registers for this measurement.
	 */

	// TODO: Adjust calculation if integrator is enabled or not and if we are working in a 50Hz or 60Hz network
	return ( ( 0.5f / SQRT2 ) * ( ((float) rms_raw) / 3766572.0f) * sensorGain ) / pgaGain;
}

/**
	Get the line voltage period on the given phase.
	When period is outside valid range, it returns 0.0

	@param phase Phase to measure period
	@returns The period in [s]. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::period(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = APERIOD;
		break;
	case PHASE_B:
		registerAddress = BPERIOD;
		break;
	case PHASE_C:
		registerAddress = CPERIOD;
		break;
	default:
		return 0.0f;
	}

	uint16_t period_raw;
	if( !this->_parent->regRead16(registerAddress, period_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	float period = ((float) period_raw / 256000.0f);

	// TODO: Make frequency bounds adjustable
	if ( period < 0.001 || period > 0.250 ) {
		return 0.0f;
	} else {
		return period;
	}
}

/**
	Get the line voltage frequency on the given phase.
	When the frequency is outside valid range, it returns 0.0

	@param phase Phase to measure frequency
	@returns The frequency in [Hz]. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::frequency(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = APERIOD;
		break;
	case PHASE_B:
		registerAddress = BPERIOD;
		break;
	case PHASE_C:
		registerAddress = CPERIOD;
		break;
	default:
		return 0.0f;
	}

	uint16_t period_raw;
	if( !this->_parent->regRead16(registerAddress, period_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	float frequency = ( 256000.0f / (float) period_raw );

	// TODO: Make frequency bounds adjustable
	if ( frequency > 1000.0f || frequency < 4.0f ) {
		return 0.0f;
	} else {
		return frequency;
	}
}

/**
	Sets the sensor gain for the current channels. The gain should be strictly positive.
	If invalid gain is given, the gain will be set to 1.0

	@param phase The phase to set the gain for
	@param gain The gain of the sensor in unit [A/V]
 */
void ADE7880Measurement::setCurrentSensorGain(Phase_t phase, float gain) {
	if( gain > 0 ) {
		this->currentSensorGain[phase] = gain;
	} else {
		this->currentSensorGain[phase] = 1.0f;
	}
}

/**
	Sets the sensor gain for the voltage channels. The gain should be strictly positive.
	If invalid gain is given, the gain will be set to 1.0

	@param phase The phase to set the gain for
	@param gain The gain of the sensor in unit [V/V] (input/output)
 */
void ADE7880Measurement::setVoltageSensorGain(Phase_t phase, float gain) {
	switch (phase) {
	case PHASE_A:
	case PHASE_B:
	case PHASE_C:
		if( gain > 0 ) {
			this->voltageSensorGain[phase] = gain;
		} else {
			this->voltageSensorGain[phase] = 1.0f;
		}
		break;
	default:
		this->voltageSensorGain[phase] = 1.0f;
	}
}


