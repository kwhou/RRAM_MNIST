#ifndef _NEURON_H_
#define _NEURON_H_

#include <systemc>
#include <iostream>
#include "Config.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(Neuron)
{
	sc_in<bool> clk_p;
	sc_in<bool> reset_p; //low active
	sc_in<bool> enable_p; //low active
	sc_in<bool> valid_p;
	sc_out< sc_uint<DATA_WIDTH> > status_p;
	sc_vector< sc_out< sc_uint<DATA_WIDTH> > > activation_p;
	sc_vector< sc_in< sc_uint<DATA_WIDTH> > > weight_p;
	sc_fifo_in< sc_uint<DATA_WIDTH> > pixel_fifo_p;
	
	public:
		float activation[NUM_OF_OUTPUT_NEURONS];
		sc_uint<DATA_WIDTH> status;
		
	SC_CTOR(Neuron):
		clk_p("clk_p"),
		reset_p("reset_p"),
		enable_p("enable_p"),
		valid_p("valid_p"),
		status_p("status_p"),
		activation_p("activation_p", NUM_OF_OUTPUT_NEURONS),
		weight_p("weight_p", NUM_OF_OUTPUT_NEURONS),
		pixel_fifo_p("pixel_fifo_p")
	{
		status = 0;
		for (int i=0; i<NUM_OF_OUTPUT_NEURONS; i++)
		{
			activation[i] = 0;
		}
		SC_THREAD(reset);
		SC_THREAD(MAC);
	}
	
	void reset(void);
	void MAC(void);
};

#endif
