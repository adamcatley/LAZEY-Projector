//#pragma once
//#ifndef _LAZEY_
//#define _LAZEY_


#include <stdint.h> // For unit8_t and unint16_t
#include <vector> // for vector to store points
#include <math.h> // for sin
#include <time.h>
#include "mmapGpio.h"
#include "spi.h"



/*
//
//	Data structure to store an individual point to be sent to the control board
//
//	x - 16bit x coordinate of point
//	y - 16bit y coordinate of point
//	reserved - not used
//	brightness - 8bit laser brightness
//
*/
struct point{
	uint16_t x;
	uint16_t y;
	uint8_t reserved;
	uint8_t brightness;
	point(){}
	point(uint16_t _x, uint16_t _y, uint8_t _reserved, uint8_t _brightness) : x(_x), y(_y), reserved(_reserved), brightness(_brightness){}
};

/*
//
//	A 2 16bit element vector
//
*/
struct vect2{
	uint16_t x;
	uint16_t y;
	vect2(uint16_t _x,uint16_t _y) : x(_x), y(_y) {}
};

class lazey{



public:

	/*
	//
	//	Creates the laser display object
	//
	//	width - width of the display object (eg pixels)
	//	height - height of the display object (eg pixels)
	//	_framPin - the GPIO pin to too toggled at the end of a frame sent over SPI
	//	BitOrder - The order in which the bits are oupt over SPI eg. LSB or MSB
	//	DataMode - Set CPOL and CPHA
	//	ClockDivider - Set the SPI clock speed 
	//	ChipselectPin - Set the pin used as chip select (slave select)
	//	ChipSelectPinState - Sets the CS pin to be active low or high
	//
	*/	

	lazey(uint16_t width, uint16_t height, uint8_t _framePin, uint8_t BitOrder,  uint8_t DataMode,  uint8_t ClockDivider, uint8_t ChipSelectPin,  uint8_t ChipSelectPinState);


	/*
	//
	//	Disposes of the laser display object
	//
	*/

	~lazey();

	/*
	//
	//	Sends the current frame to the controller/laser
	//
	*/

	bool renderFrame();

	/*
	//
	//	Draws a line from x1,y1 to x2,y2
	//
	//	x1 - x coordinate for the start of the line
	//	y1 - y coordinate for the start of the line
	//	x2 - x coordinate for the end of the line
	//	y2 - y coordinate for the end of the line
	//	brightness - the brightness of the laser
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void DrawLine(int x1, int y1, int x2, int y2, uint8_t brightness, int pointsNum);


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
	void DrawElipse(int x, int y, int r, uint8_t brightness, int pointsNum = 0);

	/*
	//
	//	Draws a rectangle from the top left corner
	//
	//	x - x coordinate for the top left corner
	//	y - y coordinate for the top left corner
	//	w - width of the rectangle
	//	h - height of the rectangle
	//	brightness - the brightness of the laser
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void DrawRect(int x, int y, int w, int h, uint8_t brightness, int pointsNum);

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
	//	pointsNum - The number of points used to rener the line, 0 = min
	//	
	*/
	void DrawRect(int cx, int cy, int w, int h, float a, uint8_t brightness, int pointsNum);

	/*
	//
	//	Draws a polygon from the list of points provided
	//
	//	Points[] - and array of points that make up the polygon
	//	NumPoints - the number of points provided in the Points array
	//	brightness - the brightness of the laser
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void DrawPoly(vect2 Points[], int NumPoints, uint8_t brightness, int pointsNum);


	/*
	//
	//	Draws a polygon from the list of points provided and rotates it about the centre point
	//
	//	Points[] - and array of points that make up the polygon
	//	NumPoints - the number of points provided in the Points array
	//	Centre - x,y position of the centre of the polygon 
	//	angle - number of radians to rotate by
	//	brightness - the brightness of the laser
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void DrawPoly(vect2 Points[], int NumPoints, vect2 Centre, float angle, uint8_t brightness, int pointsNum);

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
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void DrawTriangle(int cx, int cy, int w, int h, float a, int rcx, int rcy, uint8_t brightness, int pointsNum);
	
	/*
	//
	//	Draws a text string
	//
	//	x - x coordinate of the top left of the string
	//	y - y coordinate of the top left of the string
	//	size - height off the text
	//	text - the string to draw, null char terminated
	//	brightness - the brightness of the laser
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void DrawString(int x, int y, int size, const unsigned char* text, uint8_t brightness);


	/*
	//
	//	Adds a Raw point to the end of the list of points to be drawn
	//
	//	x - x coordinate of point
	//	y - y coordinate of point
	//	reserved - not used
	//	brightness - intensity of the laser
	//	pointsNum - The number of points used to rener the line, 0 = min
	//
	*/
	void AddRawPoint(uint16_t x, uint16_t y, uint8_t reserved, uint8_t brightness);


	/*
	 *  Sets the max number of frames to send out poer second
	 */
	void SetFrameLimit(int frameLimit);

       /*
	* Enables automaticly adding points to the frame to make all the points equally spaced
	*/
       bool extendFramePoints;


       /*
	* Inserts extra points while the laser is off only
	*/
       bool extendFramePointsBlankOnly;



private:

	// Vector to store a list of the points in the current frame
	std::vector<point> points;


	// Scale to convert points from the specified resolution to the laser display resolution
	//Calculated in constructor from the given resolution
	float scale;

	// Indicates if the laser object has been successfully setup
	bool successSetup;

	//Stores the spi port being used
	int SPI_Port_Num;

	//An SPI object to send data to the laser controller
	SPI_Port SPI;

	//GPIO object to controll the GPIO pins
	mmapGpio rpiGpio;

	//The GPIO pin that will be toggled when a frame has been sent over spi
	int framePin;

	//The current state of the frame pin
	bool framePinState;


	/*
	//
	//	Rotates a point about a centre point
	//
	//	point - the point to be rotated
	//	centre - the location that the point will be rated about
	//	angle - angle in radians to rotate by
	//
	*/
	vect2 RotatePoint(vect2 point, vect2 centre, float angle);

	/*
	 * Gets the time in MS from a unspecified point
	 */
	long getMS();

	/*
	 * Returns that max distance in eith x or y between two points
	 */
	int pointDif(point p1, point p2);

	/*
	 * Time since the last frame was fullt drawn, used to limit frame rate
	*/
	long frameStart;

	/*
	 * The max number of frames per second to output
	 */
	int frameRateLimit;

	/*
	 * The time to wait between frames to output below the max frame rate
	 */
	int frameLimitTime;


};

//#endif
