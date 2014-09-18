#include <stdio.h>
#include <stdint.h>
#include "lazey.h"
#include <bcm2835.h>
#include <vector>

struct circle{
	double x;
	double y;
	double dx;
	double dy;
	double r;
};

std::vector<circle> circleList;

int main (int argc, char* argv[]){

	if (argc != 2){
		printf("Usage: ./lazey  <circles>\n");
		return 1;
	}

	int circles = atoi(argv[1]);

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

	int xPos = 50;
	int yPos = 50;
	int dxs = 1;
	int dys = 1;

	for(int x = 0; x < circles; x++){
		circle c;
		c.x = xPos;
		c.y = yPos;
		c.dx = dxs;
		c.dy = dys;
		c.r = 40;
		circleList.push_back(c);
		xPos+=100;
		dxs+=1;
		dys+=1;
		if (xPos > width - 60){
			xPos = 50;
			yPos += 100;
		}
	}

	for(;;){

		for(int x = 0; x < circles; x++){
			LazeyProjector.DrawElipse(circleList[x].x, circleList[x].y, circleList[x].r, 255, 100);
			circleList[x].x += circleList[x].dx;
			circleList[x].y += circleList[x].dy;

		}
			

		for(int x = 0; x < circles; x++){
			if (circleList[x].x < 10 + circleList[x].r){
                                if (circleList[x].dx < 0)
				       circleList[x].dx = -circleList[x].dx;

			}else if (circleList[x].x > width - 10 - circleList[x].r){
				if (circleList[x].dx > 0)
					circleList[x].dx = -circleList[x].dx;
			}	
			if (circleList[x].y < 10 + circleList[x].r){
                                if (circleList[x].dy < 0)
				        circleList[x].dy = -circleList[x].dy;

			}else if (circleList[x].y > height - 10 - circleList[x].r){
                                if (circleList[x].dy > 0)
				        circleList[x].dy = -circleList[x].dy;
			}


			for(int y = x+1; y < circles; y++){
				if (x != y){
					int xDif = circleList[x].x - circleList[y].x;
					int yDif = circleList[x].y - circleList[y].y;
					if (xDif < 0)
						xDif = -xDif;
					if (yDif < 0)
						yDif = -yDif;
					int squared = (xDif * xDif) + (yDif * yDif);
					if (sqrt(squared) < 80){
						circleList[x].dx = -circleList[x].dx;
						circleList[x].dy = -circleList[x].dy;
						circleList[y].dx = -circleList[y].dx;
						circleList[y].dy = -circleList[y].dy;
					}
				}
			}

		}

		LazeyProjector.renderFrame();
		//getchar();

	}

	return 1;
}
