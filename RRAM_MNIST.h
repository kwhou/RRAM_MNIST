#ifndef _RRAM_MNIST_H_
#define _RRAM_MNIST_H_

#include <systemc>
#include <iostream>
#include <fstream>
#include <cmath>
#include "Config.h"
#include "Neuron.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(RRAM_MNIST)
{
	sc_in<bool> clk_p;
	sc_in<bool> cs_p;
	sc_inout< sc_lv<DATA_WIDTH> > io_p;
	
	bool cs_active;
	sc_lv<DATA_WIDTH> io_high_impedance;
	sc_uint<INS_WIDTH> instruction;
	sc_uint<ADDR_WIDTH> address;
	sc_uint<DATA_WIDTH> read_value;
	sc_uint<DATA_WIDTH> write_value;
	
	sc_signal< sc_uint<DATA_WIDTH> > page_buffer [NUM_OF_OUTPUT_NEURONS];
	
	bool data[NUM_OF_ROWS][NUM_OF_COLS];
	
	sc_uint<STATUS_REG_WIDTH> status_register_1;
	sc_uint<STATUS_REG_WIDTH> status_register_2;

	sc_event begin_get_instruction;
	sc_event begin_read;
	sc_event begin_write_enable;
	sc_event begin_page_write;
	sc_event begin_weight_write;
	sc_event begin_page_erase;
	sc_event begin_inference;
	sc_event begin_read_neuron_value;
	sc_event begin_read_neuron_status;
	sc_event begin_read_status_register;
	sc_event begin_read_weights;
	
	Neuron neuron;
	sc_signal<bool> reset;
	sc_signal<bool> enable;
	sc_signal<bool> valid;
	sc_signal< sc_uint<DATA_WIDTH>, SC_MANY_WRITERS > status;
	sc_fifo< sc_uint<DATA_WIDTH> > pixel_fifo;
	
	sc_signal< sc_uint<DATA_WIDTH> > activation [NUM_OF_OUTPUT_NEURONS];
	
	SC_CTOR(RRAM_MNIST):
		cs_active(false),
		io_high_impedance(SC_LOGIC_Z),
		status_register_1(0),
		status_register_2(0),
		neuron("neuron"),
		pixel_fifo(FIFO_SIZE)
	{
		neuron.clk_p(clk_p);
		neuron.enable_p(enable);
		neuron.reset_p(reset);
		neuron.valid_p(valid);
		neuron.status_p(status);
		neuron.pixel_fifo_p(pixel_fifo);
		
		for (int i=0; i<NUM_OF_OUTPUT_NEURONS; i++) {
			neuron.weight_p[i](page_buffer[i]);
			neuron.activation_p[i](activation[i]);
		}
		
		enable.write(true);
		reset.write(true);
		valid.write(false);
		
		for (int i=0; i<NUM_OF_ROWS; i++) {
			for(int j=0; j<NUM_OF_COLS; j++) {
				data[i][j] = true;
			}
		}
		
		SC_THREAD(read_cs);
			sensitive << cs_p.value_changed();
		SC_THREAD(get_instruction);
			sensitive << begin_get_instruction;
		SC_THREAD(page_read);
			sensitive << begin_read;
		SC_THREAD(write_enable);
			sensitive << begin_write_enable;
		SC_THREAD(page_write);
			sensitive << begin_page_write;
		SC_THREAD(page_erase);
			sensitive << begin_page_erase;
		SC_THREAD(weight_write);
			sensitive << begin_weight_write;
		SC_THREAD(read_neuron_value);
			sensitive << begin_read_neuron_value;
		SC_THREAD(read_neuron_status);
			sensitive << begin_read_neuron_status;
		SC_THREAD(read_status_register);
			sensitive << begin_read_status_register;
		SC_THREAD(inference);
			sensitive << begin_inference;
		SC_THREAD(read_weights);
			sensitive << begin_read_weights;
	}
	
	void read_cs(void);
	void get_instruction(void);
	void page_read(void);
	void write_enable(void);
	void page_write(void);
	void page_erase(void);
	void read_status_register(void);
	void weight_write(void);
	void read_neuron_value(void);
	void read_neuron_status(void);
	void inference(void);
	void read_weights(void);
};

#endif
