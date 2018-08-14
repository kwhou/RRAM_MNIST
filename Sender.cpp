#include "Sender.h"
#include <ctime>

void Sender::test_script(void)
{
	sc_lv<DATA_WIDTH> dout;
	ifstream f;
	
	cs.write(true);
	io.write(high_impedance);
	
	/*
	// Write Enable
	cout << sc_time_stamp() << " " << name() << ": Write Enable Instruction" << endl;
	wait(clk.negedge_event());
	cs.write(false);
	io.write(INS_WRITE_ENABLE);
	wait(clk.negedge_event());
	cs.write(true);
	io.write(high_impedance);
	
	// Page Write
	cout << sc_time_stamp() << " " << name() << ": Page Write Instruction" << endl;
	wait(clk.negedge_event());
	cs.write(false);
	io.write(INS_PAGE_WRITE);
	wait(clk.negedge_event());
	io.write("00000000000000000000000000000000"); //Address
	f.open("weights.txt");
	if (f == NULL) {
		cout << "Weight file does not exist." << endl;
		return;
	}
	float weight_float;
	for(int i=0; i<64; i++)
	{
		wait(clk.negedge_event());
		f >> weight_float;
		long *weight_pointer = (long *)&weight_float;
		sc_uint<DATA_WIDTH> weight_sc_uint = *weight_pointer;
		sc_lv<DATA_WIDTH> weight_sc_lv = weight_sc_uint;
		io.write(weight_sc_lv);
	}
	wait(clk.negedge_event());
	cs.write(true);
	io.write(high_impedance);
	
	// Read Status Register
	cout << sc_time_stamp() << " " << name() << ": Read Status Instruction" << endl;
	wait(clk.negedge_event());
	cs.write(false);
	io.write(INS_READ_STATUS_REG);
	wait(clk.negedge_event());
	cout << sc_time_stamp() << " " << name() << ": Waiting for BUSY bit to go low" << endl;
	do {
		wait(clk.posedge_event());
		dout = io.read();
	} while (dout[0] != SC_LOGIC_0);
	wait(clk.negedge_event());
	cs.write(true);
	*/
	
	
	// Weight Write
	cout << sc_time_stamp() << " " << name() << ": Weight Write Instruction" << endl;
	wait(clk.negedge_event());
	cs.write(false);
	io.write(INS_WEIGHT_WRITE); 
	wait(clk.negedge_event());
	cs.write(true);
	io.write(high_impedance);
	
	
	// Read
	cout << sc_time_stamp() << " " << name() << ": Read Instruction" << endl;
	wait(clk.negedge_event());
	cs.write(false);
	io.write(INS_READ);
	wait(clk.negedge_event());
	io.write("00000000000000000000000000000000"); //Address
	wait(clk.negedge_event());
	for(int i=0; i<NUM_OF_INPUT_PIXELS*NUM_OF_OUTPUT_NEURONS; i++)
	{
		wait(clk.posedge_event());
		dout = io.read();
		sc_uint<DATA_WIDTH> dout_sc_uint = dout;
		long dout_long = dout_sc_uint;
		float *dout_p = (float *)&dout_long;
		float dout_float = *dout_p;
		// cout << dout_float << endl;
	}
	wait(clk.negedge_event());
	cs.write(true);
	
	
	// Inference
	cout << sc_time_stamp() << " " << name() << ": Start MNIST testing ..." << endl;
	f.open("pixels.txt");
	int num_images = 0;
	float accuracy = 0;
	f >> num_images; num_images = 10;
	time_t t1 = time(0);
	for(int k=0; k<num_images; k++)
	{
		// Inference
		// cout << sc_time_stamp() << " " << name() << ": Inference Instruction" << endl;
		wait(clk.negedge_event());
		cs.write(false);
		io.write(INS_INFERENCE);
		for(int i=0; i<NUM_OF_INPUT_PIXELS; i++)
		{
			wait(clk.negedge_event());
			float pix_float;
			f >> pix_float;
			long *pix_pointer = (long *)&pix_float;
			sc_uint<DATA_WIDTH> pix_sc_uint = *pix_pointer;
			sc_lv<DATA_WIDTH> pix  = pix_sc_uint;
			io.write(pix);
		}
		wait(clk.negedge_event());
		cs.write(true);
		io.write(high_impedance);
		
		// Read Status Register
		// cout << sc_time_stamp() << " " << name() << ": Read Status Instruction" << endl;
		wait(clk.negedge_event());
		cs.write(false);
		io.write(INS_READ_STATUS_REG);
		wait(clk.negedge_event());
		// cout << sc_time_stamp() << " " << name() << ": Waiting for BUSY bit to go low" << endl;
		do {
			wait(clk.posedge_event());
			dout = io.read();
		} while (dout[0] != SC_LOGIC_0);
		wait(clk.negedge_event());
		cs.write(true);
		
		// Read Neuron Status
		// cout << sc_time_stamp() << " " << name() << ": Read Neuron Status Instruction" << endl;
		wait(clk.negedge_event());
		cs.write(false);
		io.write(INS_READ_NEURON_STATUS);
		wait(clk.negedge_event());
		wait(clk.posedge_event());
		dout = io.read();
		sc_uint<DATA_WIDTH> dout_sc_uint = dout;
		wait(clk.negedge_event());
		cs.write(true);
		
		// Read Neuron Value
		// cout << sc_time_stamp() << " " << name() << ": Read Neuron Status Instruction" << endl;
		wait(clk.negedge_event());
		cs.write(false);
		io.write(INS_READ_NEURON_VALUE);
		wait(clk.negedge_event());
		for(int j=0; j<NUM_OF_OUTPUT_NEURONS; j++)
		{
			wait(clk.posedge_event());
			dout = io.read();
			sc_uint<DATA_WIDTH> dout_sc_uint = dout;
			long dout_long = dout_sc_uint;
			float *dout_p = (float *)&dout_long;
			float dout_float = *dout_p;
			cout << "Neuron " << j << ": " << dout_float << endl;
		}
		wait(clk.negedge_event());
		cs.write(true);
		
		int pred = dout_sc_uint.range(DATA_WIDTH-1, STS_CLASS);
		int actual = 0;
		f >> actual;
		if (actual == pred) {
			accuracy++;
		}
		// else {
			cout << "Prediction: " << pred << ", " << "Label: " << actual << endl;
		// }
	}
	time_t t2 = time(0);
	accuracy = 100.0 * accuracy / (float)num_images;
	cout << "Accuracy = " << accuracy << endl;
	cout << "Simulation time = " << sc_time_stamp() << endl;
	cout << "Program execution time = " << t2-t1  << endl;
}
