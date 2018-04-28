/*
 * KiSAS - Keep It Simple Asset Sensor
 *
 * version update, last update:
 * 19th august 2016
 *
 */
 
//######### headers and defines #############
#include "HttpClient.h"
#include "math.h"
#include <application.h>

#include "version.h"
#include "pindefs.h"

#include "ADE7880.h"
#include "ADE7880RegisterNames.h"

#include "utils.h"
#include "Applications.h"
#include "SdFat.h"

/*
 * SYSTEM_MODE Macro to set system mode. See https://docs.particle.io/support/troubleshooting/mode-switching/electron/
 * for more information. Modes can be MANUAL, SEMI_AUTOMATIC and AUTOMATIC
 */
SYSTEM_MODE(SEMI_AUTOMATIC);

/*
 * Depending on how the hardware is connected to the ADE use different constructor.
 * Declaring the ADE7880 with the new operator makes the object lifetime independent of scope. Thus object must be deleted explicitly
 * TODO: Review if this lifetime is necessary
 */
//ADE7880 * ade7880 = new ADE7880(&SPI, SS_HSA, RESETINV, true);
//ADE7880 * ade7880 = new ADE7880(&Wire, RESETINV, true);
ADE7880 * ade7880 = new ADE7880(&SPI, &Wire, SS_HSA, RESETINV, false);
ApplicationManager * apps;

/* Defines */
#define PARTICLE_CLOUD_TO_MS	20000
#define ENABLE_BOOT_MSG

/* MACROS */
#ifdef ENABLE_BOOT_MSG
	#define BOOT_MSG(x) {\
	Serial.printf("[%8.3f] ", ((float) millis() )/1000);\
	Serial.printlnf(x);\
	}
#else
	#define BOOT_MSG(x)
#endif

#ifdef ENABLE_BOOT_MSG
	#define BOOT_MSG_FORMATTED(x, y) {\
	Serial.printf("[%8.3f] ", ((float) millis() )/1000);\
	Serial.printlnf(x, y);\
	}
#else
	#define BOOT_MSG_FORMATTED(x,y)
#endif

/* Forward declarations */
void particleConnectTimeOutCallback(void);
int getSettings(String input);

/* Variable declarations/definitions */
ADE7880 Measurement(&SPI, &Wire, SS_HSA, RESETINV, false); //TODO: Remove this one
Timer particleConnectTimeOut(PARTICLE_CLOUD_TO_MS, particleConnectTimeOutCallback, true);
String ossil = "0a-53a-104a-150a-190a-221a-243a-254a-254a-243a-221a-190a-150a-104a-53a0a53a104a150a190a221a243a254a254a243a221a190a150a104a53a0";
String PostBuffer=""; //TODO: Remove this one

// HttpClient
unsigned int nextTime = 0;    // Next time to contact the server
HttpClient _http;

http_header_t _headers[] = {
	{ "Content-Type", "application/json" },
	{ "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t _request;
http_response_t _response;

/*
 * void setup()
 * Initialisation routine
 */
#pragma GCC push_options
#pragma GCC optimize ("O0")
void setup()
{
	Serial.begin(115200);   // open serial over USB
	/* On Windows it will be necessary to implement the following line:
	 * Make sure your Serial Terminal app is closed before powering your device
	 * Now open your Serial Terminal!
	 */
	while(!Serial.isConnected()) Particle.process();
	/* Above trick not always working.
	 * Press any key to start the application and terminal output
	 */
	while(!Serial.available()) Particle.process();

	Serial.println("KISAS: Keep it simple asset sensor");

	/*
	 * Display free memory and system version
	 */
	Serial.printlnf("Particle system version: %s", System.version().c_str());
	Serial.printlnf("Free memory: %d Bytes", System.freeMemory());
	Serial.printlnf("Device ID: %s", System.deviceID().c_str());
	Serial.println("");

	/*
	 * Assign cloud functions/variables
	 */
	Particle.function("getSettings", getSettings);
	Particle.variable("getossil", ossil);

	/*
	 * Connect to cellular network
	 */
	BOOT_MSG("Start connecting to cellular network");

	Cellular.on();
	BOOT_MSG("Cellular: module on");

	Cellular.connect();
	BOOT_MSG("Cellular: connecting to cellular network");

	while(Cellular.connecting()) Particle.process();
	BOOT_MSG("Cellular: connected");

	while(!Cellular.ready() || Cellular.localIP() == (uint32_t)0) Particle.process();
	BOOT_MSG_FORMATTED("Cellular: IP adress available [%s]", Cellular.localIP().toString().c_str());

	/*
	 * Connect to particle cloud
	 */
	BOOT_MSG("Connecting to Particle cloud");

	particleConnectTimeOut.start();
	Particle.connect();
	delay(100); // This delay will make sure the particleConnectTimeOut Timer is active

	while(1) {
		if( !particleConnectTimeOut.isActive() ) {
			BOOT_MSG("Reset system required!");
			while(1) { }; // Time out on particle cloud connect.
		}
		if( Particle.connected() ) {
			particleConnectTimeOut.stop();	  // Stop timer
			particleConnectTimeOut.timeout(); // Force callback
			particleConnectTimeOut.dispose(); // Delete timer
			break;
		}
		Particle.process();
	}

	/*
	 * Initialising ADE7880
	 */
	BOOT_MSG("Initialising ADE7880");
	ade7880->Begin();

//	/* Configure HSDC (See table 52 in data sheet) */
//	uint8_t reg8= 0x0;
//	reg8 |= (0x1 << 0); // CLK is 4 MHz
//	reg8 |= (0x1 << 3); // Only send voltage and currents
//	ade7880->regWrite8(HSDC_CFG, reg8);
//
//	// Enable HSDC
//	uint16_t reg16;
//	ade7880->regRead16(CONFIG, reg16);
//	reg16 |= (1<<6); // Enable HSDC
//	ade7880->regWrite16(CONFIG, reg16);
//
//	ade7880->measurement.setCurrentSensorGain(ADE7880Measurement::PHASE_A, 10000.0f);
//	ade7880->measurement.setVoltageSensorGain(ADE7880Measurement::PHASE_A, 1.0f);
//	while(1) {
//		volatile float Irms = ade7880->measurement.Irms(ADE7880Measurement::PHASE_A);
//		volatile float Vrms = ade7880->measurement.Vrms(ADE7880Measurement::PHASE_A);
//		volatile float freq = ade7880->measurement.frequency(ADE7880Measurement::PHASE_A);
//		volatile float per = ade7880->measurement.period(ADE7880Measurement::PHASE_A);
//	}

	/*
	 * Initialising applications
	 */
	BOOT_MSG("Starting application manager");
	apps = new ApplicationManager;
	apps->TimeAverage.setHttpHost("requestbin.net", "/r/1l9mawu1", 80);
//	apps->TimeAverage.init();

	BOOT_MSG("Boot complete");

	Serial.printlnf("");
	Serial.printlnf("Free memory: %d Bytes", System.freeMemory());
	Serial.printlnf("");
}
#pragma GCC pop_options

void loop() {
	if (nextTime > millis()) {
		return;
	}
	apps->TimeAverage.httpPostRequest("Hello, world!");

	nextTime = millis() + 2000;
}

void particleConnectTimeOutCallback(void)
{
    if(!Particle.connected()) {
    	char msg[64];
    	sprintf(msg, "Error could not connect within %d seconds to Particle cloud.", (int) PARTICLE_CLOUD_TO_MS/1000);
    	BOOT_MSG(msg);
        Particle.disconnect();
    } else {
    	BOOT_MSG("Connected to Particle cloud.");
    }
}
