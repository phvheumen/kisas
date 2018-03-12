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
#include "ADE7880RegisterNames.h"
#include "ADE7880.h"
#include "utils.h"

#include "Applications.h"

#include "SdFat.h"

// Pick an SPI configuration.
// See SPI configuration section below (comments are for photon).
#ifndef SPI_CONFIGURATION
#define SPI_CONFIGURATION 1
#endif
//------------------------------------------------------------------------------
// Setup SPI configuration.
#if SPI_CONFIGURATION == 0
// Primary SPI with DMA
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFat SD;
const uint8_t chipSelect = SS;
#elif SPI_CONFIGURATION == 1
// Secondary SPI with DMA
// SCK => D4, MISO => D3, MOSI => D2, SS => D5
SdFat SD(1);
const uint8_t chipSelect = SS_SD;
#elif SPI_CONFIGURATION == 2
// Primary SPI with Arduino SPI library style byte I/O.
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFatLibSpi SD;
const uint8_t chipSelect = SS;
#elif SPI_CONFIGURATION == 3
// Software SPI.  Use any digital pins.
// MISO => D5, MOSI => D6, SCK => D7, SS => D0
SdFatSoftSpi<D5, D6, D7> SD;
const uint8_t chipSelect = D0;
#endif  // SPI_CONFIGURATION
//------------------------------------------------------------------------------

/*
 * SYSTEM_MODE Macro to set system mode. See https://docs.particle.io/support/troubleshooting/mode-switching/electron/
 * for more information.
 */
SYSTEM_MODE(SEMI_AUTOMATIC);

//ADE7880 * ade7880 = new ADE7880(&SPI, SS_HSA, RESETINV, true);
//ADE7880 * ade7880 = new ADE7880(&Wire, RESETINV, true);
ADE7880 * ade7880 = new ADE7880(&SPI, &Wire, SS_HSA, RESETINV, true);

ApplicationManager Application;

File myFile;
char filename[] = "000.txt";

uint8_t ram[20000] = { 0 };

void incrementNumStr(char * str) {
	for(int i = strlen(str) - 5; i >= 0; i--)
	{
		if(str[i] >= '9') {
			str[i] = '0';
			continue;
		} else if(str[i] < '9') {
			str[i] = str[i] + 1;
			break;
		}
	}
}
/*
 * void setup()
 * Initialisation routine
 */
uint32_t reg32;
uint16_t reg16;
bool commValid;
uint32_t selectStateCount = 0;
uint8_t tx_buffer[64];
uint8_t rx_buffer[64];


#pragma GCC push_options
#pragma GCC optimize ("O0")
void setup()
{
	ade7880->Begin();
	commValid = ade7880->regRead32(CHECKSUM, reg32);

	while(selectStateCount < 8000) {
		if( ADE7880::selectState ) {
			ADE7880::selectState = false;
			selectStateCount++;

			SPI.transfer(tx_buffer, rx_buffer, 2, NULL);
		}

	}

	__asm("BKPT");



//	ade7880->initADE7880();

//	if (!SD.begin(chipSelect, SPI_HALF_SPEED)) {
//		// If SD card is not inserted (or broken) we end up here.
////	    SD.initErrorHalt();
//		__asm("BKPT");
//	}
//
//	int c = 0;
//	while(c == 0)
//	{
//		if(!SD.exists(filename)) // So, there does not exist a file with that name
//		{
//			// open the file for write at end like the "Native SD library"
//			if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
//				SD.errorHalt("opening abc.txt for write failed");
//			}
//			myFile.close();
//			c = 1; // Breaking out of while loop
//		} else if(SD.exists(filename)) { // So, there does exist a file with that name -> change the name and search again
//			incrementNumStr(filename);
//		}
//	}
//	__asm("BKPT");
//
//
//	// open the file for write at end like the "Native SD library"
//	if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
//		SD.errorHalt("opening abc.txt for write failed");
//	}
//	myFile.println("testing 1, 2, 3."); // if the file opened okay, write to it:
//	myFile.printf("fileSize: %d\n", myFile.fileSize()); // close the file:
//	myFile.close(); // close the file:
//
//	// re-open the file for reading:
//	if (!myFile.open(filename, O_READ)) {
//	SD.errorHalt("opening test.txt for read failed");
//	}
//	Serial.println("abc.txt content:");
//
//	// read from the file until there's nothing else in it:
//	int data;
//	while ((data = myFile.read()) >= 0) {
//	Serial.write(data);
//	}
//	// close the file:
//	myFile.close();
//
//	__asm("BKPT");


	// Initialise the applications
//	Application.SimpleScope.init();
//
//	String ossil = "0a-53a-104a-150a-190a-221a-243a-254a-254a-243a-221a-190a-150a-104a-53a0a53a104a150a190a221a243a254a254a243a221a190a150a104a53a0";
//	Particle.variable("getossil", ossil);
}
#pragma GCC pop_options

////######### variables #############
float VArms;
float IArms;
float INrms;
float PArms;
float SArms2;
float SArms;
float PFA;
float CosphiA;
float SampledValue;
    unsigned long Dtime;
    unsigned long Curtime;
    unsigned long Newtime;
float VASample;
    float SampleBlock[1000];
float SingleSample;
    int StartIndex, StopIndex;

////For the SDcard:
//const uint8_t ADC_DIM = 4;
//const bool useSharedSpi = false;// Set useSharedSpi true for use of an SPI sensor.

float FrequencyMeasurement(byte RegisterX){ //works, unknown accuracy
    // TODO: Change register read function
	//SingleSample = Measurement.SPIRead16((uint16_t)APERIOD+RegisterX);
    float Period_Measured = ((float)SingleSample/256000);
    float Frequency_Measured= ((256000/(float)SingleSample));
    return Frequency_Measured;
}

float HarmonicDistortionVTHD(void){
    // TODO: Implement
	return 0.0f;
}

float HarmonicDistortionITHD(void){
    // TODO: Implement
	return 0.0f;
}

float HarmonicXVRMSMeasurement(void){ //measures the VRMS in the distortion of HX, needs testing
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead24S((uint16_t)HXVRMS);
    float HVmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
    return HVmeasured;
}

float HarmonicYVRMSMeasurement(void){ //measures the VRMS in the distortion of HY, needs testing
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead24S((uint16_t)HYVRMS);
    float HVmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
    return HVmeasured;
}

float HarmonicZVRMSMeasurement(void){ //measures the VRMS in the distortion of HZ,needs testing
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead24S((uint16_t)HZVRMS);
    float HVmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
    return HVmeasured;
}

void HarmonicsVReadout(void){//nog fatsoenlijk doormiddel van pointers uit laten poepen
	// TODO: Change register read function
    float Vrms[54];

//    uint16_t temp1 = Measurement.SPIRead16((uint16_t)HCONFIG); // 187+4 set the 2 ACTPHSEL bits to 10 for reverence of voltage C (not A B or N)
//    uint16_t temp2 = temp1 & 0xFF;
//    temp1 = temp2 + 0x200;
//    Measurement.SPIWrite16((uint16_t)HCONFIG, temp1);

    for(byte i =0; i <= 8; i = i + 3 ){
        //Measurement.SPIWrite8((uint16_t)HX, i);
        //Measurement.SPIWrite8((uint16_t)HY, i+1);
        //Measurement.SPIWrite8((uint16_t)HZ, i+2);

        Vrms[i-1] = HarmonicXVRMSMeasurement();
        Vrms[i  ] = HarmonicXVRMSMeasurement();
        Vrms[i+1] = HarmonicXVRMSMeasurement();


    }
    //Serial.println("the newline");
    //Serial.println("VRMS of upto x Harmonics:");
    for(byte i =0; i <= 10; i++){
        //Serial.print("  ");
        //Serial.print(Vrms[i]);
    }

}

float VxRMSMeasurement(byte RegisterX){ //works, unknown accuracy, probably only the first order.
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead24S((uint16_t)AVRMS+RegisterX*2);
    float Vmeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000;
    float Vmeasured2=(((float)SingleSample)/100000) *9.38660911282921877506715;
    return Vmeasured;
}

float IxRMSMeasurement(byte RegisterX){
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead24S((uint16_t)AIRMS+RegisterX*2);
    float Imeasured=(SingleSample/3766572)*0.5*.5*sqrt(2)*1000; //still needs right factor, currently unknown
    return Imeasured;
}

//write to WTHR to change accuracy of CWATTHR
float xWATTHRMeasurement(byte RegisterX){
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead32S(((uint16_t)AWATTHR+RegisterX));
    float WATTMeasured=(SingleSample/123456789); //still arbitrairy numer, need to calculate well (oa eq 26 from datasheet ADE7880)
    return WATTMeasured;
}       //DONE float fetch xWATTHR //total active engergy accumulation

float xFWATTHRMeasurement(byte RegisterX){
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead32S(((uint16_t)AFWATTHR+RegisterX));
    float WATTMeasured=(SingleSample/123456789); //still arbitrairy numer, need to calculate well (oa eq x from datasheet ADE7880)
    return WATTMeasured;
}       //DONE float fetch xFWATTHR //Fundemental active energy accumulation

float xVARHRMeasurement(byte RegisterX){
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead32S(((uint16_t)AFVARHR+RegisterX));
    float VARMeasured=(SingleSample/123456789); //still arbitrairy numer, need to calculate well (oa eq x from datasheet ADE7880)
    return VARMeasured;
}

float xPFMeasurement(byte RegisterX){
	// TODO: Change register read function
    //SingleSample = Measurement.SPIRead16(((uint16_t)APF+RegisterX));
    float PFMeasured=((float)SingleSample/(2^15));
    return PFMeasured;
}       //DONE float fetch xPF // the Powerfactor

//float fetch xWATT // Total Active Power Calculation joules/s, (waveform sample?)
//float fetch xVA // Instantaneous power (waveform sample?)

void loop()
{
	/*
	 * Measured phase 3 for neutral, then A=0 B=1 and C=2,  Neutral not yet implemented.
	 * I presume compiler will just fill in the number everywhere since it is static, dont want to do it with #DEFINE yet though
	 */
	byte            Phase = 2;
	int             Timestamp11 = Time.now();
	unsigned long   Timestamp12 = millis();
	float           VRMSC = VxRMSMeasurement(Phase);
	float           IRMSC = IxRMSMeasurement(Phase);
	float           Frequency = FrequencyMeasurement(Phase);
	float           Powerfactor = xPFMeasurement(Phase);
	float           WATTHR = xWATTHRMeasurement(Phase);
	float           FWATTHR = xFWATTHRMeasurement(Phase);
	float           VARHR = xVARHRMeasurement(Phase);
	String dataString = "";

	unsigned long   Timestamp2 = millis();
	//    File dataFile = SD.open("datalog.txt", FILE_WRITE);

	dataString += "Time start Measurements:";
	dataString += String(Timestamp11);
	dataString += "/t";
	dataString += String(Timestamp12);
	dataString += "/t time End measurements:";
	dataString += String(Timestamp2);
	dataString += "/t Frequency:";
	dataString += String(Frequency);
	dataString += "VRMS = ";
	dataString += String(VRMSC);

	while (sizeof(dataString)<512){
	 dataString += "0";
	}
	if (sizeof(dataString)>512)
	{
		// TODO: Maak korter (niet van P.H. van Heumen)
	}

//	if (dataFile) {
//	dataFile.println(dataString);
//	dataFile.close();
//	// print to the serial port too:
//	Serial.println(datastring);
//	}
//	else {
//	Serial.println("error opening datalog.txt");
//	}
	Application.SimpleScope.update(); //automatic every 1 second
	delay(500);
}
