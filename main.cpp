#include "RRAM_MNIST.h"
#include "Sender.h"

int sc_main(int argc, char* argv[])
{
	sc_clock clk("clk", 20, SC_NS, 0.5, 10, SC_NS, true);	
	sc_signal<bool> cs;
	sc_signal<sc_lv<DATA_WIDTH>, SC_MANY_WRITERS > io;
	
	RRAM_MNIST RRAM("RRAM");
		RRAM.clk_p(clk);
		RRAM.cs_p(cs);
		RRAM.io_p(io);
		
	Sender Sender("Sender");
		Sender.clk(clk);
		Sender.cs(cs);
		Sender.io(io);
	
	sc_trace_file* tracefile;
	tracefile = sc_create_vcd_trace_file("waveform");
	sc_trace(tracefile, clk, "CLK");
	sc_trace(tracefile, cs, "CS");
	sc_trace(tracefile, io, "IO");
	
	sc_start();
	
	sc_close_vcd_trace_file(tracefile);
	return 0;
}
