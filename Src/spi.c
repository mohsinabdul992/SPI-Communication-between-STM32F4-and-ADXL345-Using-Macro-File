/*
 *  #####################
 *  @ - Header and Macro Definition
 *    + import header file (.h file) (contain 'function declaration'
 *      and 'definition').
 */
#define STM32F411xE
#include "stm32f4xx.h"

#include "spi.h"

/*
 *  #####################
 *  @ - Why we don't need to declare address for 'Module'
 *      and 'Register'.
 *    + because 'STM32 HAL' / 'CMSIS driver' provided 'structure register'
 *      in form of "typedef struct" and "#define".
 */

/*
 *  #####################
 *  @ - Defining bit
 */


#define SPI1EN (1U << 12)
#define GPIOAEN (1U << 0)

#define SR_TXE (1U << 1)
#define SR_RXNE (1U << 0)

#define SR_BSY (1U << 7)

/*
	########################
	@ - Initialize GPIOA pin for SPI communication
*/
void spi_gpio_init(void) {

	/*
	   ---------------------------
	   @2 - Enable clock access to GPIOA
	      + We set "GPIOAEN bit" inside "AHB1ENR register".
	*/
	RCC->AHB1ENR |= GPIOAEN;

	/*
		---------------------------
		@2 - Set PA5, PA6, PA7 to have 'alternate function mode'
		   + We assign value '10' to (bit-11, bit-10) (bit-13, bit-12)
		     and (bit-15, bit-14) inside MODE register.
		   ..
		   + 00 = Input Mode
		   + 01 = Output Mode
		   + 10 = Alternate Function
	*/

	// PA5
	GPIOA->MODER &= ~(1U << 10); // We set bit-10 = 0
	GPIOA->MODER |= (1U << 11);  // We set bit-11 = 1

	// PA6
	GPIOA->MODER &= ~(1U << 12);
	GPIOA->MODER |= (1U << 13);

	// PA7
	GPIOA->MODER &= ~(1U << 14);
	GPIOA->MODER |= (1U << 15);

	/*
		---------------------------
		@2 - Set PA9 Mode to have 'Output Mode' (01)
		   + We assign '01' to (bit-19, bit-18) inside Mode register
		   ..
		   ..
		   + 00 = Input Mode
		   + 01 = Output Mode
		   + 10 = Alternate Function
	*/
	GPIOA->MODER |= (1U << 18);
	GPIOA->MODER &= ~(1U << 19);

	/*
	   ---------------------------
		@2 - Set PA5, PA6, PA7 to have Alternate function = SPI1 (0101)
		   + We assign '0101' value (bit-23, bit-22, bit-21, bit-20) = pin PA5 |
		     (bit-27, bit-26, bit-25, bit-24) = pin PA6 |
		     (bit-31, bit-30, bit-29, bit-28) = pin PA7
		     into 'Alternate Function Register (AFR)
		   ..
		   + Go to reference table
	*/

	// PA5
	GPIOA->AFR[0] |= (1U << 20);
	GPIOA->AFR[0] &= ~(1U << 21);
	GPIOA->AFR[0] |= (1U << 22);
	GPIOA->AFR[0] &= ~(1U << 23);

	// PA6
	GPIOA->AFR[0] |= (1U << 24);
	GPIOA->AFR[0] &= ~(1U << 25);
	GPIOA->AFR[0] |= (1U << 26);
	GPIOA->AFR[0] &= ~(1U << 27);

	// PA7
	GPIOA->AFR[0] |= (1U << 28);
	GPIOA->AFR[0] &= ~(1U << 29);
	GPIOA->AFR[0] |= (1U << 30);
	GPIOA->AFR[0] &= ~(1U << 31);
}

/*
	########################
	@ - Configure SPI1 module to have: master | 8-bit | baud fPCLK/4 |
	    software NSS
*/
void spi1_config(void) {

	/*
		---------------------------
		@2 - Enable clock access to SPI1 module
		   + We enable 'SPI1EN bit' inside 'APB2ENR register'
	*/
	RCC->APB2ENR |= SPI1EN;

	/*
		 ---------------------------
		 @2 - Set clock to fPCLK/4
			..
		    + fPCLK = Peripheral Clock Frequency (the clock that feeds SPI1)
			..
		    + fPCLK = 16 MHz
		    + fPCLK/4 = 4 MHz SPI speed
			..
		    + BR=001 -> SPI Clock = fPCLK/4
	*/
	SPI1->CR1 |= (1U << 3);  // BR0=1
	SPI1->CR1 &= ~(1U << 4); // BR1=0
	SPI1->CR1 &= ~(1U << 5); // BR2=0

	/*
		 ---------------------------
		 @2 - Set CPOL = 1 and CPHA = 1
			..
		 	+ This two bits control how data is
		      aligned with the clock signal (SCK).
			..
		 @3 - CPOL
		    + CPOL = Clock Polarity
		    + CPOL controls what the clock looks like when it’s not moving.
		    ..
		    + CPOL=0
		    + The clock line is LOW (0 volts) when idle (not sending).
		    + When SPI starts, it begins by going UP.
		    ..
		    + CPOL = 1
		    + The clock line is HIGH (3.3 V) when idle.
		    + When SPI starts, it begins by going DOWN.
            ..
         @3 - CPHA
		    + CPHA = Clock Phase Control
		    + CPHA controls when (on which clock edge) data is read and
			  written during SPI communication.
		    ..
		    + CPHA = 0
		    + Read on first clock edge
		    + First edge = rising edge → READ data here
		    + Second edge = falling edge → CHANGE data here
		    ..
		    + change = Shift the next bit out on the MOSI line
		    ..
		    + CPHA = 1
		    + Read on second clock edge
		    + first edge is used just to shift data out
		    + second edge is used to read it
            ..
		@3  - [CPOL, CPHA] = (1,1)
		    + Meaning -- Data captured on second edge -- Clock is high when idle

	*/
	SPI1->CR1 |= (1U << 0);
	SPI1->CR1 |= (1U << 1);

	/*
		 ---------------------------
		 @2 - Enable full duplex
		    + RXONLY=0 → full duplex
			..
		    + Each clock tick : shift one bit out (MOSI) | and one bit in (MISO)
	*/
	SPI1->CR1 &= ~(1U << 10);

	/*
		 ---------------------------
		 @2 - Set MSB first
		    + We choose SPI to send bits starting from the "Most Significant Bit"
			+ LSBFIRST=0 -> MSB first
	*/
	SPI1->CR1 &= ~(1U << 7);

	/*
		 ---------------------------
		 @2 - Set mode to Master
			+ MSTR=1 -> Master mode
		 	+ Microcontroller (STM32) to act as SPI master
		    + Microcontroller will generate clock signal on PA5.
	*/
	SPI1->CR1 |= (1U << 2);

	/*
		---------------------------
		@2 - Data size = 8-bit
		   + DFF = 0 -> 8-bit data
	*/
	SPI1->CR1 &= ~(1U << 11);

	/*
		 ---------------------------
		 @2 - Select software slave management -- by setting SSM=1
		    -- and SSI=1
			..
		 @3 - NSS / SS / CS
		 	+ NSS = Slave Select / Chip Select
			+ It is a signal line that tell slave device:
			  hey the master wants to talk to you, wake up and listen
			..
	     @3 - SSM
		    + SSM = software slave management
		    + let your code control NSS, instead of hardware
		    ..
		 	+ SSM = 1
		    + SPI ignores the physical NSS pin
		 	+ SPI uses the internal SSI bit
			..
		 @4 - SSI
		    + SSI bit = Internal Slave Select
		    ..
		 @4 -
		    + SSI=1 -> NSS=1
		    + SPI stay active
	*/
	SPI1->CR1 |= (1U << 8);
	SPI1->CR1 |= (1U << 9);

	/*
		 ---------------------------
		 @2 - Enable SPI module
		 	+ We set 'bit-6' inside 'Control Register'
	*/
	SPI1->CR1 |= (1U << 6);
}

/*
	########################
	@2 - Send bytes out over SPI1
*/
void spi1_transmit(uint8_t *data, uint32_t size) {

	/*
	  --------------------------
	  @2 - i
		 + "i" is a counter into "data" array.
		 + "i=0" -> "data[0]"
	*/
	uint32_t i=0;
	uint8_t temp;

	while(i < size) {

		/*
			--------------------------
			@2 - Check if "Data Register" is empty or not

			   + SPI1->SR
			   + Status Register for SPI1
			   + It has many flag bit
			   ..
			   + TXE = Transmit Buffer Empty
			   ..
			   + TXE = 0
			   + Buffer TX (Data Register) is NOT EMPTY yet
			   ..
			   + TXE = 1
			   + Buffer TX (Data Register) is EMPTY

			   + SPI1->SR & SR_TXE
			   + Use "Bitwise AND" to check only "TXE bit"
			   + It will ignore other bits.

			   + buffer not empty -> SR_TXE=0 -> while(!(0)) -> while(1)
		*/
		while(!(SPI1->SR & SR_TXE)) {}

		/*
			---------------------------
			@2 -
			   + Goal: We assign data inside data[i] into "data register".
		*/
		SPI1->DR = data[i];
		i++;
	}

	/*
		 ---------------------------
		 @2 -
		    + Goal: Check if "Data Register" is empty or not
		    + (Same as code -> 'while(!(SPI1->SR & SR_TXE)) {}')
	*/
	while(!(SPI1->SR & (SR_TXE))) {}

	/*
		----------------------------
		@2 -
	       + SPI1->SR
	       + This is "Status Register" for Module SPI1.
           ..
        @3 -
	       + SR_BSY = Busy flag
	       ..
	       + SR_BSY = 1
	       + SPI still busy - there are bits that shift-in
		     and shift-out
		   ..
		   + SR_BSY = 0
		   + No bit transfer is occuring now
           ..
		   + No bit transfer occured -> SR_BSY=0 -> while(0)
	*/
	while(SPI1->SR & (SR_BSY)) {}

	/*
		----------------------------
		@2 -
		   + This is "full-duplex".
	       + SPI has two lines: MOSI (Master → Slave)
	         and MISO (Slave → Master).
		   + Every clock pulse, two thing happens :
		     Master shift-out 1-bit (MOSI) &
		     Slave shift-out back 1-bit (MISO).
		   ..
		@2 -
	       + code -> temp = SPI1->DR;
	       + We read data from "Data Register"
	         and stored into "temp" variable.
           ..
	       + temp = SPI1->SR;
	       + We read from "Status Register"
	         and stored into "temp variable".
           ..
        @2 -
		   + SPI is full-duplex (Every time you transmit,
		     the SPI also receives a byte).
		   + If you never read that received byte, the next received byte
		     overrun it.
		   //
		@3 - Analogy :
		   + Your SPI has a small inbox (receive buffer).
		   + A new byte arrives before you read the old byte in the inbox.
		   + The new byte pushes past the old one → the old data is lost.
		   + The SPI raises the OVR flag to say: “Hey, you didn’t read me
		     in time!”
		   //
        @3 -
		   + Clean up the receive side to avoid error
           + How to clear OVR on STM32
		   + You must read two registers in this exact order:
		     1st-Read "Data Register", 2nd-Read "Status Register"
	*/
	temp = SPI1->DR;
	temp = SPI1->SR;
}

/*
	#############################
	@ -
	  + Goal: Receive bytes from SPI slave
*/
void spi1_receive(uint8_t *data, uint32_t size) {

	while(size) {
		/*
			---------------------------------
			@2 - Send dummy data
			   + We write "0x00" into "Data Register SPI".
			   + Master mula menjana 8 nadi clock dan hantar 0x00.
			   + Pada masa sama, MISO dari slave mengisi RX.
		*/
		SPI1->DR = 0;

		/*
		  ---------------------------------
		  @2 - Wait for RXNE to be set
		     //
			 + SPI1->SR & (SR_RXNE)
		     + We uses "Bitwise AND operation" to check SR_RXNE bit and
		       ignore other bit.
		     //
		     + SR_RXNE = 1
		     + All 8-bits have been received
		     //
		     + 'All 8-bits received' -> SR_RXNE=1 -> while(!(1))
		       -> while(0)

		*/
		while(!(SPI1->SR & (SR_RXNE))) {}

		/*
			---------------------------
			@2 - Read data from data register
               //
			   + We assign data from "Data Register" (that we received
				 from SPI) into "*data" pointer
            @3 -
			   + "*data++" = increment pointer
			   + the pointer now will point the next address
		*/
		*data++ = (SPI1->DR);

		size--;
	}
}

/*
	#########################
	+ Goal: Select/Activate the SPI slave by pulling Chip-Select (CS)/Slave Select (SS) line to LOW.
*/
void cs_enable(void) {
	/*
		+ ODR = Output Data Register
		+ '&= ~(1U << 9)' = 'clear bit-9' = PA9=0
		+ CS pin low.
	*/
	GPIOA->ODR &= ~(1U << 9);
}

/*
	#########################
	+ (Same sa previous)
	+ Pull high to disable
*/
void cs_disable(void) {

	GPIOA->ODR |= (1U << 9);
}

