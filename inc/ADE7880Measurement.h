/*
 * ADE7880Measurement.h
 *
 *  Created on: 25 Apr 2018
 *      Author: P.H. van Heumen (p.h.v.heumen@gmail.com)
 */

#ifndef INC_ADE7880MEASUREMENT_H_
#define INC_ADE7880MEASUREMENT_H_

#include <application.h>
#include <stdint.h>

// Defenitions
#define SQRT2 1.4142135623730951 // sqrt(2)

// Forward declare ADE7880 class
class ADE7880;

class ADE7880Measurement {
public:
	ADE7880Measurement(ADE7880 * parent);

	/*
	 * Type definitions
	 */
	typedef enum {
		PHASE_A = 0,
		PHASE_B,
		PHASE_C,
		PHASE_N
	} Phase_t;

	/*
	 * Measurement methods
	 */
	float Irms(Phase_t phase);
	float Vrms(Phase_t phase);

	float V(Phase_t phase);
	float I(Phase_t phase);

	float period(Phase_t phase);
	float frequency(Phase_t phase);

	float Ptotal(Phase_t phase);
	float Stotal(Phase_t phase);

	float PF(Phase_t phase);

	float activeEnergyTotal(Phase_t phase);
	float activeEnergyFundamental(Phase_t phase);
	float reactiveEnergyFundamental(Phase_t phase);
	float apparentEnergyTotal(Phase_t phase);

	/*
	 * Settings methods
	 */
	void setCurrentSensorGain(Phase_t phase, float gain);
	float getCurrentSensorGain(Phase_t phase) { return this->currentSensorGain[phase]; };

	void setVoltageSensorGain(Phase_t phase, float gain);
	float getVoltageSensorGain(Phase_t phase) { return this->currentSensorGain[phase]; };

	/*
	 * Miscellaneous methods
	 */
	float getVoltageFS(Phase_t phase);
	float getCurrentFS(Phase_t phase);
	float getPowerFS(Phase_t phase);

private:
	ADE7880 * _parent;

	float currentSensorGain[4];	// Sensor gain for the current input {A, B, C, N}
	float voltageSensorGain[3]; // Sensor gain for the voltage input {A, B, C}

	float activeTotalEnergyAccum;
	float activeFundamentalEnergyAccum;
	float reactiveFundamentalEnergyAccum;
	float apparentTotalEnergyAccum;
};

#endif /* INC_ADE7880MEASUREMENT_H_ */
