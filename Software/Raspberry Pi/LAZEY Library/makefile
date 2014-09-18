all:
	 g++ -Wall -fPIC -c mmapGpio.cpp spi.cpp lazey.cpp -lrt -l bcm2835 -D__STDC_LIMIT_MACROS
	 gcc -shared -Wl,-soname,liblazey.so.1 -o liblazey.so.1.0 mmapGpio.o spi.o lazey.o
	 ln -sf liblazey.so.1.0 liblazey.so
	 ln -sf liblazey.so.1.0 liblazey.so.1
	 export LD_LIBRARY_PATH=.
