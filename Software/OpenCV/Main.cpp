#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <time.h>
#include <math.h>
#include <algorithm>


int VIDEO = 1;
int BUILTIN_CONTOURS = 0;
int DYNAMIC_QUALITY = 1;
int DYNAMIC_QUALITY_POINTS = 3000;
int ENABLE_NETWORK = 0;
int GALERY_MODE = 0;

//#define VIDEO 0
//#define BUILTIN_CONTOURS 0
//#define DYNAMIC_QUALITY 0
//#define DYNAMIC_QUALITY_POINTS 3000
//#define ENABLE_NETWORK 1


struct point {
	uint16_t x;
	uint16_t y;
	uint8_t brightness;
	uint8_t resereved;
};

struct distP {
	int dist;
	bool used;
};

using boost::asio::ip::tcp;
using namespace cv;
using namespace std;


Mat img, src_gray;
Mat dst, detected_edges;


int edgeThresh = 1;
int lowThreshold = 100;
int const max_lowThreshold = 150;
int ratio = 3;
int kernel_size = 3;
char* window_Orig = "Original";
char* window_Edge = "Edge Map";
char* window_Con = "Contours";

static int mouse_x = -1;
static int mouse_y = -1;

int imgWidth = 0;
int imgHeight = 0;

int x = -1;
int y = -1;

bool mouseDown = false;
bool RmouseDown = false;

bool enableEdgeWindows = false;

time_t start,endTime;
double startTicks = 0;
int fpsRate = 0;

int ** pixelLoopUp = NULL;

int quaity = 2;
int mode = 0;

clock_t galleryTimer = 0;
int galeryWaitTime = 10;
int GaleryIndex = 1;
int GaleryIndexMax = 20;

#define SAVE_FILE 0
bool savedFile = false;


vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
int totalPoints = 0;

int findContoursLinear(Mat &image, vector<vector<Point> > &contours){

	uchar* pixelPtr = image.data;
	int cn = image.channels();
	uchar bgrPixel;
	int count = 0;

	bool ** usedPixels = (bool**) calloc(image.cols,sizeof(bool*));

	for(int i = 0; i < image.cols; i++){
		usedPixels[i] = (bool*) calloc(image.rows,sizeof(bool));
	}

	contours.clear();

	// Loop though all the pixels
	for(int y = 0; y < image.rows; y++)
	{
		for(int x = 0; x < image.cols; x++)
		{
			bgrPixel = pixelPtr[pixelLoopUp[x][y]]; // G

			//Check if the pixel is white and if it has already been used
			if((bgrPixel == 1) && (usedPixels[x][y] == false)){
				//Set the pixel to used
				usedPixels[x][y] = true;
				//Increase the line count
				count++;

				vector<Point> line;
				Point pos = Point(x,y);
				Point StartPos = pos;
				line.push_back(StartPos);
				bool startPointAgain = false;
				//Follow the line
				do{

					if((pos.y > 0) && (usedPixels[pos.x][pos.y-1] == 0) && (pixelPtr[pixelLoopUp[pos.x][pos.y-1]]) ){
						//UP
						pos = Point(pos.x, pos.y-1);

					}else if((pos.y < image.rows - 1) && (usedPixels[pos.x][pos.y+1] == 0) && (pixelPtr[pixelLoopUp[pos.x][pos.y+1]]) ){
						//Down
						pos = Point(pos.x, pos.y+1);

					}else if((pos.x > 0 ) && (usedPixels[pos.x-1][pos.y] == 0) && (pixelPtr[pixelLoopUp[pos.x-1][pos.y]]) ){
						//Left
						pos = Point(pos.x-1, pos.y);

					}else if((pos.x < image.cols - 1) && (usedPixels[pos.x+1][pos.y] == 0) && (pixelPtr[pixelLoopUp[pos.x+1][pos.y]]) ){
						//Right
						pos = Point(pos.x+1, pos.y);

					}else if((pos.x > 0) && (pos.y > 0) && (usedPixels[pos.x-1][pos.y-1] == 0) && (pixelPtr[pixelLoopUp[pos.x-1][pos.y-1]]) ){
						// Up - Left
						pos = Point(pos.x-1, pos.y-1);

					}else if((pos.x < image.cols - 1) && (pos.y > 0) && (usedPixels[pos.x+1][pos.y-1] == 0) && (pixelPtr[pixelLoopUp[pos.x+1][pos.y-1]]) ){
						//Up - Right
						pos = Point(pos.x+1, pos.y-1);

					}else if((pos.x > 0) && (pos.y < image.rows -1) && (usedPixels[pos.x-1][pos.y+1] == 0) && (pixelPtr[pixelLoopUp[pos.x-1][pos.y+1]]) ){
						//Down - Left
						pos = Point(pos.x-1, pos.y+1);

					}else if((pos.x < image.cols - 1) && (pos.y < image.rows - 1) && (usedPixels[pos.x+1][pos.y+1] == 0) && (pixelPtr[pixelLoopUp[pos.x+1][pos.y+1]]) ){
						//Down - Right
						pos = Point(pos.x+1, pos.y+1);

					}else{
						//Dead end
						pos = StartPos;
						if (startPointAgain){
							//line.push_back(pos);
							break;
						}
						startPointAgain = true;
					}
					if (startPointAgain){
						line.insert(line.begin(), pos);
					}else{
						line.push_back(pos);
					}
					usedPixels[pos.x][pos.y] = true;

				}while(true);

				contours.push_back(line);
			}
			// do something with BGR values...
		}
	}

	contours.pop_back();

	for(int i = 0; i < image.rows; i++){
		free(usedPixels[i]);
	}

	free(usedPixels);

	return 0;
}

void CannyThreshold(int, void*)
{
	/// Reduce noise with a kernel 3x3
	blur( src_gray, detected_edges, Size(3,3) );

	/// Canny detector
	Canny( src_gray, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);

	threshold(detected_edges,detected_edges,0,1,CV_THRESH_BINARY);
	//imwrite( "Gray_Image.png", detected_edges );

	img.copyTo( dst, detected_edges);
	if(enableEdgeWindows){
		imshow( window_Edge, dst );
	}

	contours.clear();



	/// Find contours
	if (BUILTIN_CONTOURS == 1){
		findContours( detected_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE , Point(0, 0) );
	}else{
		findContoursLinear(detected_edges,contours);
	}



	
	/// Draw contours
	//Mat drawing = Mat::zeros( detected_edges.size(), detected_edges.type() );
	Mat drawing = Mat::zeros( detected_edges.size(), CV_8UC3 );
	//for( int i = 0; i< contours.size(); i++ )
	//{
	//	Scalar color = Scalar( 0, 255, 0 );
	//	drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
	//}


	if (contours.size() != 0){

//The built in contours function requires some optimisations on the output
	if (BUILTIN_CONTOURS == 1){
		//Remove inner contour loop
		for( int i = 0; i< contours.size()-1; i++ ){
			if ((contours[i][0].x - contours[i+1][0].x < 2) && (contours[i][0].x - contours[i+1][0].x > -2) && (contours[i][0].y - contours[i+1][0].y < 2) && (contours[i][0].y - contours[i+1][0].y > -2)){
				contours.erase(contours.begin()+i);
				i--;

			}
		}


		//Check for non looping contours
		int p1 = 0;
		int p2 = 0;
		for( int i = 0; i< contours.size(); i++ ){
			p1 = 0;
			p2 = 0;
			for ( int p = 1; p < contours[i].size()-1; p++){
				if (contours[i][p-1] == contours[i][p+1]){
				//if ( (contours[i][p].x == 0) || (contours[i][p].x == imgWidth -2) || (contours[i][p].y == 0) || (contours[i][p].y == imgHeight-2)){
					//int diffX = 0;
					//int diffY = 0;
					//for(int y = 1; y < 10; y++){
					//	diffX = diffX + abs(contours[i][p-y].x - contours[i][p+y].x);
					//	diffY = diffY + abs(contours[i][p-y].y - contours[i][p+y].y);
					//}
					//if ((diffX < 10) && (diffY < 10)){
						if (p1 == 0){
							p1 = p;
						}else {
							p2 = p;
						}
					//}
				}
			}
			if (p1 != 0){
				if (p2 == 0 ){
					contours[i].erase(contours[i].begin() + p1,contours[i].end());
				}else {
					contours[i].erase(contours[i].begin() + p2,contours[i].end());
					contours[i].erase(contours[i].begin(),contours[i].begin() + p1);
				}
			}

			//if ((contours[i].size() % 2 == 1) && (contours[i].size() > 2)){
			//	if (contours[i][(contours[i].size()/2)-0] ==contours[i][(contours[i].size()/2)+1]){
			//		contours[i].erase(contours[i].begin() + (contours[i].size()/2)+1,contours[i].end());
			//	}
			//}
		}
	}

		//Remove unnecessary points / simplify output
if (DYNAMIC_QUALITY == 1){
		totalPoints = 0;
		do{
			totalPoints = 0;
			for( int i = 0; i< contours.size(); i++ ){
				int temp = (int)contours[i].size() /2;
				if (temp <4){
					if (contours.size() > 50){
						contours.erase(contours.begin() + i);
						i--;
					}
				}else{
					for(int x = 1; x < temp; x++){
						if (x != contours[i].size()-1)
							contours[i].erase(contours[i].begin() + x);
					}
					totalPoints = totalPoints + (int)contours[i].size();
				}

			}
		} while (totalPoints > DYNAMIC_QUALITY_POINTS);
		for( int i = 0; i< contours.size(); i++ ){
			for(int pos = 0; pos < contours[i].size()-1; pos++){
				line(drawing, contours[i][pos], contours[i][pos+1], Scalar( 0, 255, 0 ), 1);
			}
		}
}else{
		totalPoints = 0;
		for( int i = 0; i< contours.size(); i++ ){
			for ( int y = 0; y < quaity; y++){
				int temp = (int)contours[i].size() /2;
				for(int x = 1; x < temp; x++){
					if (x < contours[i].size()-4)
						contours[i].erase(contours[i].begin() + x);
				}
			}
			totalPoints = totalPoints + (int)contours[i].size();
			for(int pos = 0; pos < contours[i].size()-1; pos++){
				line(drawing, contours[i][pos], contours[i][pos+1], Scalar( 0, 255, 0 ), 1);
			}
			//drawContours(drawing, contours, i, Scalar( 0, 255, 0 ), 1, 8, hierarchy, 0, Point());
		}
}



		//printf("\nPixles: %d", totalPoints);
	}

	/// Show in a window
	namedWindow( window_Con, CV_WINDOW_AUTOSIZE );
	imshow( window_Con, drawing );
}

// Mouse callback
void my_mouse_callback( int event, int xx, int yy, int flags, void* param ){

	if(event == CV_EVENT_MOUSEMOVE)
	{
		mouse_x = xx;
		mouse_y = yy;
	} else if (event == EVENT_LBUTTONDOWN){
		mouseDown = true;
		mouse_x = xx-1;
		mouse_y = yy;
		x=xx;
		y=yy;
	}else if (event == EVENT_LBUTTONUP){
		mouseDown = false;
	}else if (event == EVENT_RBUTTONDOWN){
		mouse_x = xx-1;
		mouse_y = yy;
		x=xx;
		y=yy;
		RmouseDown = true;
	}else if (event == EVENT_RBUTTONUP){
		RmouseDown = false;
	}else if (event == EVENT_MBUTTONDBLCLK){
		//pos++;
	}
}

void printHelp(){
	std::cout << "Usage: CVTest [-m mode] [-f filepath] [options]" << std::endl;
	std::cout << "       CVTest [--help] [-v]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << " -m	[0..3]			Mode to run, pic, vid, webcam" << std::endl;
	std::cout << " -f	[filepath]		File path to open" << std::endl;
	std::cout << " -w	[index]			Index of the webcam to open" << std::endl;
	std::cout << " -h	[host]			Host to connect to {127.0.0.1}" << std::endl;
	std::cout << " -p	[port]			Port on host to connect to {1234}" << std::endl;
	std::cout << " -n	[0/1]			Enable Network transmition {0}" << std::endl;
	std::cout << " -q	[0/1]			Enable dynamic quality {1}" << std::endl;
	std::cout << " -l	[points]		Max points per frame for dynamic quality {3000}" << std::endl;
	std::cout << " -b	[0/1]			Use the built in contours function {0}" << std::endl;
	std::cout << " -e	[0/1]			Enables the edge map window {0}" << std::endl;
	std::cout << " -g   [num]			The number of images to display in gallery mode {20}" << std::endl;
	std::cout << " -t	[num]			Number of seconds on gallery rotation {10}" << std::endl;
	std::cout << " --help				Print this info" << std::endl;
	std::cout << " -v				Display the version info" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Modes:" << std::endl;
	std::cout << "1. Image" << std::endl;
	std::cout << "2. Video" << std::endl;
	std::cout << "3. Webcam" << std::endl;
	std::cout << "4. Gallery" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Webcam Index" << std::endl;
	std::cout << "0. Default" << std::endl;
	std::cout << "else. index" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Version 1.0.0" << std::endl;
	std::cout << "" << std::endl;

}

int main( int argc, const char** argv )
{
	char* filepath = NULL;
	int webCamIndex = 0;
	char* host = "127.0.0.1";
	char* port = "1234";
	bool useWebcam = false;


	if (argc < 5){
		std::cout << "ERROR: Not enough argument" << std::endl;
		std::cout << "" << std::endl;
		printHelp();
		return 1;
	}else{
		bool recivedMode = false;
		bool recivedSource = false;
		for (int i = 1; i < argc; i++){
			if (i + 1 != argc){
				if (string(argv[i]) == "-m") {
					recivedMode = true;
					mode = atoi(argv[i+1]);
					if (mode < 1 || mode > 4){
						std::cout << "ERROR: Invalid Mode" << std::endl;
						std::cout << "" << std::endl;
						printHelp();
						return 1;
					}
					if (mode == 1){
						VIDEO = 0;
					}else if (mode == 2){
						VIDEO = 1;
					}else if (mode == 3){
						useWebcam = true;
						VIDEO = 1;
					}else if (mode == 4){
						recivedSource = true;
						VIDEO = 0;
						GALERY_MODE = 1;
					}
				}else if (string(argv[i]) == "-f"){
					recivedSource = true;
					filepath = (char*)argv[i+1];
				}else if (string(argv[i]) == "-w"){
					recivedSource = true;
					webCamIndex = atoi(argv[i+1]);
				}else if (string(argv[i]) == "-h"){
					host = (char*)argv[i+1];
				}else if (string(argv[i]) == "-p"){
					port = (char*)argv[i+1];
				}else if (string(argv[i]) == "-n"){
					if (string(argv[i+1]) == "1"){
						ENABLE_NETWORK = 1;
					}else{
						ENABLE_NETWORK = 0;
					}
				}else if (string(argv[i]) == "-q"){
					if (string(argv[i+1]) == "1"){
						DYNAMIC_QUALITY = 1;
					}else{
						DYNAMIC_QUALITY = 0;
					}
				}else if (string(argv[i]) == "-l"){
					DYNAMIC_QUALITY_POINTS = atoi(argv[i+1]);
				}else if (string(argv[i]) == "-b"){
					if (string(argv[i+1]) == "1"){
						BUILTIN_CONTOURS = 1;
					}else{
						BUILTIN_CONTOURS = 0;
					}
				}else if (string(argv[i]) == "-e"){
					if (string(argv[i+1]) == "1"){
						enableEdgeWindows = 1;
					}else{
						enableEdgeWindows = 0;
					}
				}else if (string(argv[i]) == "-g"){
					GaleryIndexMax = atoi(argv[i+1]);
				}else if (string(argv[i]) == "-t"){
					galeryWaitTime = atoi(argv[i+1]);
				}
			}
		}
		if (recivedMode == 0 || recivedSource == 0){
			std::cout << "ERROR: Mode and Source not specified (-m -f/-w)" << std::endl;
			std::cout << "ERROR: Not enough argument:" << std::endl;
			printHelp();
			return 1;
		}
	}

	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		//tcp::resolver::query query(tcp::v4(), "192.168.0.65", "1234");
		tcp::resolver::query query(tcp::v4(), host, port);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		tcp::socket s(io_service);

		if(ENABLE_NETWORK == 1){
			boost::asio::connect(s, iterator);
		}

		VideoCapture cap;

	double fps = 30;
	int delay = (int)(1000 / fps);
	clock_t startTime = clock();
	galleryTimer = clock()/CLOCKS_PER_SEC;
	bool bSuccess = true; // read a new frame from video


if (VIDEO == 1){
	if (useWebcam){
		cap.open(webCamIndex);
	}else{
		cap.open(filepath);
	}
	
	//VideoCapture cap("vid5.mp4"); // open the video file for reading
	//VideoCapture cap("vid1.avi");

	if ( !cap.isOpened() )  // if not success, exit program
	{
		cout << "Cannot open the video file" << endl;
		return -1;
	}

	fps = cap.get(CV_CAP_PROP_FPS);
	delay = (int)(1000 / fps);
	startTime = clock();
	bSuccess = cap.read(img); // read a new frame from video

	if (!bSuccess) //if not success, break loop
	{
		cout << "Cannot read the frame from video file" << endl;
	}
}else{

	if (GALERY_MODE == 1){
		img = imread("images/1.png", CV_LOAD_IMAGE_UNCHANGED); 
	}else{
		img = imread(filepath, CV_LOAD_IMAGE_UNCHANGED); 
	}
	

	if (img.empty()) //check whether the image is loaded or not
	{
		cout << "Error : Image cannot be loaded..!!" << endl;
		return -1;
	}

}


	imgWidth = img.size().width;
	imgHeight = img.size().height;

if (ENABLE_NETWORK == 1){
	unsigned char frameWidthHeight[4];
	frameWidthHeight[0] = (uint16_t)imgWidth & 0xFF;;
	frameWidthHeight[1] = (uint16_t)imgWidth >> 8;
	frameWidthHeight[2] = (uint16_t)imgHeight & 0xFF;;
	frameWidthHeight[3] = (uint16_t)imgHeight >> 8;

	boost::asio::write(s, boost::asio::buffer(frameWidthHeight, 4));
}

	namedWindow(window_Orig, CV_WINDOW_AUTOSIZE); //create a window with the name "MyWindow"
	imshow(window_Orig, img); //display the image which is stored in the 'img' in the "MyWindow" window

	// Set up the callback
	cvSetMouseCallback( window_Orig, my_mouse_callback, (void*) &img);

	/// Create a matrix of the same type and size as src (for dst)
	dst.create( img.size(), img.type() );

	/// Convert the image to grayscale
	cvtColor( img, src_gray, CV_BGR2GRAY );

if (BUILTIN_CONTOURS != 1){

	pixelLoopUp = (int**) calloc(src_gray.cols,sizeof(int*));

	for(int i = 0; i < src_gray.cols; i++){
		pixelLoopUp[i] = (int*) calloc(img.rows,sizeof(int));
	}

	for (int x1 = 0; x1 < src_gray.cols; x1++){
		for ( int y = 0; y < src_gray.rows; y++){
			pixelLoopUp[x1][y] = y*src_gray.cols*src_gray.channels() + x1*src_gray.channels() + 1;
		}
	}
}

	/// Create a window
if(enableEdgeWindows){
	namedWindow( window_Edge, CV_WINDOW_AUTOSIZE );
}
	namedWindow( window_Con, CV_WINDOW_AUTOSIZE );

	/// Create a Trackbar for user to enter threshold
	if (enableEdgeWindows){
		createTrackbar( "Min Threshold:", window_Edge, &lowThreshold, max_lowThreshold, CannyThreshold );
	} else {
		createTrackbar( "Min Threshold:", window_Con, &lowThreshold, max_lowThreshold, CannyThreshold );
	}
	if (!DYNAMIC_QUALITY){
		createTrackbar( "Quality:", window_Con, &quaity, 15, NULL );
	}

	/// Show the image
	CannyThreshold(0, 0);

	time(&start);
	int counter=0;
	startTicks = (double)getTickCount();

	while(cvWaitKey(15) != 27)
	{ 
		if((mouseDown || RmouseDown) && (x != mouse_x || y != mouse_y))
		{

if (VIDEO != 1){
			line(img, cvPoint(x,y), cvPoint(mouse_x,mouse_y), mouseDown ? CV_RGB(0,0,0) : CV_RGB(255,255,255),25);
}

			x = mouse_x;
			y = mouse_y;

			//circle(img,cvPoint(x,y),25,mouseDown ? CV_RGB(0,0,0) : CV_RGB(255,255,255),-1);

		}

		imshow(window_Orig, img);
		cvtColor( img, src_gray, CV_BGR2GRAY );
		CannyThreshold(0, 0);


		//Optimise rendering order

		vector<distP> used;
		for (int x = 0; x < contours.size(); x++){
			distP d;
			d.dist = 0;
			d.used = false;
			used.push_back(d);
		}
		bool sorted = false;
		int pos = 0;
		while (!sorted){
			sorted = true;
			int usedCount = 0;
			for (int x = 0; x < contours.size(); x++){
				used[x].dist = sqrt(((contours[pos][contours[pos].size()-1].x - contours[x][0].x) *
								(contours[pos][contours[pos].size()-1].x - contours[x][0].x)) + 
								((contours[pos][contours[pos].size()-1].y - contours[x][0].y) *
								(contours[pos][contours[pos].size()-1].y - contours[x][0].y)));
				if (used[x].used == false){
					sorted = false;
				}else{
					usedCount++;
				}
			}
			int closestPos = -1;
			int closestVal = INT_MAX;
			for (int x = 0; x < contours.size(); x++){
				if ((!used[x].used) && (x != pos) && (used[x].dist < closestVal)){
					closestPos = x;
					closestVal = used[x].dist;
				}
			}
			if (closestPos != -1){
				used[pos].used = true;
				iter_swap(used.begin() + pos + 1, used.begin() + closestPos);
				iter_swap(contours.begin() + pos + 1, contours.begin() + closestPos);
			}else if (usedCount == contours.size()-1){
				sorted =  true;
			}
			pos++;
		}


			int CountedPointsTotal = 0;
			for( int i = 0; i< contours.size(); i++ ){
					CountedPointsTotal = CountedPointsTotal + (int)contours[i].size();
			}
			uint16_t pointsNum = CountedPointsTotal + (int)contours.size()*2;
			printf("\nPixles: %d", pointsNum);


		if (ENABLE_NETWORK == 1){
			unsigned char pointsNumChar[2];
			int counterNum = 0;


			pointsNumChar[0] = pointsNum & 0xFF;;
			pointsNumChar[1] = pointsNum >> 8;;
			boost::asio::write(s, boost::asio::buffer(pointsNumChar, 2));
#if SAVE_FILE == 1
			FILE* pFile = NULL;
			if (!savedFile)
				pFile = fopen("adamDump.csv", "w");
			if(pFile)
				fprintf(pFile, "0x%x, 0x%x,\n", pointsNumChar[0], pointsNumChar[1]);
#endif 
			unsigned char packet[6];
			for( int i = 0; i< contours.size(); i++ ){
				point p1;
				p1.x = contours[i][0].x;
				p1.y = contours[i][0].y;
				p1.brightness = 0;
				p1.resereved = 0;

				packet[0] = p1.brightness;
				packet[1] = p1.resereved;
				packet[2] = p1.x & 0xFF;
				packet[3] = p1.x >> 8;
				packet[4] = p1.y & 0xFF;
				packet[5] = p1.y >> 8;

				boost::asio::write(s, boost::asio::buffer(packet, 6));
#if SAVE_FILE == 1
				if(pFile)
					fprintf(pFile, "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n", packet[1], packet[0], packet[2], packet[3], packet[4], packet[5]);
#endif 
				counterNum++;
				for(int pos = 0; pos < contours[i].size(); pos++){
					point p;
					p.x = contours[i][pos].x;
					p.y = contours[i][pos].y;
					p.brightness = 255;
					p.resereved = 0;

					packet[0] = p.brightness;
					packet[1] = p.resereved;
					packet[2] = p.x & 0xFF;
					packet[3] = p.x >> 8;
					packet[4] = p.y & 0xFF;
					packet[5] = p.y >> 8;

					boost::asio::write(s, boost::asio::buffer(packet, 6));
#if SAVE_FILE == 1
					if(pFile)
						fprintf(pFile, "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n", packet[1], packet[0], packet[2], packet[3], packet[4], packet[5]);
#endif 
					counterNum++;
				}
				point p2;
				p2.x = contours[i][contours[i].size()-1].x;
				p2.y = contours[i][contours[i].size()-1].y;
				p2.brightness = 0;
				p2.resereved = 0;

				packet[0] = p2.brightness;
				packet[1] = p2.resereved;
				packet[2] = p2.x & 0xFF;
				packet[3] = p2.x >> 8;
				packet[4] = p2.y & 0xFF;
				packet[5] = p2.y >> 8;

				boost::asio::write(s, boost::asio::buffer(packet, 6));
#if SAVE_FILE == 1
				if(pFile)
					fprintf(pFile, "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n", packet[1], packet[0], packet[2], packet[3], packet[4], packet[5]);
#endif 
				counterNum++;

			}
			counterNum = 0;
#if SAVE_FILE == 1
			if(pFile)
				fclose(pFile);
#endif 
		}


if( VIDEO == 1){
		bool bSuccess = cap.read(img); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read the frame from video file" << endl;
			break;
		}
		//while (clock() - startTime < 1000) {
		//	waitKey(1);
		//}

		cap.set(CV_CAP_PROP_POS_MSEC, clock() - startTime);

		//startTime = clock();
}

		//time(&endTime);
		counter++;
		//int p = countNonZero(detected_edges);
		//double sec=difftime(endTime,start);
		//double fps=counter/sec;
		//printf("\n%lf, %d",fps, p);
		double feq = getTickFrequency();
		double ticks = (double)getTickCount();
		double tmpp = ( ticks - startTicks);
		double tmppp = tmpp / feq;
		if ( tmppp > 1){
			startTicks = (double)getTickCount();
			fpsRate = counter;
			counter = 0;
		}
		printf("\nFPS: %d", fpsRate);

		if (GALERY_MODE){
			if (clock()/CLOCKS_PER_SEC > galleryTimer + galeryWaitTime){
				//int GaleryIndex = 1;
				//int GaleryIndexMax = 20;

				GaleryIndex+=1;

				if (GaleryIndex > GaleryIndexMax){
					GaleryIndex = 1;
				}

				char buf[4];
				_itoa(GaleryIndex, buf, 10);
				char path[80];
				strcpy(path,"images/");
				strcat(path,buf);
				strcat(path,".png");
				
				img = imread(path, CV_LOAD_IMAGE_UNCHANGED); 

				if (img.empty()) //check whether the image is loaded or not
				{
					cout << "Error : Image cannot be loaded..!!" << endl;
					return -1;
				}

				galleryTimer = clock() / CLOCKS_PER_SEC;
			}
		}

	}


	waitKey(0); //wait infinite time for a keypress

if (BUILTIN_CONTOURS != 1){
	for(int i = 0; i < img.rows; i++){
		free(pixelLoopUp[i]);
	}

	free(pixelLoopUp);
}

	destroyWindow(window_Orig); //destroy the window with the name, "MyWindow"
	if(enableEdgeWindows){
		destroyWindow(window_Edge);
	}



	}catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}