#include "lazey.h"
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include "alphabet.h"

lazey::lazey(uint16_t width, uint16_t height, uint8_t _framePin, uint8_t BitOrder,  uint8_t DataMode,  uint8_t ClockDivider,  uint8_t ChipSelectPin,  uint8_t ChipSelectPinState){

	//Check if height or width is bigger, use the larger to calculate the max
	//scale that the picture can be stretched by and still display
	if (width > height){
		scale = UINT16_MAX / width;
	}else{
		scale = UINT16_MAX / height;
	}

	printf("Lazey Library 0.9\n");

	// Setup SPI port
	successSetup = SPI.SpiOpenPort(BitOrder, 
			DataMode, 
			ClockDivider,
			ChipSelectPin,
			ChipSelectPin,
			ChipSelectPinState);

	SetFrameLimit(200);
	
	frameStart = getMS();

	extendFramePoints = false;
	extendFramePointsBlankOnly = true;

	framePinState = false;
	framePin = _framePin;

	rpiGpio.setPinDir(framePin ,mmapGpio::OUTPUT); // set GPIO to output
	rpiGpio.writePinLow(framePin); // Set pin state to low 	

}

lazey::~lazey(){

	SPI.SpiClosePort();
}

long lazey::getMS(){
	struct timespec gettime_now;
	clock_gettime(CLOCK_REALTIME, &gettime_now);
	//printf("nano: %lu\n", gettime_now.tv_nsec);
	return (gettime_now.tv_nsec / 1000000) + (gettime_now.tv_sec * 1000);
}

void lazey::SetFrameLimit(int frameLimit){
	frameRateLimit = frameLimit;
	if (frameLimit){
		frameLimitTime = 1000 / frameLimit;
	} else{
		frameLimitTime = 0;
	}
}

int lazey::pointDif(point p1, point p2){
	int xDif = p1.x - p2.x;
	if (xDif < 0)
		xDif = -xDif;
	int yDif = p1.y - p2.y;
	if (yDif < 0)
		yDif = -yDif;
	if (xDif > yDif)
		return xDif;
	else
		return yDif;
}

bool lazey::renderFrame(){
	
	//Only try to render the frame if everything has been setup correctly
	if (successSetup){

		//Process the list of points to form a output fuffer to send over SPI
		//the outbuffer must have points that are not sppaced too far appart
		std::vector<point> pointsBuff;
		if (points.size() > 0){

		point p2 = points[0];
		p2.brightness = 0x00;
		pointsBuff.push_back(p2);

		for(unsigned int x = 0; x < points.size() -1; x++){
			point p = points[x];
			pointsBuff.push_back(p);
		//	pointsBuff.push_back(p);
		//	p.brightness = points[x].brightness;

			const int PixelGap = 4;

			if ( (extendFramePoints || extendFramePointsBlankOnly) && ((pointDif(p, points[x+1]) > PixelGap))){
				int pointsToAdd = pointDif(p, points[x+1]) / PixelGap;
				double x1 = p.x;
				double y1 = p.y;
				double xDif = (x1 - points[x+1].x) / (pointsToAdd + 1);
				double yDif = (y1 - points[x+1].y) / (pointsToAdd + 1);
				int brightness = points[x+1].brightness;
				x1=x1-xDif;
				y1=y1-yDif;
				//printf("Adding: %d\n", pointsToAdd);
				if (extendFramePoints || (extendFramePointsBlankOnly && brightness == 0)){
					for(int i = 0; i < pointsToAdd; i++, x1=x1-xDif, y1=y1-yDif){
						p.x = x1;
						p.y = y1;
						p.reserved = 0xff;
						p.brightness = brightness;
						pointsBuff.push_back(p);
					}
				}
			}
		}
		}
		//Turn the laser off at the end of the frame
		point endPoint;
		endPoint.x = 0;
		endPoint.y = 0;
		endPoint.reserved = 0xff;
		endPoint.brightness = 0x00;
		pointsBuff.push_back(endPoint);
		pointsBuff.push_back(endPoint);

		//printf("Start Wait (now:%lu, for:%d)...", getMS(), frameLimitTime);
		//Wait to limit the frame rate
		while(getMS() < frameStart + frameLimitTime){
		
		}
		frameStart = getMS();
		//printf("End Wait\n");

		//Gets the number of points in the current frame
		uint16_t numOfPoints = pointsBuff.size();

	//	FILE* pFile = (FILE*)1;
	//	if(pFile != NULL)
	//		pFile = fopen("adamDump.csv", "w");

		//Transmit the number of points
		char numOfPointsChars[2];
		numOfPointsChars[0] = numOfPoints >> 8;
		numOfPointsChars[1] = numOfPoints & 0xFF;

	//	if (pFile != NULL)
	//	fprintf(pFile, "0x%x, 0x%x,\n", numOfPointsChars[0], numOfPointsChars[1]);

		//nanosleep(100);
		SPI.SpiWriteAndRead(numOfPointsChars, 2);
		
		//usleep(500);

		//Transmit the points
		for(unsigned int x = 0; x < numOfPoints; x++){
			//Calculate x and y laser pos
			int16_t xPos = (pointsBuff[x].x * scale) - 32768;
			int16_t yPos = (pointsBuff[x].y * scale) - 32768;
			//Create the 6 byte packet from the point data
			char packet[6];
			packet[0] = pointsBuff[x].brightness;
			packet[1] = pointsBuff[x].reserved;
			packet[2] = xPos >> 8;
			packet[3] = xPos & 0xFF;
			packet[4] = yPos >> 8;
			packet[5] = yPos & 0xFF;
			//printf("%X %X %X %X %X %X \n",packet[0], packet[1], packet[2], packet[3], packet[4], packet[5]); 
			//Send the packet
	//		if (pFile != NULL)
	//		fprintf(pFile, "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n", packet[1], packet[0], packet[3], packet[2], packet[5], packet[4]);
			SPI.SpiWriteAndRead(packet, 6);
					}
		if (framePinState){
			rpiGpio.writePinLow(framePin); 
			framePinState = false;
		}else{
			rpiGpio.writePinHigh(framePin);
			framePinState = true;
		}
	//	if(pFile!=NULL)
	//	fclose(pFile);
	//	pFile = NULL;
		points.clear();
		return true;
	}else{
		return false;
	}
}


/*
//
//	Draws a line from x1,y1 to x2,y2
//
//	x1 - x coordinate for the start of the line
//	y1 - y coordinate for the start of the line
//	x2 - x coordinate for the end of the line
//	y2 - y coordinate for the end of the line
//	brightness - the brightness of the laser
//
*/
void lazey::DrawLine(int x1, int y1, int x2, int y2, uint8_t brightness, int pointsNum){

	//Move the laser to the beginning of the line
	point p;
		//Only move the laser if the new point is different to the last one
	if ((points.size() == 0) || (points[points.size()-1].x != x1 || points[points.size()-1].y != y1)){
		p.x= x1;
		p.y = y1;
		p.reserved = 0xFF;
		p.brightness = 0;
		points.push_back(p);
	}

	if (pointsNum < 3){
		//Move the laser to the end of the line
		p.x= x2;
		p.y = y2;
		p.reserved = 0xFF;
		p.brightness = brightness;
		points.push_back(p);
	} else {
		//Calculate the required dx dy for number oof given points
		int xDif = x1-x2;
		int yDif = y1-y2;
		double dx = xDif/pointsNum;
		double dy = yDif/pointsNum;
		double xPos = x1 - dx;
		double yPos = y1 - dy;

		// Add points along the line, evenly spoaced
		for( int x = 1; x < pointsNum; x++, xPos-=dx, yPos-=dy){
			p.x= xPos;
			p.y = yPos;
			p.reserved = 0xFF;
			p.brightness = brightness;
			points.push_back(p);
		}
	}
}


/*
//
//	Draws a circle with a centre at x,y and a radius r
//
//	x - x coordinate for the centre of the circle
//	y - y coordinate for the centre of the circle
//	r - radius of the circle
//	brightness - the brightness of the laser
//	points - number of points used to draw the circle 0 = auto
//
*/
void lazey::DrawElipse(int x, int y, int r, uint8_t brightness, int pointsNum){
	point p;
	p.x= x;
	p.y = y+r;
	p.reserved = 0xFF;
	p.brightness = 0;
	points.push_back(p);
	p.brightness = brightness;
	int xx,yy;
	const double pi = 3.14159265; 
	double space = (2 * pi) / (pointsNum == 0 ? 0.1 : pointsNum );
	for(double d=0; d<=2*pi; d+=space) //you can play with the value to be added to d
	{
		xx=x+sin(d)*r;
		yy=y+sin(d+(pi/2))*r;
		p.x= xx;
		p.y = yy;
		points.push_back(p);
	}
	p.x = x;
	p.y = y+r;
	points.push_back(p);
	p.brightness = 0x00;
	points.push_back(p);
}

/*
//
//	Draws a rectangle from the top left corner
//
//	x - x coordinate for the top left corner
//	y - y coordinate for the top left corner
//	w - width of the rectangle
//	h - height of the rectangle
//	brightness - the brightness of the laser
//
*/
void lazey::DrawRect(int x, int y, int w, int h, uint8_t brightness, int pointsNum){
	point p;

	//Laser off, move to first point of rectangle
	p.x= x;
	p.y = y;
	p.reserved = 0xFF;
	p.brightness = 0;
	points.push_back(p);


	if (pointsNum < 5){
		//Laser on, draw rectangle
		p.brightness = brightness;
		p.x = x+w;
		p.y = y;
		points.push_back(p);
		p.x = x+w;
		p.y = y+h;
		points.push_back(p);
		p.x = x;
		p.y = y+h;
		points.push_back(p);
		p.x = x;
		p.y = y;
		points.push_back(p);
	}else{
		// Add one since the first point for each line will be removed
		int pointsPerSide = (pointsNum/4)+1;
		DrawLine(x,y, x+w, y, brightness, pointsPerSide);
		DrawLine(x+w,y, x+w, y+h, brightness, pointsPerSide);
		DrawLine(x+w,y+h, x, y+h, brightness, pointsPerSide);
		DrawLine(x,y+h, x, y, brightness, pointsPerSide);
	}
}

/*
//
//	Draws a rectangle from the centre with a rotation about the centre
//
//	x - x coordinate for the centre
//	y - y coordinate for the centre
//	w - width of the rectangle
//	h - height of the rectangle
//	a - rotation of the rectangle in radians
//	brightness - the brightness of the laser
//
*/
void lazey::DrawRect(int cx, int cy, int w, int h, float a, uint8_t brightness, int pointsNum){

	vect2 UL  =  vect2(cx + ( w / 2 ) * cosf( a) - ( h / 2 ) * sinf(a) ,  cy + ( h / 2 ) * cosf(a)  + ( w / 2 ) * sinf(a));
	vect2 UR  =  vect2(cx - ( w / 2 ) * cosf( a) - ( h / 2 ) * sinf(a) ,  cy + ( h / 2 ) * cosf(a)  - ( w / 2 ) * sinf(a));
	vect2 BL =   vect2(cx + ( w / 2 ) * cosf( a) + ( h / 2 ) * sinf(a) ,  cy - ( h / 2 ) * cosf(a)  + ( w / 2 ) * sinf(a));
	vect2 BR  =  vect2(cx - ( w / 2 ) * cosf( a) + ( h / 2 ) * sinf(a) ,  cy - ( h / 2 ) * cosf(a)  - ( w / 2 ) * sinf(a));

	vect2 points[] = {UL,BL,BR,UR};
	DrawPoly(points,4,brightness,pointsNum);
}

/*
//
//	Draws a polygon from the list of points provided
//
//	Points[] - and array of points that make up the polygon
//	NumPoints - the number of points provided in the Points array
//	brightness - the brightness of the laser
//
*/
void lazey::DrawPoly(vect2 Points[], int NumPoints, uint8_t brightness, int pointsNum){
	int pointsPerLine = 0;
	if (pointsNum != 0)
		pointsPerLine = NumPoints / pointsNum;

	//Draw all the lines
	for(int x = 0; x < NumPoints-1; x++){
		DrawLine(Points[x].x,Points[x].y, Points[x+1].x, Points[x+1].y, brightness, pointsPerLine); 
	}
	//Join the end point to the first point
	DrawLine(Points[NumPoints-1].x,Points[NumPoints-1].y,Points[0].x,Points[0].y, brightness, pointsPerLine);
}


/*
//
//	Draws a polygon from the list of points provided and rotates it about the centre point
//
//	Points[] - and array of points that make up the polygon
//	NumPoints - the number of points provided in the Points array
//	Centre - x,y position of the centre of the polygon 
//	angle - number of radians to rotate by
//	brightness - the brightness of the laser
//
*/
void lazey::DrawPoly(vect2 Points[], int NumPoints, vect2 Centre, float angle, uint8_t brightness, int pointsNum){
	vect2 p1(0,0);
	vect2 p2(0,0);

	int pointsPerLine = 0;
	if (pointsNum != 0)
		pointsPerLine = NumPoints / pointsNum;

	//Draw the poly
	for(int x =0; x < NumPoints -1; x++){
		p1 = RotatePoint(Points[x],Centre, angle);
		p2 = RotatePoint(Points[x+1],Centre, angle);
		DrawLine(p1.x,p1.y,p2.x,p2.y,brightness,pointsPerLine);
	}

	//Join the end point to the first point
	p2 = RotatePoint(Points[0],Centre, angle);
	p1 = RotatePoint(Points[NumPoints-1],Centre, angle);
	DrawLine(p1.x,p1.y,p2.x,p2.y,brightness,pointsPerLine);

}

/*
//
//	Draws a equilateral triangle 
//
//	cx - x coordinate of the centre of the triangle with no rotation
//	cy - y coordinate of the centre of the triangle with no rotation
//	w - width of the widest part of the triangle with no rotation
//	h - height of the tallest part of the triangle with no rotation
//	a - number of radians to rotate by
//	rcx - x coordinate of the centre of rotation
//	rcy - y coordinate of the centre of rotation
//	brightness - the brightness of the laser
//
*/
void lazey::DrawTriangle(int cx, int cy, int w, int h, float a, int rcx, int rcy, uint8_t brightness, int pointsNum){
	vect2 Points[] = {vect2((float)cx - (w/2), (float)(cy + (h/2))),vect2((float)cx + (w/2),(float) (cy + (h/2))),vect2((float)cx, (float)(cy - (h/2)))};
	DrawPoly(Points,3,vect2((float)rcx,(float)rcy),a ,brightness, pointsNum);
}

/*
//
//	Draws a text string
//
//	x - x coordinate of the top left of the string
//	y - y coordinate of the top left of the string
//	size - height off the text
//	text - the string to draw, null char terminated
//	brightness - the brightness of the laser
//
*/
void lazey::DrawString(int x, int y, int size, const unsigned char* text, uint8_t brightness){
	int index = 0;
	float scale = size / 10;

	//printf("Drawing string: %s\n", text);

	while( (text[index]) != 0){
		int count  = 0;

		//printf("Pos: %d, Char: %u, Ascii: %c\n", index,  text[index], text[index]);
		//printf("p1: x:%d, y:%d, r:%d, b:%d\n", alphabet[text[index]][count].x, alphabet[text[index]][count].y, alphabet[text[index]][count].reserved, alphabet[text[index]][count].brightness);
		//printf("p1: x:%d, y:%d, r:%d, b:%d\n", alphabet[text[index]][count+1].x, alphabet[text[index]][count+1].y, alphabet[text[index]][count+1].reserved, alphabet[text[index]][count+1].brightness);

		while ( alphabet[text[index]][count].brightness != 0 || alphabet[text[index]][count+1].brightness != 0){
			int xPos = x + (size * index) + (alphabet[text[index]][count].x * scale);
			int yPos = y + (alphabet[text[index]][count].y * scale);
			int bri = brightness;
			int xPos2 = x + (size * index) + (alphabet[text[index]][count+1].x * scale);
			int yPos2 = y + (alphabet[text[index]][count+1].y * scale);

			//printf("Index: %d, Point: x:%d, y:%d\n", count,  xPos, yPos);


			if (alphabet[text[index]][count+1].brightness == 0)
				bri = 0;
			//AddRawPoint(xPos, yPos, 0, bri);
			DrawLine(xPos, yPos, xPos2, yPos2,bri, 0);
			count++;
		}
		index++;
	}
}


/*
//
//	Adds a Raw point to the end of the list of points to be drawn
//
//	x - x coordinate of point
//	y - y coordinate of point
//	reserved - not used
//	brightness - intensity of the laser
//
*/
void lazey::AddRawPoint(uint16_t x, uint16_t y, uint8_t reserved, uint8_t brightness){
	point p;
	p.x= x;
	p.y = y;
	p.reserved = reserved;
	p.brightness = brightness;
	points.push_back(p);
}

/*
//
//	Rotates a point about a centre point
//
//	point - the point to be rotated
//	centre - the location that the point will be rated about
//	angle - angle in radians to rotate by
//
*/
vect2 lazey::RotatePoint(vect2 point, vect2 centre, float angle){
	return vect2(centre.x + ( (centre.x - point.x) ) * cosf( angle) - ((centre.y - point.y)) * sinf(angle) ,  centre.y + ( (centre.y - point.y) ) * cosf(angle)  + ((centre.x - point.x) ) * sinf(angle));
}
