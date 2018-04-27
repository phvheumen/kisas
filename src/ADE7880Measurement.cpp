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
	this->activeTotalEnergyAccum = 0.0f;
	this->activeFundamentalEnergyAccum = 0.0f;
	this->reactiveFundamentalEnergyAccum = 0.0f;
	this->apparentTotalEnergyAccum = 0.0f;
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
	Get the instantaneous voltage waveform measurement of a specified phase.

	@param phase The phase to measure.
	@return The waveform voltage [V]. If value is 0.0f, check for errors that may occurred

 */
float ADE7880Measurement::V(Phase_t phase) {
	uint16_t registerAddress;
	float pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA3);;
	float sensorGain;

	switch (phase) {
	case PHASE_A:
		registerAddress = VAWV;
		sensorGain = this->voltageSensorGain[PHASE_A];
		break;
	case PHASE_B:
		registerAddress = VBWV;
		sensorGain = this->voltageSensorGain[PHASE_B];
		break;
	case PHASE_C:
		registerAddress = VCWV;
		sensorGain = this->voltageSensorGain[PHASE_C];
		break;
	case PHASE_N:
		return 0.0f; // There is no neutral voltage measurement, return 0.0f
		break;
	default: // If ever enumerations are added to Phase_t, this function doesn't handle them and returns 0.0f
		return 0.0f;
	}

	int32_t v_raw;
	if( !this->_parent->regRead24S(registerAddress, v_raw) ) {
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

	return ( 0.5f * ( ((float) v_raw) / 5326737.0f) * sensorGain ) / pgaGain;
}

/**
	Get the instantaneous current waveform measurement of a specified phase.

	@param phase The phase to measure.
	@return The waveform current [A]. If value is 0.0f, check for errors that may occurred

 */
float ADE7880Measurement::I(Phase_t phase) {
	uint16_t registerAddress;
	float pgaGain;
	float sensorGain;

	switch (phase) {
	case PHASE_A:
		registerAddress = IAWV;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		sensorGain = this->currentSensorGain[PHASE_A];
		break;
	case PHASE_B:
		registerAddress = IBWV;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		sensorGain = this->currentSensorGain[PHASE_B];
		break;
	case PHASE_C:
		registerAddress = ICWV;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		sensorGain = this->currentSensorGain[PHASE_C];
		break;
	case PHASE_N:
		registerAddress = INWV;
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA2);
		sensorGain = this->currentSensorGain[PHASE_N];
		break;
	default: // If ever enumerations are added to Phase_t, this functions doesn't handle them and returns 0.0f
		return 0.0f;
	}

	int32_t i_raw;
	if( !this->_parent->regRead24S(registerAddress, i_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	/*
	 * ADC produces 24-bit signed two's complement codes.
	 * Extreme codes are then +/- 2^23 = 8,388,608 corresponding with +/- 0.787V
	 * Full scale range is then +/- 5,326,737 corresponding with +/- 0.5V
	 * Full scale RMS measurement corresponds then with 5,326,737/srqt(2) = 3,766,572 (0.5V/sqrt(2) = 0.354Vrms at ADC input)
	 *
	 * In the calculations of the real world values, xIRMSOS and xIGAIN registers are not used.
	 * These register are considered to be calibration registers for this measurement.
	 */

	return ( ( 0.5f ) * ( ((float) i_raw) / 5326737.0f) * sensorGain ) / pgaGain;
}

/**
	Get the PF of a specified phase.

	@param phase The phase to measure.
	@return The power factor between -1 and 1 for the given phase.
 */
float ADE7880Measurement::PF(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = APF;
		break;
	case PHASE_B:
		registerAddress = BPF;
		break;
	case PHASE_C:
		registerAddress = CPF;
		break;
	case PHASE_N:
		return 0.0f;
		break;
	default: // If ever enumerations are added to Phase_t, this functions doesn't handle them and returns 0.0f
		return 0.0f;
	}

	int16_t pf_raw;
	if ( !this->_parent->regRead16S(registerAddress, pf_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	return ( (float) pf_raw / (2^15) );
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
	Gives the instantaneous total active power.

	@param phase Phase to measure total active power.
	@return The total instantaneous active power. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::Ptotal(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = AWATT;
		break;
	case PHASE_B:
		registerAddress = BWATT;
		break;
	case PHASE_C:
		registerAddress = CWATT;
		break;
	default:
		return 0.0f;
	}

	int32_t power_raw;
	if( !this->_parent->regRead24S(registerAddress, power_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	uint32_t pMax = 27059678 >> 4;
	float powerFS = this->getPowerFS(phase);

	return powerFS * ( (float) power_raw ) / (float) pMax;
}

/**
	Gives the instantaneous total apparent power.

	@param phase Phase to measure total apparent power.
	@return The total instantaneous apparent power. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::Stotal(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = AVA;
		break;
	case PHASE_B:
		registerAddress = BVA;
		break;
	case PHASE_C:
		registerAddress = CVA;
		break;
	default:
		return 0.0f;
	}

	int32_t power_raw;
	if( !this->_parent->regRead24S(registerAddress, power_raw) ) {
		// Register read failed, return 0.0f
		return 0.0f;
	}

	const uint32_t pMax = 27059678 >> 4;
	float powerFS = this->getPowerFS(phase);

	return powerFS * ( (float) power_raw ) / (float) pMax;
}

float ADE7880Measurement::activeEnergyTotal(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = AWATTHR;
		break;
	case PHASE_B:
		registerAddress = BWATTHR;
		break;
	case PHASE_C:
		registerAddress = CWATTHR;
		break;
	default:
		return 0.0f;
	}

	int32_t energy_raw;
	if ( !this->_parent->regRead32S(registerAddress, energy_raw)){
		return 0.0f;
	}

	const uint32_t pMax = 27059678;
	const float fs = 1024e6;
	float powerFS = this->getPowerFS(phase);
	uint32_t wattHourThreshold = this->_parent->settings.getWHourThreshold();

	float activeEnergyTotal = ( energy_raw * ((float) wattHourThreshold) * powerFS ) / ( fs * 3600 * pMax );

	/*
	 * Check if read-with-reset is enabled. If enabled we just accumulate value to internal counter of measurement class.
	 * If not, we have to accumulate the difference with a previous measurement;
	 */
	if( this->_parent->settings.wattHourResetRead() ) {
		this->activeTotalEnergyAccum += activeEnergyTotal;
	} else {
		//TODO: Handle the case read-with-reset is disabled
		return 0.0f;
	}

	return this->activeTotalEnergyAccum;
}

float ADE7880Measurement::activeEnergyFundamental(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = AFWATTHR;
		break;
	case PHASE_B:
		registerAddress = BFWATTHR;
		break;
	case PHASE_C:
		registerAddress = CFWATTHR;
		break;
	default:
		return 0.0f;
	}

	int32_t energy_raw;
	if ( !this->_parent->regRead32S(registerAddress, energy_raw)){
		return 0.0f;
	}

	const uint32_t pMax = 27059678;
	const float fs = 1024e6;
	float powerFS = this->getPowerFS(phase);
	uint32_t wattHourThreshold = this->_parent->settings.getWHourThreshold();

	float activeEnergyFundamental = ( energy_raw * ((float) wattHourThreshold) * powerFS ) / ( fs * 3600 * pMax );

	/*
	 * Check if read-with-reset is enabled. If enabled we just accumulate value to internal counter of measurement class.
	 * If not, we have to accumulate the difference with a previous measurement;
	 */
	if( this->_parent->settings.wattHourResetRead() ) {
		this->activeFundamentalEnergyAccum += activeEnergyFundamental;
	} else {
		//TODO: Handle the case read-with-reset is disabled
		return 0.0f;
	}

	return this->activeFundamentalEnergyAccum;
}

float ADE7880Measurement::reactiveEnergyFundamental(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = AFVARHR;
		break;
	case PHASE_B:
		registerAddress = BFVARHR;
		break;
	case PHASE_C:
		registerAddress = CFVARHR;
		break;
	default:
		return 0.0f;
	}

	int32_t energy_raw;
	if ( !this->_parent->regRead32S(registerAddress, energy_raw)){
		return 0.0f;
	}

	const uint32_t pMax = 27059678;
	const float fs = 1024e6;
	float powerFS = this->getPowerFS(phase);
	uint32_t varHourThreshold = this->_parent->settings.getVARHourThreshold();

	float reactiveEnergyFundamental = ( energy_raw * ((float) varHourThreshold) * powerFS ) / ( fs * 3600 * pMax );

	/*
	 * Check if read-with-reset is enabled. If enabled we just accumulate value to internal counter of measurement class.
	 * If not, we have to accumulate the difference with a previous measurement;
	 */
	if( this->_parent->settings.wattHourResetRead() ) {
		this->reactiveFundamentalEnergyAccum += reactiveEnergyFundamental;
	} else {
		//TODO: Handle the case read-with-reset is disabled
		return 0.0f;
	}

	return this->reactiveFundamentalEnergyAccum;
}

float ADE7880Measurement::apparentEnergyTotal(Phase_t phase) {
	uint16_t registerAddress;

	switch (phase) {
	case PHASE_A:
		registerAddress = AVAHR;
		break;
	case PHASE_B:
		registerAddress = BVAHR;
		break;
	case PHASE_C:
		registerAddress = CVAHR;
		break;
	default:
		return 0.0f;
	}

	int32_t energy_raw;
	if ( !this->_parent->regRead32S(registerAddress, energy_raw)){
		return 0.0f;
	}

	const uint32_t pMax = 27059678;
	const float fs = 1024e6;
	float powerFS = this->getPowerFS(phase);
	uint32_t vaHourThreshold = this->_parent->settings.getVAHourThreshold();

	float apparentEnergyFundamental = ( energy_raw * ((float) vaHourThreshold) * powerFS ) / ( fs * 3600 * pMax );

	/*
	 * Check if read-with-reset is enabled. If enabled we just accumulate value to internal counter of measurement class.
	 * If not, we have to accumulate the difference with a previous measurement;
	 */
	if( this->_parent->settings.wattHourResetRead() ) {
		this->apparentTotalEnergyAccum += apparentEnergyFundamental;
	} else {
		//TODO: Handle the case read-with-reset is disabled
		return 0.0f;
	}

	return this->apparentTotalEnergyAccum;
}

/**
	Sets the sensor gain for the current channels. The gain should be strictly positive.
	If invalid gain is given, the gain will be set to 1.0

	@param phase The phase to set the gain for
	@param gain The gain of the sensor in unit [A/V] (input/output)
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
	@param gain The gain of the sensor in unit [V/V] (input/output).
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

/**
	Get voltage full scale range based on sensor gain and ADE7880 settings

	@param phase Phase to get the FS range from
	@return The full scale range voltage of the specified channel. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::getVoltageFS(Phase_t phase) {
	float pgaGain;
	float sensorGain;

	switch (phase) {
	case PHASE_A:
	case PHASE_B:
	case PHASE_C:
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA3);
		sensorGain = this->voltageSensorGain[phase];
		break;
	default:
		return 0.0;
	}

	return ( 0.5f * sensorGain / pgaGain );
}

/**
	Get current full scale range based on sensor gain and ADE7880 settings

	@param phase Phase to get the FS range from
	@return The full scale range current of the specified channel. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::getCurrentFS(Phase_t phase) {
	float pgaGain;

	switch (phase) {
	case PHASE_A:
	case PHASE_B:
	case PHASE_C:
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA1);
		break;
	case PHASE_N:
		pgaGain = (float) this->_parent->settings.getPGAGain(ADE7880Setting::PGA2);
		break;
	default:
		return 0.0;
	}

	float sensorGain = this->currentSensorGain[phase];
	return ( 0.5f * sensorGain / pgaGain );
}

/**
	Get power full scale range based on sensor gain and ADE7880 settings

	@param phase Phase to get the FS range from
	@return The full scale range power of the specified channel. If value is 0.0f, check for errors that may occurred
 */
float ADE7880Measurement::getPowerFS(Phase_t phase) {
	float currentFS;
	float voltageFS;

	switch (phase) {
	case PHASE_A:
	case PHASE_B:
	case PHASE_C:
		currentFS = this->getCurrentFS(phase);
		voltageFS = this->getVoltageFS(phase);
		break;
	default:
		return 0.0;
	}

	return ( currentFS * voltageFS ) / 2;
}


