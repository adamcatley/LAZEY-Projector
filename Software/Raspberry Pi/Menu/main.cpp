#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>


#define READ 0
#define WRITE 1

using boost::asio::ip::udp;

pid_t popen2(char **command, int *infp, int *outfp) {

	    int p_stdin[2], p_stdout[2];
	        pid_t pid;

		    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
			            return -1;

		        pid = fork();

			    if (pid < 0)
				            return pid;
			        else if (pid == 0)
					    {
						            close(p_stdin[WRITE]);
							            dup2(p_stdin[READ], READ);
								            close(p_stdout[READ]);
									            dup2(p_stdout[WRITE], WRITE);

										            execvp(*command, command);
											            perror("execvp");
												            exit(1);
													        }

				    if (infp == NULL)
					            close(p_stdin[WRITE]);
				        else
						        *infp = p_stdin[WRITE];

					    if (outfp == NULL)
						            close(p_stdout[READ]);
					        else
							        *outfp = p_stdout[READ];

						    return pid;
}

        char * commands[][3] = {
               {"../Network/Network.o", "1234", NULL},
               {"../Tetris/Tetris.o", NULL, NULL},
               {"../Circles/Lazey.o", "6", NULL},
               {"../SquareDemo/Lazey.o", "1", NULL},
               {"../SquareDemo/Lazey.o", "2", NULL},
               {"../SquareDemo/Lazey.o", "3", NULL},
               {"../SquareDemo/Lazey.o", "4", NULL},
               {"../SquareDemo/Lazey.o", "5", NULL},
               {"../Blank/blank.o", NULL, NULL}
	        };


void catch_int(int sig_num){
	printf("Exiting...\n");
	pid_t id2 = popen2(commands[8], NULL, NULL);
	sleep(2);
	kill(id2, 1);
	exit(1);

}

bool exitNow = false;

int main()

{

	signal(SIGINT, catch_int);

	pid_t id = NULL;
	char commandIndex = 0;

        try
	  {
		boost::asio::io_service io_service;
		udp::socket socket(io_service, udp::endpoint(udp::v4(), 8080));


		for (;;){
			boost::array<char, 1> recv_buf;
			udp::endpoint remote_endpoint;
			boost::system::error_code error;
			socket.receive_from(boost::asio::buffer(recv_buf),
			remote_endpoint, 0, error);

			if (error && error != boost::asio::error::message_size)
			throw boost::system::system_error(error);

			char command = recv_buf[0];

			if(id != NULL && id != -1)
				kill(id,1);
			//printf("Command: %x", command);

		/*	if (command == 1){
				command = {"../Network/Network.o", "1234", NULL};
			}else if (command == 2){
				command = {"/../Tetris/Tetris.o", NULL};
			}else if (command == 3){
				command = {"../circles/Lazey.o", "6", NULL};
			}else if (command == 4){
				command = {"../squareDemo/Lazey.o", "1", NULL};
			}else if (command == 5){
				command = {"../squareDemo/Lazey.o", "2", NULL};
			}else if (command == 6){
				command = {"../squareDemo/Lazey.o", "3", NULL};
			}else if (command == 7){
				command = {"../squareDemo/Lazey.o", "4", NULL};
			}else if (command == 8){
				command = {"../squareDemo/Lazey.o", "5", NULL};
			}*/
			commandIndex = command -1;
			id = popen2(commands[commandIndex], NULL, NULL);
		}
	  }
	 catch (std::exception& e)
	   {
		std::cerr << e.what() << std::endl;
	}
	

   return 1;
}

