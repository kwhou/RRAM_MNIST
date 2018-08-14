all: test.exe

test.exe: main.o Neuron.o RRAM_MNIST.o Sender.o
	g++ -L$(SYSTEMC_HOME)/lib-$(ARCH) main.o Neuron.o RRAM_MNIST.o Sender.o -lsystemc -o test.exe

main.o: main.cpp Config.h RRAM_MNIST.h Sender.h
	g++ -c -I$(SYSTEMC_HOME)/include main.cpp

Neuron.o: Neuron.cpp Neuron.h Config.h
	g++ -c -I$(SYSTEMC_HOME)/include Neuron.cpp

RRAM_MNIST.o: RRAM_MNIST.cpp RRAM_MNIST.h Config.h Neuron.h
	g++ -c -I$(SYSTEMC_HOME)/include RRAM_MNIST.cpp

Sender.o : Sender.cpp Sender.h Config.h
	g++ -c -I$(SYSTEMC_HOME)/include Sender.cpp

run:
	./test.exe

clean:
	rm *.o test.exe *.vcd *.fsdb
