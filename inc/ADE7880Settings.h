/*
 * ADE7880Settings.h
 *
 *  Created on: 15 Apr 2018
 *      Author: Pasquale
 */

#ifndef INC_ADE7880SETTINGS_H_
#define INC_ADE7880SETTINGS_H_

#include <application.h>
#include <stdint.h>

// Forward declare ADE7880 class
class ADE7880;

class ADE7880Settings
{
public:
	/*
	 * Type defenitions
	 */
	typedef enum PGA {
		PGA_PhaseVoltage,
		PGA_PhaseCurrent,
		PGA_NeutralCurrent,
	} PGA_t;

	typedef enum PGA_Gain{
		PGA_Gain1 = 1,
		PGA_Gain2 = 2,
		PGA_Gain4 = 4,
		PGA_Gain8 = 8,
		PGA_Gain16 = 16,
	} PGA_Gain_t;

public:
	ADE7880Settings(ADE7880 * parent);

	void setPGAGain(PGA_t PGA, PGA_Gain_t gain);
	PGA_Gain_t getPGAGain(PGA_t PGA);

	/*
	 * User defined operators
	 */
	ADE7880Settings& operator= ( const ADE7880Settings & ) = delete; // Delete copy assignment operator
	ADE7880Settings& operator= ( ADE7880Settings && ) = delete; // Delete move assignment operator
private:
	ADE7880 * parent;

};



#endif /* INC_ADE7880SETTINGS_H_ */
