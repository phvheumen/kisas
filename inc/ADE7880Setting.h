/*
 * ADE7880Setting.h
 *
 *  Created on: 15 Apr 2018
 *      Author: P.H. van Heumen (p.h.v.heumen@gmail.com)
 */

#ifndef INC_ADE7880SETTING_H_
#define INC_ADE7880SETTING_H_

#include <application.h>
#include <stdint.h>

// Forward declare ADE7880 class
class ADE7880;

class ADE7880Setting
{
public:
	/*
	 * Type definitions
	 */
	typedef enum PGA {
		PGA1 = 1, // Phase currents
		PGA2 = 2, // Neutral current
		PGA3 = 3, // Phase voltages
	} PGA_t;

	typedef enum PGA_Gain{
		PGA_Gain1 = 1,
		PGA_Gain2 = 2,
		PGA_Gain4 = 4,
		PGA_Gain8 = 8,
		PGA_Gain16 = 16,
	} PGA_Gain_t;

public:
	ADE7880Setting(ADE7880 * parent);

	void setPGAGain(PGA_t PGA, PGA_Gain_t gain);
	PGA_Gain_t getPGAGain(PGA_t PGA);

	/*
	 * User defined operators
	 */
	ADE7880Setting& operator= ( const ADE7880Setting & ) = delete; // Delete copy assignment operator
	ADE7880Setting& operator= ( ADE7880Setting && ) = delete; // Delete move assignment operator
private:
	ADE7880 * _parent;

};



#endif /* INC_ADE7880SETTING_H_ */
