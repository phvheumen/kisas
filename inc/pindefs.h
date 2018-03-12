#ifndef PINDEFS_H
#define PINDEFS_H

    //#########   pin definitions   ########
    #if defined(__AVR__)
        // AD7880 Datalines
        //We use I2C by default
        #define MOSI_SDA 11 //A4
        #define MISO_HSD 12
        #define SCLK_SCL 13 //A5
        #define CF3_HSCLK 6
        #define CF2_HREADY 5
        #define CF1 4
        #define IRQ0INV 2
        #define IRQ1INV 3
        #define SS_HSA 9
        #define RESETINV 8
        #define PM1 A1
        #define PM0 A0
        
        // Only needed if DIAG is enabled
        #define DIAGPORT_RX     0 //4
        #define DIAGPORT_TX     1 //5
        
        #define FATAL_LED       13 //6
        #define FATAL_LED_OFF   LOW
        #define FATAL_LED_ON    HIGH
    #elif defined (__arm__)
		/* SPI bus to ADE7880 */
        #define MOSI_SDA 	A5
        #define MISO_HSD 	A4
        #define SCLK_SCL 	A3
        #define SS_HSA	 	A2

		/* SPI bus to SD card */
        #define MOSI_SD 	D2
        #define MISO_SD 	D3
        #define SCLK_SD 	D4
        #define SS_SD	 	D5

		/* GPIO to ADE7880 */
		#define RESETINV 	A1
        #define CF3_HSCLK 	D0
        #define CF2_HREADY 	D1
        #define CF1 		B0 //not used

		/* Interrupt lines from ADE7880 */
        #define IRQ0INV 	B0 //not used
        #define IRQ1INV 	B0 //not used

		/* Power mode selection ADE7880 */
        #define PM1 		B0 //not used
        #define PM0 		B0 //not used
    #endif
#endif /* PINDEFS_H */
