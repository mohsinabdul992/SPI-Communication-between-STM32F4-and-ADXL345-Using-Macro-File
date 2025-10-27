#include <stdio.h>
#include <stdint.h>

#define STM32F411xE
#include "stm32f4xx.h"

#include "adxl345.h"

int16_t x,y,z;
double xg, yg, zg;

/*
 *  ------------------
 *  @ -
 *    + This is place to store measured acceleration data from ADXL345
 */
uint8_t data_rec[6];

int main(void){

	adxl_init();

	while(1)	{
	  adxl_read(DATA_START_ADDR,data_rec);

	  /*
	   *  @ - Combine 2 '8 bit data' into 1 '16 bits data'
	   *    ..
	   *    + "<<" means bit shift-left
	   *    ..
	   *    + Eg
	   *    +2  (data_rec[1] << 8) | data_rec[0]
			+2	= (0x01 << 8) | 0xF4
			+2	= (0b0000,0001 << 8) | 0b1111,0100
			+2	// << means shift left
			+2	= 0b0000,0001,0000,0000 | 0b1111,0100
			+2	= 0b0000,0001,1111,0100
			+2	= 0x01F4
			+2	= 500 (in decimal)
	   */
		x = ((data_rec[1]<<8)|data_rec[0]);
		y = ((data_rec[3]<<8)|data_rec[2]);
		z = ((data_rec[5]<<8)|data_rec[4]);

		/*
		 *  @ - why multiply value from sensor with "0.0078"
		 *    ..
		 *    + RESOULTION for ADXL345 sensor = 10 bits.
		 *    + This is from datahseet.
		 *    ..
		 *    + sensor MEASUREMENT RANGE = -4g ~> 4g = 8
		 *    ..
		 *    + STEP SIZE = [RANGE] / [2**RESOLUTION]
		 *    +2 STEP SIZE = [8] / [1024]
		 *    +2 STEP SIZE = 0.0078125 ≈ 0.0078
		 *
		 *	  ---------------------------
		 *	  @ - what is "STEP SIZE" meaning here
		 *	    + How much change in acceleration (measurement) is represented
		 *	      by 1 digital count
		 *	    + 1 'digital count' / 'digital value measured by ADC' =
		 *	      0.0078 =g in real-world measurement
		 */
		xg = (x * 0.0078);
		yg = (y * 0.0078);
		zg = (z * 0.0078);
  }
}
