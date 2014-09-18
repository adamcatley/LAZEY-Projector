#include <stdio.h>
#include <stdint.h>
#include "lazey.h"
#include <bcm2835.h>

int main (int argc, char* argv[]){

	//Creates a display object with an effective resolution of 800x800
	lazey LazeyProjector((uint16_t)800, (uint16_t)800,
			4,
			BCM2835_SPI_BIT_ORDER_MSBFIRST,
			BCM2835_SPI_MODE0,
			BCM2835_SPI_CLOCK_DIVIDER_8,
			BCM2835_SPI_CS0,
			LOW);

	LazeyProjector.extendFramePoints = true;

	for(;;){
		//LazeyProjector.DrawRect(400,400,1,1,0,0);

		LazeyProjector.renderFrame();
	}

	return 1;
}
