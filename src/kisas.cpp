/*
 * KiSAS - Keep It Simple Asset Sensor
 *
 * version update, last update:
 * 19th august 2016
 *
 */
 
//######### headers and defines #############
#if defined(__AVR__)
  #include <Arduino.h>
  #include <Wire.h>
  #include <SoftwareSerial.h>
  #include "MyWatchdog.h"
#endif
#if defined (__arm__)
    #include "math.h"
    #include <application.h>
#endif

#include "version.h"
#include "pindefs.h"

#include "ADE7880.h"
#include "ADE7880RegisterNames.h"

#include "utils.h"
#include "Applications.h"
#include "SdFat.h"

/*
 * SYSTEM_MODE Macro to set system mode. See https://docs.particle.io/support/troubleshooting/mode-switching/electron/
 * for more information.
 */
SYSTEM_MODE(SEMI_AUTOMATIC);

//ADE7880 * ade7880 = new ADE7880(&SPI, SS_HSA, RESETINV, true);
//ADE7880 * ade7880 = new ADE7880(&Wire, RESETINV, true);
ADE7880 * ade7880 = new ADE7880(&SPI, &Wire, SS_HSA, RESETINV, false);

/*
 * void setup()
 * Initialisation routine
 */

#pragma GCC push_options
#pragma GCC optimize ("O0")
void setup()
{
	ade7880->Begin();

	/* Configure HSDC (See table 52 in data sheet) */
	uint8_t reg8= 0x0;
	reg8 |= (0x1 << 0); // CLK is 4 MHz
	reg8 |= (0x1 << 3); // Only send voltage and currents
	ade7880->regWrite8(HSDC_CFG, reg8);

	// Enable HSDC
	uint16_t reg16;
	ade7880->regRead16(CONFIG, reg16);
	reg16 |= (1<<6); // Enable HSDC
	ade7880->regWrite16(CONFIG, reg16);

	ade7880->measurement.setCurrentSensorGain(ADE7880Measurement::PHASE_A, 10000.0f);
	ade7880->measurement.setVoltageSensorGain(ADE7880Measurement::PHASE_A, 1.0f);
	while(1) {
		volatile float Irms = ade7880->measurement.Irms(ADE7880Measurement::PHASE_A);
		volatile float Vrms = ade7880->measurement.Vrms(ADE7880Measurement::PHASE_A);
		volatile float freq = ade7880->measurement.frequency(ADE7880Measurement::PHASE_A);
		volatile float per = ade7880->measurement.period(ADE7880Measurement::PHASE_A);
		__asm("BKPT");
	}
}
#pragma GCC pop_options

void loop() {

}
