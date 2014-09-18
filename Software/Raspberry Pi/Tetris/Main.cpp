#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include "Shapes.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

#define ENABLE_LASER_OUTPUT 1
#define LASER_MODE 1
#define DEGUG_OUT 0

//includes needed for outputting to the laser
#if ENABLE_LASER_OUTPUT == 1
#include <stdint.h>
#include "lazey.h"
#include <bcm2835.h>
#endif

using boost::asio::ip::udp;

void GameLogic();
void gameInit();
void UpdateScreen();
void RenderScreen();


bool canPlacePeice(signed int peice, signed int rotation, signed int x, signed int y);
bool placePeice(signed int peice, signed int rotation, signed int x, signed int y);
void drawPeice(signed int peice, signed int rot, signed int xPlace, signed int yPlace);
void clearRows();
void newPeice();
bool checkWin();


void SDLRender();

bool grphicsInit();
void graphicsEnd();

struct peicePos{
	signed short x;
	signed short y;
	signed short peice;
	signed short rotation;
};

enum state { PAUSED, GAME_OVER, PLAYING};

state gameState = PLAYING;
float countdown = 0;
signed long RunDelta = 0;
signed int level = 0;
bool boardDrawn[BOARD_WIDTH][BOARD_HEIGHT];
unsigned int board[BOARD_WIDTH][BOARD_HEIGHT] ;
bool peiceDrawn[4][4] ;
unsigned int peiceNumPlaced = 1;
peicePos movingPeice;
peicePos nextPeice;
signed int points = 0;
bool quit = 0;

const signed int SCREEN_WIDTH = 640;
const signed int SCREEN_HEIGHT = 480;

char keyPressCode = 0;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Init the laser output
#if ENABLE_LASER_OUTPUT == 1
        lazey LazeyProjector((uint16_t)SCREEN_WIDTH,(uint16_t)SCREEN_HEIGHT,
              4,
              BCM2835_SPI_BIT_ORDER_MSBFIRST,
              BCM2835_SPI_MODE0,
              BCM2835_SPI_CLOCK_DIVIDER_8,
              BCM2835_SPI_CS0,
              LOW);
#endif
                                                                                                        


int main(int argc, char* argv[]){

#if ENABLE_LASER_OUTPUT == 1
	printf("Laser output Enaabled\n");
#else
	printf("Laser output Disabled\n");
#endif

#if DEGUG_OUT == 1
	printf("Writing Debug file\n");
#endif

#if LASER_MODE == 1
	printf("Laser graphics ON\n");
#else
	printf("Laser graphics OFF");
	printf("Laser output Disabled");
#endif
	
	LazeyProjector.extendFramePoints = true;

	grphicsInit();

	gameInit();

#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice3, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif

	try
	   {
	       boost::asio::io_service io_service;
	       udp::socket socket(io_service, udp::endpoint(udp::v4(), 1234));


	while(!quit){

		//Recive UDP key presses
		if (socket.available() > 0){
			boost::array<char, 1> recv_buf;
			udp::endpoint remote_endpoint;
			boost::system::error_code error;
	        	socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

			    if (error && error != boost::asio::error::message_size)
				            throw boost::system::system_error(error);
			    keyPressCode = recv_buf[0];
			//printf("Recived: %x\n", recv_buf[0]);
		}



		if ( gameState == PAUSED){


		}else if (gameState == GAME_OVER ){


		}else if (gameState == PLAYING) {

			//Handle UDP key press
			if(keyPressCode != 0){
                        	if (keyPressCode == 2){
					if (canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x-1,movingPeice.y))
						movingPeice.x -=1;
					}
			        else if (keyPressCode == 4){
					if (canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x+1,movingPeice.y))
						movingPeice.x += 1;
					}
				else if (keyPressCode == 1){
					if (movingPeice.rotation >= 3){
						if (canPlacePeice(movingPeice.peice,0,movingPeice.x,movingPeice.y))
							movingPeice.rotation = 0;
					}else {
					        if (canPlacePeice(movingPeice.peice,movingPeice.rotation+1,movingPeice.x,movingPeice.y))
							movingPeice.rotation += 1;
						}
					}
				else if (keyPressCode == 3){
					while(canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x,movingPeice.y+1)){
						movingPeice.y+=1;
					}
				}	
				keyPressCode = 0;
			}

			GameLogic();

		}

		UpdateScreen();

		RenderScreen();

	}

	   }
	catch (std::exception& e)
	    {
	        std::cerr << e.what() << std::endl;
	}

	graphicsEnd();

	return 0;
}


void gameInit(){

	srand((unsigned int)time(NULL));

	for(signed int x = 0; x < BOARD_WIDTH; x++){
		for (signed int y = 0; y < BOARD_HEIGHT; y++){
			board[x][y]=0;
			boardDrawn[x][y]=0;
		}
	}

	movingPeice.peice = 0;
	movingPeice.rotation = 0;
	movingPeice.y = 0;
	movingPeice.x = (signed int)(BOARD_WIDTH/2);

	nextPeice.peice = 0;
	nextPeice.rotation = 0;
	nextPeice.y = 0;
	nextPeice.x = (signed int)(BOARD_WIDTH/2);

#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice1, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif

	points = 0;
	quit = false;
	gameState = PLAYING;
	level = 1;
	countdown = float(0.05 * float(11 - level));
	RunDelta = clock();

	newPeice();newPeice();
#if DEGUG_OUT == 1
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice2, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif

	peiceNumPlaced = 1;
}

void GameLogic(){
	if (gameState == GAME_OVER)
		return;
	if (gameState == PAUSED)
		return;

	float time = ((float)clock() - RunDelta) / CLOCKS_PER_SEC;
#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif


	if ( time >= countdown){
		RunDelta = clock();

		//Check if the moving piece can be moved down
		if (canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x,movingPeice.y+1)){
			movingPeice.y += 1;
		} else{
			if(placePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x,movingPeice.y) == false)
				gameState = GAME_OVER; // Cannot place piece so game over

			clearRows();

			newPeice();

			if  (checkWin())
				gameState = GAME_OVER;

		}

	}
}


void UpdateScreen(){
	//Render components in momory for SDL and laser
	SDLRender();
}

void RenderScreen(){
	//Update SDL screen
	SDL_RenderPresent( gRenderer );
#if ENABLE_LASER_OUTPUT == 1
	//Render the Laser screen
	LazeyProjector.renderFrame();
#endif

}

bool canPlacePeice(signed int peice, signed int rotation, signed int x, signed int y){
	//if (x < 2)
	//	return false;
	//if (x > BOARD_WIDTH - 1)
	//	return false;
	//if (y < 1)
	//	return false;
	//if (y > BOARD_HEIGHT - 3)
	//	return false;
	signed int xPos, xLoc, yPos, yLoc;
	for (xPos = x - 2, xLoc = 0; xPos <= x + 1; xPos++, xLoc++){
		for (yPos = y -1,yLoc = 0; yPos <= y + 2; yPos++, yLoc++){
			if (xPos >= 0 && xPos < BOARD_WIDTH && yPos < BOARD_HEIGHT){
				if (pieces[peice][rotation][xLoc][yLoc] == 1 && board[xPos][yPos] != 0){
					return false;
				}
			}else{
				if (pieces[peice][rotation][xLoc][yLoc] == 1)
					return false;
			}
		}
	}
	return true;
}

bool placePeice(signed int peice, signed int rotation, signed int x, signed int y){
	//if (x < 2)
	//	return false;
	//if (x > BOARD_WIDTH - 1)
	//	return false;
	//if (y < 1)
	//	return false;
	//if (y > BOARD_HEIGHT - 3)
	//	return false;

	signed int xPos, xLoc, yPos, yLoc;
#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	for (xPos = x - 2, xLoc = 0; xPos <= x + 1; xPos++, xLoc++){
		for (yPos = y -1, yLoc = 0; yPos <= y + 2; yPos++, yLoc++){
			fprintf (pFile, "{%d, ", pieces[peice][rotation][xLoc][yLoc]);
			if (xPos >= 0 && xPos < BOARD_WIDTH && yPos < BOARD_HEIGHT){
				fprintf (pFile, "1, ");
				if (pieces[peice][rotation][xLoc][yLoc] == 1){
					fprintf (pFile, "1}, ");
				}else{
					fprintf (pFile, "0}, ");
				}
			}else{
				fprintf (pFile, "0, ");
				if (pieces[peice][rotation][xLoc][yLoc] == 1){
					fprintf (pFile, "1}, ");
				}else{
					fprintf (pFile, "0}, ");
				}
			}
		}
		fprintf (pFile, "\n");
	}
		fprintf (pFile, "\n");	fprintf (pFile, "\n");
#endif

	for (xPos = x - 2, xLoc = 0; xPos <= x + 1; xPos++, xLoc++){
		for (yPos = y -1, yLoc = 0; yPos <= y + 2; yPos++, yLoc++){
			if (xPos >= 0 && xPos < BOARD_WIDTH && yPos < BOARD_HEIGHT){
				if (pieces[peice][rotation][xLoc][yLoc] == 1){
					board[xPos][yPos] = peiceNumPlaced;
#if DEGUG_OUT == 1
fprintf (pFile, "\nx:%d, y:%d, num:%d\n", xPos,yPos, peiceNumPlaced);
#endif
				}
			}else{
				if (pieces[peice][rotation][xLoc][yLoc] == 1){
#if DEGUG_OUT == 1
fprintf (pFile, "\nReturning FALSE\n");
fclose (pFile);
#endif
					return false;
				}
			}
		}
	}
#if DEGUG_OUT == 1
//	FILE * pFile;
//	pFile = fopen ("myfile.txt","a");

	for( signed int y = 0; y < BOARD_HEIGHT; y++){
		for (signed int x = 0; x < BOARD_WIDTH; x++){
			fprintf (pFile, "%d,",board[x][y]);
		}
		fprintf (pFile, "\n");
	}
	fprintf (pFile, "\n");fprintf (pFile, "\n");fprintf (pFile, "\n");fprintf (pFile, "\n");fprintf (pFile, "\n");
	fclose (pFile);
#endif

	peiceNumPlaced++;
	return true;
}

void clearRows(){
	signed int count = 0;
	for( signed int y = BOARD_HEIGHT -1; y >= 0; y--){
		bool complete = true;
		for (signed int x = 0; x < BOARD_WIDTH; x++){
			if (board[x][y] == 0){
				complete = false;
				break;
			}
		}

		if (complete){
			count++;
			points += 10;
			for( signed int yy = y -1; yy >=0; yy--){
				for (signed int x = 0; x < BOARD_WIDTH; x++){
					board[x][yy+1] = board[x][yy];
				}
			}
			y +=1;
		}else if (count >= 1){
			count++;
		}
		if (count >=4)
			break;
	}

}

void newPeice(){
	movingPeice.peice = nextPeice.peice;
	nextPeice.peice = rand() % 7;
	movingPeice.rotation  = 0;
	movingPeice.x = BOARD_WIDTH/2;
	movingPeice.y = 0;
}

bool checkWin(){
	for (signed int x = 0; x < BOARD_WIDTH; x++){
		if (board[x][0] != 0)
			return true;
	}
	return false;

}


void SDLRender(){

#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice5, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif
	SDL_Event e;
	while (SDL_PollEvent(&e)){

		switch (e.type){
		case SDL_QUIT:
			quit = true;
			break;

		case SDL_KEYDOWN:
			if (e.key.keysym.sym == SDLK_LEFT){
				if (canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x-1,movingPeice.y))
					movingPeice.x -=1;
#if DEGUG_OUT == 1
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nLEFT BUTTON\n");
fclose (pFile);
#endif
			}
			else if (e.key.keysym.sym == SDLK_RIGHT){
				if (canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x+1,movingPeice.y))
					movingPeice.x += 1;
			}
			else if (e.key.keysym.sym == SDLK_UP){
				if (movingPeice.rotation >= 3){
					if (canPlacePeice(movingPeice.peice,0,movingPeice.x,movingPeice.y))
						movingPeice.rotation = 0;
				}else {
					if (canPlacePeice(movingPeice.peice,movingPeice.rotation+1,movingPeice.x,movingPeice.y))
						movingPeice.rotation += 1;
				}
			}
			else if (e.key.keysym.sym == SDLK_DOWN){
				while(canPlacePeice(movingPeice.peice,movingPeice.rotation,movingPeice.x,movingPeice.y+1)){
					movingPeice.y+=1;
				}
			} else if (e.key.keysym.sym == SDLK_p){
				if (gameState == PAUSED){
					gameState = PLAYING;
				}else{
					gameState = PAUSED;
				}
			} else if (e.key.keysym.sym == SDLK_ESCAPE){
				quit = true;
			}

		}
#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice6, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif
		////if user closes the window
		//if (e.type == SDL_QUIT)
		//	quit = true;
		////if user presses any key
		//if (e.type == SDL_KEYDOWN)
		//	quit = true;
		////if user clicks the mouse
		//if (e.type == SDL_MOUSEBUTTONDOWN)
		//	quit = true;

	}

	//Clear screen
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0, 0xFF );
	SDL_RenderClear( gRenderer );

	signed int size = SCREEN_HEIGHT / BOARD_HEIGHT;
	signed int offset = (SCREEN_WIDTH/2) - (size * BOARD_WIDTH/2);


#if LASER_MODE
	signed int gap = 2;

	SDL_Rect fillRect = { offset, 0, size * BOARD_WIDTH, SCREEN_HEIGHT };
	SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );		
	SDL_RenderFillRect( gRenderer, &fillRect );

#if ENABLE_LASER_OUTPUT == 1
	LazeyProjector.DrawRect(offset, 0, size * BOARD_WIDTH, SCREEN_HEIGHT, 255, 20);
#endif

	for(signed int y = BOARD_HEIGHT -1; y >= 0; y--){
		for(signed int x = 0; x < BOARD_WIDTH; x++){
			
			if (boardDrawn[x][y] == 0 && board[x][y] != 0){
				boardDrawn[x][y] = 1;
				signed int xPos1 = x;
				signed int yPos1 = y;
				signed int xPos2 = x;
				signed int yPos2 = y;
				signed int xStart = x;
				signed int yStart = y;

				unsigned int peiceNum = board[x][y];
				signed int dir = 1;

				if (board[xPos2][yPos2-1] == peiceNum){
					dir = 1;
				}else if (board[xPos2][yPos2+1] == peiceNum){
					dir = 4;
				}else if (board[xPos2-1][yPos2] == peiceNum){
					dir = 2;
				}else if (board[xPos2+1][yPos2] == peiceNum){
					dir = 3;
				}else{
					dir = 0;

					SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, (yPos1 * size) + size - gap, (xPos2 * size) + offset + gap, (yPos2 * size) + gap );
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + size + offset - gap, (yPos1 * size) + size - gap, (xPos2 * size) + offset + size - gap, (yPos2 * size) + gap );
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, (yPos1 * size) + size - gap, (xPos2 * size) + size + offset - gap, (yPos2 * size) + size - gap );
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, (yPos1 * size) + gap, (xPos2 * size) + size + offset - gap, (yPos2 * size) + gap );

#if ENABLE_LASER_OUTPUT == 1
					LazeyProjector.DrawLine( (xPos1 * size) + offset + gap, (yPos1 * size) + size - gap, (xPos2 * size) + offset + gap, (yPos2 * size) + gap , 255, 0);
					LazeyProjector.DrawLine( (xPos1 * size) + size + offset - gap, (yPos1 * size) + size - gap, (xPos2 * size) + offset + size - gap, (yPos2 * size) + gap  , 255, 0);
					LazeyProjector.DrawLine( (xPos1 * size) + offset + gap, (yPos1 * size) + size - gap, (xPos2 * size) + size + offset - gap, (yPos2 * size) + size - gap  , 255, 0);
					LazeyProjector.DrawLine( (xPos1 * size) + offset + gap, (yPos1 * size) + gap, (xPos2 * size) + size + offset - gap, (yPos2 * size) + gap  , 255, 0);
#endif

				}
				signed int startDir = dir;
				do{
					if (dir == 1){ // UP
						do{
							yPos2--;
							boardDrawn[xPos2][yPos2] = 1;
						}while(board[xPos2][yPos2-1] == peiceNum && board[xPos2-1][yPos2] != peiceNum);

						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );
						SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, ((board[xPos1-1][yPos1] == peiceNum ? yPos1-1 : yPos1) * size) + size - (board[xPos1-1][yPos1] == peiceNum ? -gap : gap), (xPos2 * size) + offset + gap, ((board[xPos2-1][yPos2] == peiceNum ? yPos2+1 : yPos2 )* size) + (board[xPos2-1][yPos2] == peiceNum ? -gap : gap));

#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine((xPos1 * size) + offset + gap, ((board[xPos1-1][yPos1] == peiceNum ? yPos1-1 : yPos1) * size) + size - (board[xPos1-1][yPos1] == peiceNum ? -gap : gap), (xPos2 * size) + offset + gap, ((board[xPos2-1][yPos2] == peiceNum ? yPos2+1 : yPos2 )* size) + (board[xPos2-1][yPos2] == peiceNum ? -gap : gap) , 255, 0);
#endif

						yPos1 = yPos2;
						if (board[xPos2-1][yPos2] == peiceNum){ //Left
							dir = 2;
						}else if (board[xPos2+1][yPos2] == peiceNum){ // Right
							dir = 3;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, (yPos1 * size) + gap, ((xPos2 + 1) * size) + offset - gap, (yPos2 * size) + gap );
#if ENABLE_LASER_OUTPUT == 1
							LazeyProjector.DrawLine( (xPos1 * size) + offset + gap, (yPos1 * size) + gap, ((xPos2 + 1) * size) + offset - gap, (yPos2 * size) + gap , 255,0);
#endif
							dir = 4;
						}
					}else if (dir == 2){ //Left
						do{
							xPos2--;
							boardDrawn[xPos2][yPos2] = 1;
						}while(board[xPos2-1][yPos2] == peiceNum && board[xPos2][yPos2+1] != peiceNum);
						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, ((board[xPos1][yPos1+1] == peiceNum ? xPos1-1 : xPos1) * size) + size + offset - (board[xPos1][yPos1+1] == peiceNum ? -gap : gap), (yPos1 * size) + size - gap, ((board[xPos2][yPos2+1] == peiceNum ? xPos2+1 : xPos2) * size) + offset + (board[xPos2][yPos2+1] == peiceNum ? -gap : gap), (yPos2 * size) + size - gap );
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine(  ((board[xPos1][yPos1+1] == peiceNum ? xPos1-1 : xPos1) * size) + size + offset - (board[xPos1][yPos1+1] == peiceNum ? -gap : gap), (yPos1 * size) + size - gap, ((board[xPos2][yPos2+1] == peiceNum ? xPos2+1 : xPos2) * size) + offset + (board[xPos2][yPos2+1] == peiceNum ? -gap : gap), (yPos2 * size) + size - gap , 25,0);
#endif

						xPos1 = xPos2;
						if (board[xPos2][yPos2+1] == peiceNum){ //Down
							dir = 4;
						}else if (board[xPos2][yPos2-1] == peiceNum){ // UP
							dir = 1;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, (yPos1 * size) + gap, (xPos2 * size) + offset + gap, (yPos2 * size) - gap + size );
#if ENABLE_LASER_OUTPUT == 1
							LazeyProjector.DrawLine( (xPos1 * size) + offset + gap, (yPos1 * size) + gap, (xPos2 * size) + offset + gap, (yPos2 * size) - gap + size  , 255,0);
#endif

							dir = 3;
						}
					}else if (dir == 3){ // Right
						do{
							xPos2++;
							boardDrawn[xPos2][yPos2] = 1;
						}while(board[xPos2+1][yPos2] == peiceNum && board[xPos2][yPos2-1] != peiceNum);
						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, ((board[xPos1][yPos1-1] == peiceNum ? xPos1+1 : xPos1) * size) + offset + (board[xPos1][yPos1-1] == peiceNum ? -gap : gap), (yPos1 * size) + gap, ((board[xPos2][yPos2-1] == peiceNum ? xPos2-1 : xPos2) * size) +size + offset - (board[xPos2][yPos2-1] == peiceNum ? -gap : gap), (yPos2 * size) + gap );
#if ENABLE_LASER_OUTPUT == 1
							LazeyProjector.DrawLine(((board[xPos1][yPos1-1] == peiceNum ? xPos1+1 : xPos1) * size) + offset + (board[xPos1][yPos1-1] == peiceNum ? -gap : gap), (yPos1 * size) + gap, ((board[xPos2][yPos2-1] == peiceNum ? xPos2-1 : xPos2) * size) +size + offset - (board[xPos2][yPos2-1] == peiceNum ? -gap : gap), (yPos2 * size) + gap  , 255,0);
#endif

						xPos1 = xPos2;
						if (board[xPos2][yPos2-1] == peiceNum){ //UP
							dir = 1;
						}else if (board[xPos2][yPos2+1] == peiceNum){ // Down
							dir = 4;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + size - gap, (yPos1 * size) + gap, (xPos2 * size) + offset + size - gap, (yPos2 * size) - gap + size );
#if ENABLE_LASER_OUTPUT == 1
							LazeyProjector.DrawLine( (xPos1 * size) + offset + size - gap, (yPos1 * size) + gap, (xPos2 * size) + offset + size - gap, (yPos2 * size) - gap + size  , 255,0);
#endif

							dir = 2;
						}
					}else if (dir == 4){ // Down
						do{
							yPos2++;
							boardDrawn[xPos2][yPos2] = 1;
						}while(board[xPos2][yPos2+1] == peiceNum && board[xPos2+1][yPos2] != peiceNum);
						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset - gap + size, ((board[xPos1+1][yPos1] == peiceNum ? yPos1+1 : yPos1) * size) + (board[xPos1+1][yPos1] == peiceNum ? -gap : gap), (xPos2 * size) + offset - gap + size, ((board[xPos2+1][yPos2] == peiceNum ? yPos2-1 : yPos2 )* size) + size - (board[xPos2+1][yPos2] == peiceNum ? -gap : gap) );
#if ENABLE_LASER_OUTPUT == 1
							LazeyProjector.DrawLine(  (xPos1 * size) + offset - gap + size, ((board[xPos1+1][yPos1] == peiceNum ? yPos1+1 : yPos1) * size) + (board[xPos1+1][yPos1] == peiceNum ? -gap : gap), (xPos2 * size) + offset - gap + size, ((board[xPos2+1][yPos2] == peiceNum ? yPos2-1 : yPos2 )* size) + size - (board[xPos2+1][yPos2] == peiceNum ? -gap : gap) , 255,0);
#endif

						yPos1 = yPos2;
						if (board[xPos2+1][yPos2] == peiceNum){ //Right
							dir = 3;
						}else if (board[xPos2-1][yPos2] == peiceNum){ // Left
							dir = 2;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap, (yPos1 * size) + size - gap, ((xPos2 + 1) * size) + offset - gap, (yPos2 * size) + size - gap );
#if ENABLE_LASER_OUTPUT == 1
							LazeyProjector.DrawLine( (xPos1 * size) + offset + gap, (yPos1 * size) + size - gap, ((xPos2 + 1) * size) + offset - gap, (yPos2 * size) + size - gap , 255,0);
#endif

							dir = 1;
						}
					}else{
						break;
					}
				}while(!(dir == startDir && xPos1 == xStart && yPos1 == yStart));

			}

		}
	}

	for(signed int x = 0; x < BOARD_WIDTH; x++){
		for (signed int y = 0; y < BOARD_HEIGHT; y++){
			boardDrawn[x][y]=0;
		}
	}


#else

	for(signed int y = BOARD_HEIGHT -1; y >= 0; y--){
		for(signed int x = 0; x < BOARD_WIDTH; x++){
			if (board[x][y] != 0){
				SDL_Rect fillRect = { x*size + offset, y*size, size, size };
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );		
				SDL_RenderFillRect( gRenderer, &fillRect );
			}else{
				SDL_Rect fillRect = { x*size + offset, y*size, size, size };
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0xFF, 0x00 );		
				SDL_RenderFillRect( gRenderer, &fillRect );
			}

		}
	}

#endif

#if DEGUG_OUT == 1
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice7, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif

	drawPeice(movingPeice.peice, movingPeice.rotation, movingPeice.x, movingPeice.y);
#if DEGUG_OUT == 1
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice8, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif
	drawPeice(nextPeice.peice,nextPeice.rotation, -4, 2);
#if DEGUG_OUT == 1
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice9, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
fclose (pFile);
#endif




				////Render red filled quad
				//SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
				//SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );		
				//SDL_RenderFillRect( gRenderer, &fillRect );

				////Render green outlined quad
				//SDL_Rect outlineRect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
				//SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );		
				//SDL_RenderDrawRect( gRenderer, &outlineRect );
				//
				////Draw blue horizontal line
				//SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );		
				//SDL_RenderDrawLine( gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2 );

				////Draw vertical line of yellow dots
				//SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x00, 0xFF );
				//for( int i = 0; i < SCREEN_HEIGHT; i += 4 )
				//{
				//	SDL_RenderDrawPoint( gRenderer, SCREEN_WIDTH / 2, i );
				//}

}

bool grphicsInit(){
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_SOFTWARE );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

			}
		}
	}

	return success;
}

void graphicsEnd(){
		//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void drawPeice(signed int peice, signed int rot, signed int xPlace, signed int yPlace){

	signed int size = SCREEN_HEIGHT / BOARD_HEIGHT;
	signed int offset = (SCREEN_WIDTH/2) - (size * BOARD_WIDTH/2);
#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice10, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
	fclose (pFile);
#endif


#if LASER_MODE
	signed int gap = 2;
	signed int xOffset = (xPlace * size) - (2*size);
	signed int yOffset = (yPlace * size) - (size);

	for (signed int y = 3; y >= 0; y--){
		for( signed int x = 0; x < 4; x++){
			peiceDrawn[x][y] = 0;
		}
	} for (signed int y = 3; y >= 0; y--){ for( signed int x = 0; x < 4; x++){ 
#if DEGUG_OUT == 1
	FILE * pFile;
	pFile = fopen ("myfile.txt","a");
	fprintf (pFile, "\nMoving Peice12, x:%d, y:%d\n", movingPeice.x, movingPeice.y);
	fclose (pFile);
#endif
			if (peiceDrawn[x][y] == 0 && pieces[peice][rot][x][y] != 0){
				peiceDrawn[x][y] = 1;
				signed int xPos1 = x;
				signed int yPos1 = y;
				signed int xPos2 = x;
				signed int yPos2 = y;
				signed int xStart = x;
				signed int yStart = y;

				unsigned int peiceNum = pieces[peice][rot][x][y];
				signed int dir = 1;

				if (pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true)){
					dir = 1;
				}else if (pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true)){
					dir = 4;
				}else if (pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true)){
					dir = 2;
				}else if (pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true)){
					dir = 3;
				}else{
					dir = 0;

					SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + size - gap + yOffset, (xPos2 * size) + offset + gap + xOffset, (yPos2 * size) + gap + yOffset );
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + size + offset - gap + xOffset, (yPos1 * size) + size - gap + yOffset, (xPos2 * size) + offset + size - gap + xOffset, (yPos2 * size) + gap + yOffset );
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + size - gap + yOffset, (xPos2 * size) + size + offset - gap + xOffset, (yPos2 * size) + size - gap + yOffset);
					SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + gap + yOffset, (xPos2 * size) + size + offset - gap + xOffset, (yPos2 * size) + gap + yOffset);
#if ENABLE_LASER_OUTPUT == 1
					LazeyProjector.DrawLine( (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + size - gap + yOffset, (xPos2 * size) + offset + gap + xOffset, (yPos2 * size) + gap + yOffset , 255, 0);
					LazeyProjector.DrawLine( (xPos1 * size) + size + offset - gap + xOffset, (yPos1 * size) + size - gap + yOffset, (xPos2 * size) + offset + size - gap + xOffset, (yPos2 * size) + gap + yOffset, 255, 0 );
					LazeyProjector.DrawLine( (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + size - gap + yOffset, (xPos2 * size) + size + offset - gap + xOffset, (yPos2 * size) + size - gap + yOffset, 255, 0);
					LazeyProjector.DrawLine( (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + gap + yOffset, (xPos2 * size) + size + offset - gap + xOffset, (yPos2 * size) + gap + yOffset, 255, 0);
#endif

				}
				signed int startDir = dir;
				do{
					if (dir == 1){ // UP
						do{
							yPos2--;
							peiceDrawn[xPos2][yPos2] = 1;
						}while(pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true) && pieces[peice][rot][xPos2-1][yPos2] != (peiceNum == 0 ? false : true));

						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, ((pieces[peice][rot][xPos1-1][yPos1] == (peiceNum == 0 ? false : true) ? yPos1-1 : yPos1) * size) + yOffset +  size - (pieces[peice][rot][xPos1-1][yPos1] == (peiceNum == 0 ? false : true) ? -gap : gap), (xPos2 * size) + offset + gap + xOffset, ((pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true) ? yPos2+1 : yPos2 )* size) + (pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true) ? -gap : gap) + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine( (xPos1 * size) + offset + gap + xOffset, ((pieces[peice][rot][xPos1-1][yPos1] == (peiceNum == 0 ? false : true) ? yPos1-1 : yPos1) * size) + yOffset +  size - (pieces[peice][rot][xPos1-1][yPos1] == (peiceNum == 0 ? false : true) ? -gap : gap), (xPos2 * size) + offset + gap + xOffset, ((pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true) ? yPos2+1 : yPos2 )* size) + (pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true) ? -gap : gap) + yOffset, 255, 0);
#endif
						yPos1 = yPos2;
						if (pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true)){ //Left
							dir = 2;
						}else if (pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true)){ // Right
							dir = 3;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + gap + yOffset, ((xPos2 + 1) * size) + offset - gap + xOffset, (yPos2 * size) + gap + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine( (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + gap + yOffset, ((xPos2 + 1) * size) + offset - gap + xOffset, (yPos2 * size) + gap + yOffset,255, 0);
#endif
							dir = 4;
						}
					}else if (dir == 2){ //Left
						do{
							xPos2--;
							peiceDrawn[xPos2][yPos2] = 1;
						}while(pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true) && pieces[peice][rot][xPos2][yPos2+1] != (peiceNum == 0 ? false : true));
						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, ((pieces[peice][rot][xPos1][yPos1+1] == (peiceNum == 0 ? false : true) ? xPos1-1 : xPos1) * size) + size + xOffset + offset - (pieces[peice][rot][xPos1][yPos1+1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos1 * size) + size - gap + yOffset, ((pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true) ? xPos2+1 : xPos2) * size) + offset + xOffset + (pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos2 * size) + size - gap + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine(((pieces[peice][rot][xPos1][yPos1+1] == (peiceNum == 0 ? false : true) ? xPos1-1 : xPos1) * size) + size + xOffset + offset - (pieces[peice][rot][xPos1][yPos1+1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos1 * size) + size - gap + yOffset, ((pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true) ? xPos2+1 : xPos2) * size) + offset + xOffset + (pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos2 * size) + size - gap + yOffset ,255, 0);
#endif

						xPos1 = xPos2;
						if (pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true)){ //Down
							dir = 4;
						}else if (pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true)){ // UP
							dir = 1;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + gap + yOffset, (xPos2 * size) + offset + gap + xOffset, (yPos2 * size) - gap + size + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine( (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + gap + yOffset, (xPos2 * size) + offset + gap + xOffset, (yPos2 * size) - gap + size + yOffset,255, 0);
#endif

							dir = 3;
						}
					}else if (dir == 3){ // Right
						do{
							xPos2++;
							peiceDrawn[xPos2][yPos2] = 1;
						}while(pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true) && pieces[peice][rot][xPos2][yPos2-1] != (peiceNum == 0 ? false : true));
						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, ((pieces[peice][rot][xPos1][yPos1-1] == (peiceNum == 0 ? false : true) ? xPos1+1 : xPos1) * size) + offset + xOffset + (pieces[peice][rot][xPos1][yPos1-1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos1 * size) + gap + yOffset, ((pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true) ? xPos2-1 : xPos2) * size) +size + xOffset + offset - (pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos2 * size) + gap + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine(((pieces[peice][rot][xPos1][yPos1-1] == (peiceNum == 0 ? false : true) ? xPos1+1 : xPos1) * size) + offset + xOffset + (pieces[peice][rot][xPos1][yPos1-1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos1 * size) + gap + yOffset, ((pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true) ? xPos2-1 : xPos2) * size) +size + xOffset + offset - (pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true) ? -gap : gap), (yPos2 * size) + gap + yOffset,255, 0);
#endif

						xPos1 = xPos2;
						if (pieces[peice][rot][xPos2][yPos2-1] == (peiceNum == 0 ? false : true)){ //UP
							dir = 1;
						}else if (pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true)){ // Down
							dir = 4;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + xOffset + size - gap, (yPos1 * size) + gap + yOffset, (xPos2 * size) + offset + xOffset + size - gap, (yPos2 * size) - gap + size + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine(  (xPos1 * size) + offset + xOffset + size - gap, (yPos1 * size) + gap + yOffset, (xPos2 * size) + offset + xOffset + size - gap, (yPos2 * size) - gap + size + yOffset,255, 0);
#endif

							dir = 2;
						}
					}else if (dir == 4){ // Down
						do{
							yPos2++;
							peiceDrawn[xPos2][yPos2] = 1;
						}while(pieces[peice][rot][xPos2][yPos2+1] == (peiceNum == 0 ? false : true) && pieces[peice][rot][xPos2+1][yPos2] != (peiceNum == 0 ? false : true));
						SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
						SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset - gap + size + xOffset, ((pieces[peice][rot][xPos1+1][yPos1] == (peiceNum == 0 ? false : true) ? yPos1+1 : yPos1) * size) + yOffset + (pieces[peice][rot][xPos1+1][yPos1] == (peiceNum == 0 ? false : true) ? -gap : gap), (xPos2 * size) + offset - gap + size + xOffset, ((pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true) ? yPos2-1 : yPos2 )* size) + size - (pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true) ? -gap : gap) + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine( (xPos1 * size) + offset - gap + size + xOffset, ((pieces[peice][rot][xPos1+1][yPos1] == (peiceNum == 0 ? false : true) ? yPos1+1 : yPos1) * size) + yOffset + (pieces[peice][rot][xPos1+1][yPos1] == (peiceNum == 0 ? false : true) ? -gap : gap), (xPos2 * size) + offset - gap + size + xOffset, ((pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true) ? yPos2-1 : yPos2 )* size) + size - (pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true) ? -gap : gap) + yOffset ,255, 0);
#endif

						yPos1 = yPos2;
						if (pieces[peice][rot][xPos2+1][yPos2] == (peiceNum == 0 ? false : true)){ //Right
							dir = 3;
						}else if (pieces[peice][rot][xPos2-1][yPos2] == (peiceNum == 0 ? false : true)){ // Left
							dir = 2;
						}else { //back
							SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0x00 );		
							SDL_RenderDrawLine( gRenderer, (xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + size - gap + yOffset, ((xPos2 + 1) * size) + offset - gap + xOffset, (yPos2 * size) + size - gap + yOffset);
#if ENABLE_LASER_OUTPUT == 1
						LazeyProjector.DrawLine((xPos1 * size) + offset + gap + xOffset, (yPos1 * size) + size - gap + yOffset, ((xPos2 + 1) * size) + offset - gap + xOffset, (yPos2 * size) + size - gap + yOffset ,255, 0);
#endif

							dir = 1;
						}
					}else{
						break;
					}
				}while(!(dir == startDir && xPos1 == xStart && yPos1 == yStart));
			}
		}
	}

#else
	signed int xPos, xLoc, yPos, yLoc;
	for (xPos = xPlace - 2, xLoc = 0; xPos <= xPlace + 1; xPos++, xLoc++){
		for (yPos = yPlace -1, yLoc = 0; yPos <= yPlace + 2; yPos++, yLoc++){
			if (pieces[peice][rot][xLoc][yLoc]  == 1){
				SDL_Rect fillRect = { xPos*size + offset, yPos*size, size, size };
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );		
				SDL_RenderFillRect( gRenderer, &fillRect );
			}
		}
	}

#endif

}
