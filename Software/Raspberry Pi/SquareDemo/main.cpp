#include <stdio.h>
#include <stdint.h>
#include "lazey.h"
#include <bcm2835.h>

int main (int argc, char* argv[]){

	if (argc != 2){
		printf("Usage: ./lazey  <animation>\n1. Still Square\n2. Rotating Square\n3. Hello World\n4. 4 Spinning Rectangles\n5. Circle\n");
		return 1;
	}

	int animation = atoi(argv[1]);

	int width = 800;
	int height = 800;


	//Creates a display object with an effective resolution of 800x800
	lazey LazeyProjector((uint16_t)width,(uint16_t)height,
			4,
			BCM2835_SPI_BIT_ORDER_MSBFIRST,
			BCM2835_SPI_MODE0,
			BCM2835_SPI_CLOCK_DIVIDER_8,
			BCM2835_SPI_CS0,
			LOW);
	LazeyProjector.extendFramePoints = true;


	float angle = 0;
	float angle2 = 0;
	float angle3 = 0;
	float angle4 = 0;
	float speed = 0.01;

	int centreX = width /2;
	int centreY = height /2;

	int x1 = width /4;
	int y1 = height /4;

	int rectWidth = width /2;
	int rectHeight = height /2;

	printf("Resolution: %dx%d", width, height);
	printf("X: %d, Y: %d, rectWidth: %d, rectHeight: %d\n", x1, y1, rectWidth, rectHeight);

	if (animation == 1){
		printf("Still rectangle\n");
	}else if (animation == 2){
		printf("Spinning rectangle\n");
	}else if(animation == 3){
		printf("Hello World\n");
	}else if (animation == 4){
		printf("4 Spinning Rectangles\n");
	}else if (animation == 5){
		printf("5. Circle\n");
	}else{
		printf("Invalid option: %d\n", animation);
	}

	for(;;){

		if (animation == 2){
			LazeyProjector.DrawRect(centreX, centreY, rectWidth, rectHeight, angle, 255, 10);
			angle += speed;
		}else if (animation == 1){
			LazeyProjector.DrawRect(x1, y1, rectWidth, rectHeight, 255, 0);
		}else if (animation == 3){
			LazeyProjector.DrawString(10,10,150,(const unsigned char *)"09:58",255);
		}else if (animation == 4){
			LazeyProjector.DrawRect(width/4, height/4, width/5, height/5, angle, 255, 0);
			LazeyProjector.DrawRect(width/4, 3*height/4, width/5, height/5, angle2, 255, 0);
			LazeyProjector.DrawRect(3*width/4, 3*height/4, width/5, height/5, angle3, 255, 0);
			LazeyProjector.DrawRect(3*width/4, height/4, width/5, height/5, angle4, 255, 0);
			angle+=speed;
			angle2-=speed;
			angle3+=speed*2;
			angle4+=speed*10;
		}else if (animation == 5){
			LazeyProjector.DrawElipse(centreX, centreY, width/4, 255, 100);
		}

		LazeyProjector.renderFrame();
		//getchar();

	}

	return 1;
}
