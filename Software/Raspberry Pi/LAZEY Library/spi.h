#include <bcm2835.h>
#include <stdint.h>

class SPI_Port
{

public:

SPI_Port(){

	connected = false;

}

~SPI_Port(){

}


int SpiOpenPort (uint8_t BitOrder,  uint8_t DataMode,  uint8_t ClockDivider,  uint8_t ChipSelect,  uint8_t ChipSelectPin,  uint8_t ChipSelectPinState);


//************************************
//********** SPI CLOSE PORT **********
//************************************
//************************************
int SpiClosePort ();

//*******************************************
//*******************************************
//********** SPI WRITE & READ DATA **********
//*******************************************
//*******************************************
//data		Bytes to write.  Contents is overwritten with bytes read.
int SpiWriteAndRead (char *data, int length);


private:

bool connected;

};

