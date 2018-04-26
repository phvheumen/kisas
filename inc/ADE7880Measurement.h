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

};

//
//float HarmonicDistortionVTHD(void){
//    // TODO: Implement
//	return 0.0f;
//}
//
//float HarmonicDistortionITHD(void){
//    // TODO: Implement
//	return 0.0f;
//}
//
//float HarmonicXVRMSMeasurement(void){ //measures the VRMS in the distortion of HX, needs testing
//    //SingleSample = Measurement.SPIRead24S((uint16_t)HXVRMS);
//    float HVmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
//    return HVmeasured;
//}
//
//float HarmonicYVRMSMeasurement(void){ //measures the VRMS in the distortion of HY, needs testing
//    //SingleSample = Measurement.SPIRead24S((uint16_t)HYVRMS);
//    float HVmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
//    return HVmeasured;
//}
//
//float HarmonicZVRMSMeasurement(void){ //measures the VRMS in the distortion of HZ,needs testing
//    //SingleSample = Measurement.SPIRead24S((uint16_t)HZVRMS);
//    float HVmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
//    return HVmeasured;
//}
//
//void HarmonicsVReadout(void){//nog fatsoenlijk doormiddel van pointers uit laten poepen
//	// TODO: Change register read function
//    float Vrms[54];
//
////    uint16_t temp1 = Measurement.SPIRead16((uint16_t)HCONFIG); // 187+4 set the 2 ACTPHSEL bits to 10 for reverence of voltage C (not A B or N)
////    uint16_t temp2 = temp1 & 0xFF;
////    temp1 = temp2 + 0x200;
////    Measurement.SPIWrite16((uint16_t)HCONFIG, temp1);
//
//    for(byte i =0; i <= 8; i = i + 3 ){
//        //Measurement.SPIWrite8((uint16_t)HX, i);
//        //Measurement.SPIWrite8((uint16_t)HY, i+1);
//        //Measurement.SPIWrite8((uint16_t)HZ, i+2);
//
//        Vrms[i-1] = HarmonicXVRMSMeasurement();
//        Vrms[i  ] = HarmonicXVRMSMeasurement();
//        Vrms[i+1] = HarmonicXVRMSMeasurement();
//
//
//    }
//    //Serial.println("the newline");
//    //Serial.println("VRMS of upto x Harmonics:");
//    for(byte i =0; i <= 10; i++){
//        //Serial.print("  ");
//        //Serial.print(Vrms[i]);
//    }
//
//}
//



#endif /* INC_ADE7880MEASUREMENT_H_ */
