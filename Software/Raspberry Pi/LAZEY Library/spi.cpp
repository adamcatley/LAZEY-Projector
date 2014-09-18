#include "spi.h"
#include <stdio.h>

//********** SPI OPEN PORT **********
//***********************************
//***********************************
int SPI_Port::SpiOpenPort (uint8_t BitOrder,  uint8_t DataMode,  uint8_t ClockDivider,  uint8_t ChipSelect,  uint8_t ChipSelectPin,  uint8_t ChipSelectPinState)
{
	if (!bcm2835_init())
		return 0;
	printf("Start\n");
	bcm2835_spi_begin();
	printf("part 2\n");
	bcm2835_spi_setBitOrder(BitOrder);     
	bcm2835_spi_setDataMode(DataMode);               
	bcm2835_spi_setClockDivider(ClockDivider);  
	bcm2835_spi_chipSelect(ChipSelect);                    
	bcm2835_spi_setChipSelectPolarity(ChipSelectPin, ChipSelectPinState);
	connected = true;
	return 1;
}



//************************************
//************************************
//********** SPI CLOSE PORT **********
//************************************
//************************************
int SPI_Port::SpiClosePort ()
{
    if (connected){
	bcm2835_spi_end();
	bcm2835_close();
	return 1;
    }else{
	return -1;
    }
}



//*******************************************
//*******************************************
//********** SPI WRITE & READ DATA **********
//*******************************************
//*******************************************
//data		Bytes to write.  Contents is overwritten with bytes read.
int SPI_Port::SpiWriteAndRead (char *data, int length)
{
	if (connected){
		bcm2835_spi_transfern(data, length);
		return 1;
	} else{
	    return -1;
	}
}
