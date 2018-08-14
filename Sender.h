#ifndef _SENDER_H_
#define _SENDER_H_

#include <systemc>
#include <iostream>
#include <fstream>
#include "Config.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(Sender)
{
	sc_in<bool> clk;
	sc_out<bool> cs;
	sc_inout< sc_lv<DATA_WIDTH> > io;
	
	sc_lv<DATA_WIDTH> high_impedance;
	
	SC_CTOR(Sender):
		high_impedance(SC_LOGIC_Z)
	{
		SC_THREAD(test_script);
	}
	
	void test_script(void);
};
#endif
