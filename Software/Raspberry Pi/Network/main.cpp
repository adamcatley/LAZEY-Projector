
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <stdint.h>
#include "lazey.h"
#include <bcm2835.h>


using boost::asio::ip::tcp;

const int max_length = 1024;

typedef boost::shared_ptr<tcp::iostream> stream_ptr;

struct PixPoint {
	uint16_t x;
	uint16_t y;
	uint8_t brightness;
	uint8_t reserved;
};


void session(stream_ptr sock)
{

  try
  {
	std::cout << "Connected\n";
	  char widthHeight[4];
	  sock->read(widthHeight,4);
	  uint16_t width = widthHeight[0] | uint16_t(widthHeight[1]) << 8;
	  uint16_t height = widthHeight[2] | uint16_t(widthHeight[3]) << 8;
	std::cout << "Width: " << width << ", Height: " << height << "\n";
	//Creates a display object with an effective resolution of 800x800
	lazey LazeyProjector((uint16_t)width,(uint16_t)height,
	           4,
	           BCM2835_SPI_BIT_ORDER_MSBFIRST,
	           BCM2835_SPI_MODE0,
	           BCM2835_SPI_CLOCK_DIVIDER_8,
	           BCM2835_SPI_CS0,
	           LOW);

	//LazeyProjector.extendFramePoints = true;


	std::cout << "SPI setup\nWaiting for data...\n";

	
    for (;;)
    {
      char data[6];

      boost::system::error_code error;
      //printf("Data...");
	sock->read(data, 2);
	//printf("Recived\n");
	error = sock->error();
      //size_t length = sock->read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof){
		std::cout << "EOF\n";//throw boost::system::system_error(error);//break; // Connection closed cleanly by peer.
		throw boost::system::system_error(error);
      }else if (error){
        throw boost::system::system_error(error); // Some other error.
	std::cout << "ERROE:\n";
      }

	if (*sock){
	uint16_t pixelNum = data[0] | uint16_t(data[1]) << 8;
	//std::cout << "Pixles to recived: " << pixelNum << "\n";

	if(pixelNum != 0){

	for(int pixelPos = 0; pixelPos < pixelNum;){

		//std::cout << "count: " << pixelPos << "\n";
		sock->read(data, 6);

		error = sock->error();
	      if (error == boost::asio::error::eof){
	        std::cout << "EOF\n";//throw boost::system::system_error(error); //break; // Connection closed cleanly by peer.
	      	throw boost::system::system_error(error);
	      } else if (error){
	        throw boost::system::system_error(error); // Some other error.
	      }

		if (*sock){
		//std::cout << "count: " << pixelPos << "\n";
	
		PixPoint p;
		p.x = data[2] | uint16_t(data[3]) << 8;
		p.y = data[4] | uint16_t(data[5]) << 8;
		p.brightness = data[0];
		p.reserved = data[1];

		LazeyProjector.AddRawPoint(p.x,p.y,p.reserved,p.brightness);

		//std::cout << "X: " << (int)p.x << "\nY: " << (int)p.y << "\nBrightness: " << (int)p.brightness << "\nReserved: " << (int)p.reserved << "\n";

		pixelPos++;

		}
	}

	}



	LazeyProjector.renderFrame();


	}


      //boost::asio::write(*sock, boost::asio::buffer(data, length));
    }
  }
  catch (std::exception& e)
  {
  std::cout << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_service& io_service, short port)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  std::cout << "Waiting for client...\n";
  for (;;)
  {
    //socket_ptr sock(new tcp::socket(io_service));
    //a.accept(*sock);

    stream_ptr stream(new tcp::iostream);;
    a.accept(*stream->rdbuf());
    //boost::thread t(boost::bind(session, sock));
    std::cout << "Got Client...\n";
    session(stream);
    std::cout << "Getting new client...\n";
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: Network  <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    server(io_service, atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

