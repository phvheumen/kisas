#ifndef ADE7880_H
#define ADE7880_H

#include "utils.h"
#include "ADE7880RegisterNames.h"
#include "ADE7880Measurement.h"
#include <application.h>
#include <stdint.h>
#include "ADE7880Setting.h"

#define SPI_CLK_FREQ	1000000
#define I2C_ADDR_7b		0x38

class ADE7880
{
public:
	ADE7880(SPIClass * spi, pin_t ss, pin_t reset, bool comVerification = false);
	ADE7880(TwoWire * i2c, pin_t reset, bool comVerification = false);
	ADE7880(SPIClass * spi, TwoWire * i2c, pin_t ss, pin_t reset, bool comVerification = false);
	~ADE7880();

	/*
	 * Initialisation functions
	 */
	void Begin(void);
	void initGPIO(void);
	void initCommunicationBus(void);
	void initADE7880(void);

	/*
	 * SPI read/write functions
	 */
	void SPITransaction(uint16_t RegisterAddress, uint8_t * Data, uint16_t len, bool rw);
	void SPIWrite(uint16_t RegisterAddress, uint8_t * Data, uint16_t len);
	void SPIRead(uint16_t RegisterAddress, uint8_t * Data, uint16_t len);

	/*
	 * High Speed Data Capture functions HSDC
	 */
	static void onHSDCTransferFinished(void);
	static void onHSDCSelect(uint8_t state);

	typedef struct HSDCStruct {
		uint32_t ssAssert;
		uint32_t ssDeAssert;
		uint32_t rxBytesCount;
		uint32_t rxOk;
		uint32_t rxNotOk;
	} HSDCStruct_t;

	static HSDCStruct_t HSDCCounters;

	static uint8_t RxBufferHSDC[64];
	static bool selectState;
	static bool HSDCTransferFinished;


	/*
	 * I2C read/write functions
	 */
	void I2CTransaction(uint16_t RegisterAddress, uint8_t * Data, uint16_t len, bool rw);
	void I2CWrite(uint16_t RegisterAddress, uint8_t * Data, uint16_t len);
	void I2CRead(uint16_t RegisterAddress, uint8_t * Data, uint16_t len);

	/*
	 * Register read/write functions
	 */
	void regWrite(uint16_t RegisterAddress, uint8_t * Data, uint16_t len);
	void regRead(uint16_t RegisterAddress, uint8_t * Data, uint16_t len);

	bool regRead8		(uint16_t RegisterAddresss, uint8_t & Data);
	bool regWrite8		(uint16_t RegisterAddresss, uint8_t Data);

	bool regRead10S		(uint16_t RegisterAddresss, int16_t & Data);
	bool regWrite10S	(uint16_t RegisterAddresss, int16_t Data);

	bool regRead16		(uint16_t RegisterAddresss, uint16_t & Data);
	bool regWrite16		(uint16_t RegisterAddresss, uint16_t Data);

	bool regRead16S		(uint16_t RegisterAddresss, int16_t & Data);

	bool regRead20		(uint16_t RegisterAddresss, uint32_t & Data);

	bool regRead24		(uint16_t RegisterAddresss, uint32_t & Data);
	bool regWrite24		(uint16_t RegisterAddresss, uint32_t Data);

	bool regRead24S		(uint16_t RegisterAddresss, int32_t & Data);
	bool regWrite24S	(uint16_t RegisterAddresss, int32_t Data);

	bool regRead28S		(uint16_t RegisterAddresss, int32_t & Data);
	bool regWrite28S	(uint16_t RegisterAddresss, int32_t Data);

	bool regRead32		(uint16_t RegisterAddresss, uint32_t & Data);
	bool regWrite32		(uint16_t RegisterAddresss, uint32_t Data);

	bool regRead32S		(uint16_t RegisterAddresss, int32_t & Data);
	bool regWrite32S	(uint16_t RegisterAddresss, int32_t Data);

	/*
	 * Verify if communication with AD7880 was successful.
	 * ADE7880 stores address, data and type (R/W) of last communication operation
	 */
	template<typename T>
	bool verifyCommunication(uint16_t RegisterAddress, T Data, bool rw) {
		// Verify register
		uint16_t lastRegisterAddress = 0x0;
		this->regRead(LAST_ADD, (uint8_t *)&lastRegisterAddress, sizeof(lastRegisterAddress));
		lastRegisterAddress = byteswap(lastRegisterAddress); // ADE7880::regRead gives data in communication order, which is big Endian
		if( RegisterAddress != lastRegisterAddress ) {
			return false;
		}

		// Verify operation type (read/write)
		uint8_t lastOperation = 0x0;
		this->regRead(LAST_OP, (uint8_t *)&lastOperation, sizeof(lastOperation));
		uint8_t operation = rw ? 0x35 : 0xCA;
		if( operation != lastOperation ) {
			return false;
		}

		switch (sizeof(Data)) {
		case 1: {
			uint8_t lastData8 = 0x0;
			this->regRead(LAST_RWDATA8, (uint8_t *)&lastData8, sizeof(lastData8));
			if( (uint8_t)Data != lastData8) {
				return false;
			}
		}
			break;
		case 2: {
			uint16_t lastData16 = 0x0;
			// Note: when data is read from this register, byte order is already little Endian
			this->regRead(LAST_RWDATA16, (uint8_t *)&lastData16, sizeof(lastData16));
			lastData16 = byteswap(lastData16);
			if( (uint16_t)Data != lastData16) {
				return false;
			}
		}
			break;
		case 4: {
			uint32_t lastData32 = 0x0;
			this->regRead(LAST_RWDATA32, (uint8_t *)&lastData32, sizeof(lastData32));
			lastData32 = byteswap(lastData32);
			if( (uint32_t)Data != lastData32) {
				return false;
			}
		}
			break;
		default:
			// Other data sizes are not possible.
			return false;
			break;
		}

		// Ending up here, all checks are succeeded
		return true;
	}

	/*
	 * Hardware auxiliary functions
	 */
	void hardwareReset(void);

	/*
	 * Members
	 */
	ADE7880Setting settings; // ADE7880Setting class. Use this class and its methods to change/verify/read settings of the ADE7880
	ADE7880Measurement measurement; // ADE7880 Measurement class. Use this class and its methods to do measurements with the ADE7880

private:
	typedef enum {
		I2C_register,	// Use I2C for reading/writing registers
		SPI_register,	// Use SPI for reading/writing registers
		SPI_HSDC,		// Use I2C for reading/writing registers, and SPI as slave for High Speed Data Capture (HSDC)
	} CommMode;

	SPIClass *	spi;	// SPI object
	TwoWire *	i2c;	// I2C object
	CommMode	commMode; // Which of the three modes op communication operation should be used
	pin_t		ss;		// Chip select pin
	pin_t		reset;	// Reset pin

	bool 		comVerification; // Should every communication transaction be verified?
};

#endif /* ADE7880_H */
