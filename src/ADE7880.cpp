#include "ADE7880.h"

#include "ADE7880Setting.h"
#include "utils.h"
#include "ADE7880RegisterNames.h"

ADE7880::ADE7880(SPIClass * spi, pin_t ss, pin_t reset, bool comVerification) :
settings(this),
measurement(this)
{
	this->spi = spi;
	this->ss = ss; // Note: chip select GPIO is configured by SPI object

	this->i2c = nullptr;
	this->commMode = ADE7880::SPI_register;	// Use SPI for register read/write

	this->reset = reset;
	this->comVerification = comVerification;
}

ADE7880::ADE7880(TwoWire * i2c, pin_t reset, bool comVerification) :
settings(this),
measurement(this)
{
	this->spi = nullptr;
	this->ss = 0x0;

	this->i2c = i2c;
	this->commMode = ADE7880::I2C_register;	// Use SPI for register read/write

	this->reset = reset;
	this->comVerification = comVerification;
}

ADE7880::ADE7880(SPIClass * spi, TwoWire * i2c, pin_t ss, pin_t reset, bool comVerification) :
settings(this),
measurement(this)
{
	this->spi = spi;
	this->ss = ss;

	this->i2c = i2c;
	this->commMode = ADE7880::SPI_HSDC;	// Use I2C for register read/write and SPI as slave for HSDC

	this->reset = reset;
	this->comVerification = comVerification;
}

ADE7880::~ADE7880(void) {

}

void ADE7880::Begin(void) {
	this->initGPIO();
	this->initCommunicationBus();
	this->initADE7880(); // TODO: Not sure to leave here. Revise when implementing variable settings.
}

void ADE7880::initGPIO(void) {
	pinMode(this->reset, OUTPUT);
	digitalWrite(this->reset, HIGH);
}


void ADE7880::initCommunicationBus(void) {
	switch(this->commMode) {
	case ADE7880::I2C_register:
		// I2C initialisation
		if (!this->i2c->isEnabled()) {
			this->i2c->setSpeed(CLOCK_SPEED_100KHZ);
			this->i2c->begin();
			/*
			 * When Wire initialises bus, it immediately goes in locked state.
			 * This is not the behaviour as expected from this library.
			 * TODO: Do further investigation and additionally submit a bug report.
			 */
			this->i2c->reset();
		}
	break;
	case ADE7880::SPI_register:
		// SPI master initialisation
		this->spi->begin(SPI_MODE_MASTER, this->ss);
		this->spi->setClockSpeed(SPI_CLK_FREQ);
		this->spi->setDataMode(SPI_MODE3); // CPOL = 1, CPHA = 1
		digitalWrite(this->ss, HIGH);
	break;
	case ADE7880::SPI_HSDC:
		// I2C initialisation
		if (!this->i2c->isEnabled()) {
			this->i2c->setSpeed(CLOCK_SPEED_100KHZ);
			this->i2c->begin();
			/*
			 * When Wire initialises bus, it immediately goes in locked state.
			 * This is not the behaviour as expected from this library.
			 * TODO: Do further investigation and additionally submit a bug report.
			 */
			this->i2c->reset();
		}

		// SPI slave initialisation
		this->spi->onSelect(ADE7880::onHSDCSelect);
		this->spi->setDataMode(SPI_MODE3);
		this->spi->begin(SPI_MODE_SLAVE, this->ss);

	break;
	}
}

void ADE7880::initADE7880(void) {
	this->hardwareReset();

	/*
	 * Lock the relevant serial bus for communication with the registers
	 */
	if ( this->commMode == ADE7880::SPI_register ) {
		// Toggle SS pin 3 times to select SPI as the serial interface (see p.76 of data sheet)
		delay(2);
		digitalWrite(this->ss, LOW); delay(2); digitalWrite(this->ss, HIGH);
		digitalWrite(this->ss, LOW); delay(2); digitalWrite(this->ss, HIGH);
		digitalWrite(this->ss, LOW); delay(2); digitalWrite(this->ss, HIGH);
		delay(2);

		// Alternatively 3 writes to the 0xEBFF register selects SPI as the serial interface (see p.76 of data sheet)
		/*
		regWrite8(0xEBFF, 0x00);
		regWrite8(0xEBFF, 0x00);
		regWrite8(0xEBFF, 0x00);
		*/

		// Lock the SPI choice by writing an arbitrary value to CONFIG2
		this->regWrite8(CONFIG2, 0x00);
	} else {
		// Lock the I2C choice by setting Bit 1 (I2C_LOCK) of CONFIG2 register
		uint8_t reg8 = 0x0;
		this->regRead8(CONFIG2, reg8);
		reg8 |= (1<<1);
		this->regWrite8(CONFIG2, reg8);
	}


	/*
	 * Data sheet says:
	 *
	 * If the RESET pin is held low while the IC powers up or if the
	 * power-up sequence timing cannot be maintained as per
	 * Figure 35, perform the following sequence of write operations
	 * prior to starting the DSP (setting the RUN register to 0x01), to
	 * ensure that the modulators are reset properly.
	 * 1. 8-bit write: 0xAD is written at Address 0xE7FE.
	 * 2. 8-bit write: 0x14 is written at Address 0xE7E2.
	 * 3. Wait 200 us.
	 * 4. 8-bit write: 0xAD is written at Address 0xE7FE.
	 * 5. 8-bit write: 0x04 is written at Address 0xE7E2.
	 *
	 * Since the software is never sure how well defined the reset pin is, do this procedure in all cases
	 */
	this-regWrite8(0xE7FE, 0xAD);
	this-regWrite8(0xE7E2, 0x14);
	delay(1); // Delay of 1ms is smallest we can do.
	this-regWrite8(0xE7FE, 0xAD);
	this-regWrite8(0xE7E2, 0x04);

	/*
	 * Do all other configuration
	 */

	/* Configure current signal path */
	{
		// PGA
		typedef enum PGA_GAIN {
			PGA_Gain1 = 0,
			PGA_Gain2 = 1,
			PGA_Gain4 = 2,
			PGA_Gain8 = 3,
			PGA_Gain16 = 4,
		} PGA_GAIN_t;
		// 						[ Phase voltage gain 		| Neutral current gain 		  | Phase current gain ]
		this->regWrite16(GAIN, (((PGA_Gain1) << 6) & 0b111) |(((PGA_Gain1) << 3) & 0b111) | (((PGA_Gain1) << 0) & 0b111));
		// HPF
		uint8_t reg8;
		this->regRead8(CONFIG3, reg8);
		reg8 |= (0x1 << 0); // HPFEN
		//reg8 |= (0x1 << 3); // ININTEN
		this->regWrite8(CONFIG3, reg8);
		// Digital Integrator
		this->regWrite24S(DICOEFF, 0xFFF8000); // See p. 27 of datasheet for more details
		uint16_t reg16;
		this->regRead16(CONFIG, reg16);
		reg16 |= (0x1 << 0); // INTEN
		this->regWrite16(CONFIG, reg16);
		// Digital Gain I (See p. 28 of datasheet for more details)
		// Gain = 1 + regValue/2^23
		this->regWrite24S(AIGAIN, -293910);
		this->regWrite24S(BIGAIN, 0x00000);
		this->regWrite24S(CIGAIN, -293910);
		this->regWrite24S(NIGAIN, 0x00000);

		// RMS offsets adjust
		this->regWrite24S(AIRMSOS, -4);
		this->regWrite24S(BIRMSOS, 0x000000);
		this->regWrite24S(CIRMSOS, -4);
		this->regWrite24S(NIRMSOS, 0x000000);

		int32_t reg32s;
		this->regRead24S(AIGAIN, reg32s);
	}

	/* Configure HSDC */
	{
		/* Configure HSDC (See table 52 in data sheet) */
		uint8_t reg8= 0x0;
		reg8 |= (0x1 << 0); // CLK is 4 MHz
		reg8 |= (0x1 << 3); // Only send voltage and currents
		this->regWrite8(HSDC_CFG, reg8);

		// Enable HSDC
		uint16_t reg16;
		this->regRead16(CONFIG, reg16);
		reg16 |= (1<<6); // Enable HSDC
		this->regWrite16(CONFIG, reg16);
	}

	// Start the DSP by setting Run = 1
	uint16_t reg = 0x0001;
	this->regWrite16(RUN, reg);
	this->regWrite16(RUN, reg);
	this->regWrite16(RUN, reg);
}

void ADE7880::SPITransaction(uint16_t RegisterAddress, uint8_t * Data, uint16_t len, bool rw) {
	uint8_t _rw_byte = rw ? 0x1 : 0x0; 						// Read (1) / Write (0)
	uint16_t _RegisterAddress = byteswap(RegisterAddress);	// ARM is little Endian, SPI communication is big Endian

	digitalWrite(this->ss, LOW);
	this->spi->transfer(&_rw_byte, NULL, 1, NULL);
	this->spi->transfer(&_RegisterAddress, NULL, 2, NULL);
	this->spi->transfer(rw ? NULL : Data, rw ? Data : NULL, len, NULL);
	digitalWrite(this->ss, HIGH);
}

void ADE7880::SPIWrite(uint16_t RegisterAddress, uint8_t * Data, uint16_t len) {
	this->SPITransaction(RegisterAddress, Data, len, false);
}

void ADE7880::SPIRead(uint16_t RegisterAddress, uint8_t * Data, uint16_t len) {
	this->SPITransaction(RegisterAddress, Data, len, true);
}

void ADE7880::onHSDCTransferFinished(void) {
	return;
}
void ADE7880::onHSDCSelect(uint8_t state) {

	volatile float instantaneous[7] = { 0 };
	volatile int32_t instantaneousHex[7] = { 0x0 };

	if ( state == 0 ) { // Slave chip select is high
		ADE7880::HSDCCounters.ssDeAssert += 1;
		// Stop ongoing receive sequence
		SPI.transferCancel();
		ADE7880::HSDCCounters.rxBytesCount = SPI.available();
		if (SPI.available() == 28 ) {
			ADE7880::HSDCCounters.rxOk += 1;
		} else {
			ADE7880::HSDCCounters.rxNotOk += 1;
		}
		// Start receiving
		SPI.transfer(NULL, ADE7880::RxBufferHSDC, sizeof(ADE7880::RxBufferHSDC), ADE7880::onHSDCTransferFinished);
	} else { // Slave chip select is low
		ADE7880::HSDCCounters.ssAssert += 1;
		// Data comes too fast after chip select assertion.
		// Since the slave receives data periodically at 8 kHz
		// Start and stop receiving is both done on the chip select
		// de-assert.
	}
	return;
}
bool ADE7880::selectState = false;
bool ADE7880::HSDCTransferFinished = false;
uint8_t ADE7880::RxBufferHSDC[64] = { 0x0 };
ADE7880::HSDCStruct_t ADE7880::HSDCCounters = { 0 };

void ADE7880::I2CTransaction(uint16_t RegisterAddress, uint8_t * Data, uint16_t len, bool rw) {
	uint16_t _RegisterAddress = byteswap(RegisterAddress);	// ARM is little Endian, I2C communication is big Endian

	if(rw) {
		this->i2c->beginTransmission(I2C_ADDR_7b);
		this->i2c->write((uint8_t *)&_RegisterAddress, (size_t)sizeof(_RegisterAddress));
		this->i2c->endTransmission(false);

		this->i2c->requestFrom(I2C_ADDR_7b, len);
		uint8_t bytesAvailable = this->i2c->available();
		this->i2c->readBytes((char *)Data, bytesAvailable);
	} else {
		this->i2c->beginTransmission(I2C_ADDR_7b);
		this->i2c->write((uint8_t *)&_RegisterAddress, (size_t)sizeof(_RegisterAddress));
		this->i2c->write((uint8_t *)Data, len);
		this->i2c->endTransmission(true);
	}
}

void ADE7880::I2CWrite(uint16_t RegisterAddress, uint8_t * Data, uint16_t len) {
	this->I2CTransaction(RegisterAddress, Data, len, false);
}

void ADE7880::I2CRead(uint16_t RegisterAddress, uint8_t * Data, uint16_t len) {
	this->I2CTransaction(RegisterAddress, Data, len, true);
}

void ADE7880::regWrite(uint16_t RegisterAddress, uint8_t * Data, uint16_t len) {
	if( this->commMode == ADE7880::SPI_register ) {
		// Use SPI for register communication
		this->SPIWrite(RegisterAddress, Data, len);
	} else {
		// Use I2C for register communication
		this->I2CWrite(RegisterAddress, Data, len);
	}
}

void ADE7880::regRead(uint16_t RegisterAddress, uint8_t * Data, uint16_t len) {
	if( this->commMode == ADE7880::SPI_register ) {
		// Use SPI for register communication
		this->SPIRead(RegisterAddress, Data, len);
	} else {
		// Use I2C for register communication
		this->I2CRead(RegisterAddress, Data, len);
	}
}

void ADE7880::hardwareReset(void) {
	digitalWrite(this->reset, LOW);
	delay(1);
	digitalWrite(this->reset, HIGH);

	Wire.begin();
}

bool ADE7880::regRead8(uint16_t RegisterAddresss, uint8_t & Data) {
	this->regRead(RegisterAddresss, (uint8_t *)&Data, sizeof(Data));
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, true);
	} else {
		return true;
	}
}
bool ADE7880::regWrite8(uint16_t RegisterAddresss, uint8_t Data) {
	this->regWrite(RegisterAddresss, (uint8_t *)&Data, sizeof(Data));
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, false);
	} else {
		return true;
	}
}

bool ADE7880::regRead10S(uint16_t RegisterAddresss, int16_t & Data) {
	// TODO: Implement, unclear if truly signed or just artificially. See table 45. For the registers that apply
	return false;
}
bool ADE7880::regWrite10S(uint16_t RegisterAddresss, int16_t Data) {
	// TODO: Implement, unclear if truly signed or just artificially. See table 45. For the registers that apply
	return false;
}

bool ADE7880::regRead16(uint16_t RegisterAddresss, uint16_t & Data) {
	this->regRead(RegisterAddresss, (uint8_t *)&Data, sizeof(Data));
	Data = byteswap(Data);
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, true);
	} else {
		return true;
	}
}
bool ADE7880::regWrite16(uint16_t RegisterAddresss, uint16_t Data) {
	uint16_t TxData = byteswap(Data);
	this->regWrite(RegisterAddresss, (uint8_t *)&TxData, sizeof(TxData));
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, false);
	} else {
		return true;
	}
}

bool ADE7880::regRead16S(uint16_t RegisterAddresss, int16_t & Data) {
	this->regRead(RegisterAddresss, (uint8_t *)&Data, sizeof(Data));
	Data = byteswap(Data);
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, true);
	} else {
		return true;
	}
}

bool ADE7880::regRead20(uint16_t RegisterAddresss, uint32_t & Data) {
	/*
	 * Unsigned integers of 20 bit are always zero padded.
	 * Just call ADE7880::regRead32
	 */
	bool ret = this->regRead32(RegisterAddresss, Data);
	Data &= 0xFFFFF; // Mask 20 bits, just in case.
	return ret;
}

bool ADE7880::regRead24(uint16_t RegisterAddresss, uint32_t & Data) {
	/*
	 * Unsigned integers of 24 bit are always zero padded.
	 * Just call ADE7880::regRead32
	 */
	bool ret = this->regRead32(RegisterAddresss, Data);
	Data &= 0xFFFFFF; // Mask 24 bits, just in case
	return ret;
}
bool ADE7880::regWrite24(uint16_t RegisterAddresss, uint32_t Data) {
	/*
	 * Unsigned integers of 24 bit are always zero padded.
	 * Just call ADE7880::regWrite32
	 */
	Data &= 0xFFFFFF; // Mask 24 bits, just in case
	return this->regWrite32(RegisterAddresss, Data);
}

// TODO: register format harmonics not clear at the moment
bool ADE7880::regRead24S(uint16_t RegisterAddresss, int32_t & Data) {
	/*
	 * Signed integers of 24 bit have the sign symbol on the 24th bit. To handle this value
	 * In a signed 32 bit integer. Extend this sign bit to the 32th bit. This is true for both
	 * ZP and ZPSE registers
	 */
	bool ret = this->regRead32S(RegisterAddresss, Data);
	if ( Data & (1<<23) ) {
		Data |= 0xFF000000;
	} else {
		Data &= ~(0xFF000000);
	}

	return ret;
}
bool ADE7880::regWrite24S(uint16_t RegisterAddresss, int32_t Data) {
	/*
	 * Just mask 28 bits.
	 * Now the integer is stored as 24 bits sign extended to 28 bits and zero padded to 32 bits.
	 */
	Data &= 0xFFFFFFF; // Mask 28 bits.
	return this->regWrite32S(RegisterAddresss, Data);
}

bool ADE7880::regRead28S(uint16_t RegisterAddresss, int32_t & Data) {
	/*
	 * Signed integers of 28 bit have the sign symbol on the 28th bit. To handle this value
	 * In a signed 32 bit integer. Extend this sign bit to the 32th bit.
	 */
	bool ret = this->regRead32S(RegisterAddresss, Data);
	if ( Data & (1<<27) ) {
		Data |= 0xF0000000;
	} else {
		Data &= ~(0xF0000000);
	}

	return ret;
}
bool ADE7880::regWrite28S(uint16_t RegisterAddresss, int32_t Data) {
	/*
	 * Just mask 28 bits.
	 * Now the integer is stored as 28 bits signed integer zero padded to 32 bits.
	 */
	Data &= 0xFFFFFFF; // Mask 28 bits.
	return this->regWrite32S(RegisterAddresss, Data);
}

bool ADE7880::regRead32(uint16_t RegisterAddresss, uint32_t & Data) {
	this->regRead(RegisterAddresss, (uint8_t *)&Data, sizeof(Data));
	Data = byteswap(Data);
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, true);
	} else {
		return true;
	}
}
bool ADE7880::regWrite32(uint16_t RegisterAddresss, uint32_t Data) {
	uint32_t TxData = byteswap(Data);
	this->regWrite(RegisterAddresss, (uint8_t *)&TxData, sizeof(TxData));
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, false);
	} else {
		return true;
	}
}

bool ADE7880::regRead32S(uint16_t RegisterAddresss, int32_t & Data) {
	this->regRead(RegisterAddresss, (uint8_t *)&Data, sizeof(Data));
	Data = byteswap(Data);
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, true);
	} else {
		return true;
	}
}
bool ADE7880::regWrite32S(uint16_t RegisterAddresss, int32_t Data) {
	int32_t TxData = byteswap(Data);
	this->regWrite(RegisterAddresss, (uint8_t *)&TxData, sizeof(TxData));
	if(this->comVerification) {
		return this->verifyCommunication(RegisterAddresss, Data, false);
	} else {
		return true;
	}
}
