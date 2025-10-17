#include "adxl345.h"

/*
	#######################
	+ 0x40 = 0b0100,0000
	+ 0x80 = 0b1000,0000
*/
#define   MULTI_BYTE_EN		0x40
#define	  READ_OPERATION    0x80

/*
  #######################
  (3)
  + Goal: This function read data from ADXL345 sensor registers usually X,Y,Z (acceleration value)
*/
void adxl_read(uint8_t address, uint8_t * rxdata)
{

	  /*
	     @ - Set read operation
	       + address = 0x32 = 0b0011,0010
	       + READ_OPERATION = 0x80 = 0b1000,0000
	       + OR operation → 0xB2 = 0b1011,0010
	       + This sets bit-7 = 1, which means “I want to READ, not WRITE.”
	  */
	  address |= READ_OPERATION;

	  /*
	     @ - Enable multi-byte
	       + MULTI_BYTE_EN = 0x40 = 0b0100,0000
	       + address = 0xB2 = 0b1011,0010
	       + OR → 0xF2 = 0b1111,0010
	       + This sets bit-6 = 1 → auto-increment address mode.
	  */
	  address |= MULTI_BYTE_EN;

	  /*
	   * + This will pull CS (chip select) = LOW
	   *   → Telling “Hey, I am talking to you"
	   */
	  cs_enable();

      /*
       *  @ - Send address
       *  	+ The first argument is a pointer — uint8_t *data
       *  	+ &address means “the memory address of the variable address.”
       *  	..
       *  	+ The second argument is how many bytes to send — size
       *  	..
       *  	+ You dont need to send address multiple times
       *  	  → because you have enable “multi-byte mode”.
       */
	  spi1_transmit(&address,1);

	  /*
	   *  @ - Read 6 bytes
	   *    + SPI will generate 6 SPI clock cycle
	   *    + For each clock, ADXL345 will send 1 byte:
	   *      -- rxdata[0] = X0 (DATAX0)
			  -- rxdata[1] = X1 (DATAX1)
              -- rxdata[2] = Y0 (DATAY0)
              -- rxdata[3] = Y1 (DATAY1)
              -- rxdata[4] = Z0 (DATAZ0)
              -- rxdata[5] = Z1 (DATAZ1)
	   */
	  spi1_receive(rxdata,6);

	  /*
	   *  @ - Pull CS high → “I’m done talking to you.”
	   *      → ADXL345 stops sending data.
	   */
	  cs_disable();
}

/*
  #########################
  (2nd)
  + Before we can read data (acceleration) from ADXL345 sensor, we must write
    some settings inside sensor to make sensor activate.

  + Goal: We send 1 address (for ADXL345 register address) and
    value to assign inside ADXL345 address using SPI1 communication

*/
void adxl_write (uint8_t address, uint8_t value)
{
  /*
    ------------------------
    + Goals: Provide buffer 2 byte
    + data[0] -> address for register inside ADXL345
    + data[1] -> value to assign into that address
  */
  uint8_t data[2];

  /*
    --------------------------
    @ - Enable multi-byte, place address into buffer

		+ Eg
		+ address = 0x2D = 0b0010,1101 (POWER_CTL_R)
		+ MULTI_BYTE_EN = 0x40 = 0b0100,0000
		+ OR -> 0x6D = 0b0110,1101

		+ Kenapa tak buat 'data[0]=address' ?
		+ Kenapa kita tambah 'MULTI_BYTE_EN'?
		+ Sebab -> ADXL345 ada banyak register bersambung:
		  (DATAX0=0x32, DATAX1=0x33, DATAY0=0x34, DATAY1, ...
		+ Kalau nak baca 6 byte ni satu-satu:
		  Hantar 0x32 >> Hantar 0x33 >> Hantar 0x34
		+ 6 kali hantar address -> Sangat lambat
		+ ...
		+ Tetapi ADXL345 ada ciri khas
		+ Kalau anda aktifkan bit-6 dalam address pertama
		  (eg 0x32) -> Sensor akan lompat ke alamat seterusnya
		  secara automatik selepas setiap bacaan
		+ Anda cuma hantar alamat sekali je, tak perlu 6 kali
		+ Ini dinamakan "auto-increment address"

		+ alamat asal: 0b0010,1101 -> alamat baru(selepas OR): 0b0110,1101
		+ Soalan: Kan alamat dah lain, kita tak salah hantar ke?
		+ Jawapan: ADXL345 tak terus ambil 8-bit sebagai alamat,
		  dia guna 2-bit atas sebagai arahan, 6-bit bawah sebagai alamat sebenar
		+ bit-7 = 1(Read) / 0(Write)
		+ bit-6 = 1(Auto-increment address) / 0(Nothing)
		+ bit-5..bit-0 = address
  */
  data[0] = address|MULTI_BYTE_EN;

  /*
    -------------------------
    @ - Place data into buffer
      + Data yang kita nak hantar ke address register dalam ADXL345
  */
  data[1] = value;

  /*
    -----------------------
    + @ - Pull cs line low to enable slave
  */
  cs_enable();

  /*
    -----------------------
    @ - Transmit data and address into ADXL using SPI
  */
  spi1_transmit(data, 2);

  /*
    ------------------------
    @ - Pull cs line high to disable slave
  */
  cs_disable();

}

/*
  ####################
  (1st)
  + Goal: Activate ADXL so the sensor can start measure acceleration
*/
void adxl_init (void)
{
	/*
	  ----------------------
	  + Enable SPI GPIO
	*/
	spi_gpio_init();

	/*
	  ------------------------
	  + Configure SPI
	*/
	spi1_config();

	/*
	  --------------------
	  + Goals: Set data format range to operate at ±4g

	  + DATA_FORMAT_R = 0x31
	  + FOUR_G = 0x01
	*/
	adxl_write (DATA_FORMAT_R, FOUR_G);

	/*
	  --------------------
	  + Goal: Reset all bits
	*/
	adxl_write (POWER_CTL_R, RESET);

	/*
	  ----------------------
	  + Goal: Configure power control measure bit
	*/
	adxl_write (POWER_CTL_R, SET_MEASURE_B);
}


