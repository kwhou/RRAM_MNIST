#include "RRAM_MNIST.h"

void RRAM_MNIST::read_cs(void)
{
	bool cs;
	while(true)
	{
		wait(SC_ZERO_TIME);
		cs = cs_p->read();
		if (!cs_active && cs == false)
		{
			// cout << sc_time_stamp() << " " << name() << ": cs is active" << endl;
			cs_active = true;
			begin_get_instruction.notify();
		}
		else if (cs_active && cs == true)
		{
			// cout << sc_time_stamp() << " " << name() << ": cs is inactive" << endl;
			cs_active = false;
		}
		wait();
	}
}

void RRAM_MNIST::get_instruction()
{
	while(1)
	{
		wait();
		
		wait(clk_p->posedge_event() | cs_p->default_event());
		if (cs_p->event()) {
			continue;
		}
		sc_uint<DATA_WIDTH> io_in = io_p->read();
		instruction = io_in.range(7,0);
		
		if (status_register_1[0] != 0 && instruction.to_string(SC_BIN) != INS_READ_STATUS_REG && instruction.to_string(SC_BIN) != INS_READ_NEURON_STATUS) {
			cout << sc_time_stamp() << " " << name() << ": busy ... instruction " << instruction << " ignored" << endl;
			continue;
		}
		
		if (instruction.to_string(SC_BIN) == INS_READ) {
			cout << sc_time_stamp() << " " << name() << ": receive READ instruction " << endl;
			begin_read.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_WRITE_ENABLE) {
			cout << sc_time_stamp() << " " << name() << ": receive WRITE_ENABLE instruction " << endl;
			begin_write_enable.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_PAGE_WRITE) {
			cout << sc_time_stamp() << " " << name() << ": receive PAGE_WRITE instruction " << endl;
			begin_page_write.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_WEIGHT_WRITE) {
			cout << sc_time_stamp() << " " << name() << ": receive WEIGHT_WRITE instruction " << endl;
			begin_weight_write.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_PAGE_ERASE) {
			cout << sc_time_stamp() << " " << name() << ": receive PAGE_ERASE instruction " << endl;
			begin_page_erase.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_READ_STATUS_REG) {
			// cout << sc_time_stamp() << " " << name() << ": receive READ_STATUS_REG instruction " << endl;
			begin_read_status_register.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_INFERENCE) {
			// cout << sc_time_stamp() << " " << name() << ": receive INFERENCE instruction " << endl;
			begin_inference.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_READ_NEURON_VALUE) {
			// cout << sc_time_stamp() << " " << name() << ": receive READ_NEURON_VALUE instruction " << endl;
			begin_read_neuron_value.notify();
		}
		else if (instruction.to_string(SC_BIN) == INS_READ_NEURON_STATUS) {
			// cout << sc_time_stamp() << " " << name() << ": receive READ_NEURON_STATUS instruction " << endl;
			begin_read_neuron_status.notify();
		}
		else {
			cout << sc_time_stamp() << " " << name() << ": instruction not recognized" << endl;
		}
	}
}

void RRAM_MNIST::page_read(void)
{
	while(true)
	{
		wait();
		wait(clk_p->posedge_event() | cs_p->default_event());
		if (cs_p->event()) {
			continue;
		}
		sc_uint<DATA_WIDTH> io_in = io_p->read();
		address = io_in.range(ADDR_WIDTH-1,0);
		
		int row = address / NUM_OF_ROWS;
		int col = address % NUM_OF_ROWS;
		
		while (true)
		{
			wait(clk_p->negedge_event() | cs_p->default_event());
			if(cs_p->event()) {
				break;
			}
			
			bool read_row_finish = false;
			for(int j=0; j<DATA_WIDTH; j++)
			{
				if ((col+j) >= NUM_OF_COLS)
				{
					row++;
					if (row >= NUM_OF_ROWS)
					{
						row = 0;
					}
					col = 0;
					read_row_finish = true;
					break;
				}
				
				read_value[DATA_WIDTH-j-1] = data[row][col+j];
			}
			
			if (read_row_finish == false)
			{
				col += DATA_WIDTH;
				sc_lv<DATA_WIDTH> io_out = read_value;
				io_p->write(io_out);
			}
		}
		wait(clk_p->negedge_event());
		// io_p->write(io_high_impedance);
	}
}

void RRAM_MNIST::write_enable(void)
{
	while(true)
	{
		wait();
		// Set write enable bit = 1
		status_register_1[1] = 1;
	}
}

void RRAM_MNIST::page_write(void)
{
	while(true)
	{
		wait();
		
		// Check write enable/disable
		if (status_register_1[1] != 1) {
			cout << sc_time_stamp() << " " << name() << ": write disable" << endl;
			continue;
		}
		
		wait(clk_p->posedge_event() | cs_p->default_event());
		if (cs_p->event()) {
			continue;
		}
		sc_uint<DATA_WIDTH> io_in = io_p->read();
		address = io_in.range(ADDR_WIDTH-1,0);
		
		int row = address / NUM_OF_ROWS;
		int col = address % NUM_OF_ROWS;
		
		while(true)
		{
			wait(clk_p->posedge_event() | cs_p->default_event());
			if (cs_p->event()) {
				break;
			}
			sc_uint<DATA_WIDTH> io_in = io_p->read();
			write_value = io_in;

			for(int j=0; j<DATA_WIDTH; j++)
			{
				if(col >= NUM_OF_COLS)
				{
					col = 0;
				}
				data[row][col] = write_value[DATA_WIDTH-j-1];
				col++;
			}
		}
		
		status_register_1[0] = 1;
		wait(PAGE_WRITE_LATENCY, SC_NS);
		status_register_1[1] = 0;
		status_register_1[0] = 0;
	}
}

void RRAM_MNIST::page_erase(void)
{
	while(true)
	{
		wait();
		
		// Check write enable/disable
		if (status_register_1[1] != 1) {
			cout << sc_time_stamp() << " " << name() << ": write disable" << endl;
			continue;
		}
		
		wait(clk_p->posedge_event() | cs_p->default_event());
		if (cs_p->event()) {
			continue;
		}
		sc_uint<DATA_WIDTH> io_in = io_p->read();
		address = io_in.range(ADDR_WIDTH-1,0);
		
		int row = address / NUM_OF_ROWS;
		int col = 0;

		while(col<NUM_OF_COLS)
		{
			data[row][col] = false;
			col++;
		}
		
		status_register_1[0] = 1;
		wait(PAGE_ERASE_LATENCY, SC_NS);
		status_register_1[1] = 0;
		status_register_1[0] = 0;
	}
}

void RRAM_MNIST::read_status_register(void)
{
	while(true)
	{
		wait();
		while (true)
		{
			wait(clk_p->negedge_event() | cs_p->default_event());
			if (cs_p->event()) {
				break;
			}
			sc_lv<DATA_WIDTH> io_out = status_register_1;
			io_p->write(io_out);
		}
		io_p->write(io_high_impedance);
	}
}

void RRAM_MNIST::weight_write(void)
{
	ifstream f;
	
	while(true)
	{
		wait();
		
		f.open("weights.txt");
		if (f == NULL) {
			cout << "Weight file does not exist." << endl;
			continue;
		}
		
		int row = 0;
		int col = 0;
		int beg = 0;
		
		int num_bit_pixel = NUM_OF_OUTPUT_NEURONS * DATA_WIDTH;
		int num_weights = NUM_OF_INPUT_PIXELS * NUM_OF_OUTPUT_NEURONS;
		
		float weight_float;
		
		for (int i=0; i<num_weights; i++)
		{
			f >> weight_float;
			
			long *weight_pointer = (long *)&weight_float;
			sc_uint<DATA_WIDTH> weight_sc_uint = *weight_pointer;
			
			for(int j=0; j<DATA_WIDTH; j++)
			{
				data[row][col] = weight_sc_uint[DATA_WIDTH-j-1];
				col++;
			}
			
			if (((i+1) % NUM_OF_OUTPUT_NEURONS == 0) || col >= NUM_OF_COLS)
			{
				col = beg;
				row++;
			}
			if (row >= NUM_OF_ROWS)
			{
				row = 0;
				beg += num_bit_pixel;
				col = beg;
			}
		}
	}
}

void RRAM_MNIST::read_neuron_value(void)
{
	while(true)
	{
		wait();
		int act = 0;
		while (true)
		{
			wait(clk_p->negedge_event() | cs_p->default_event());
			if (cs_p->event()) {
				break;
			}
			sc_lv<DATA_WIDTH> io_out = activation[act].read();
			io_p->write(io_out);
			act++;
			if (act >= NUM_OF_OUTPUT_NEURONS) act = 0;
		}
		io_p->write(io_high_impedance);
	}
}

void RRAM_MNIST::read_neuron_status(void)
{
	while(true)
	{
		wait();
		while (true)
		{
			wait(clk_p->negedge_event() | cs_p->default_event());
			if (cs_p->event()) {
				break;
			}
			sc_lv<DATA_WIDTH> io_out = status.read();
			io_p->write(io_out);
		}
		io_p->write(io_high_impedance);
	}
}

void RRAM_MNIST::inference(void)
{
	sc_uint<DATA_WIDTH> pixel;
	
	while(true)
	{
		wait();
		status_register_1[0] = 1;
		begin_read_weights.notify();
		for(int i=0; i < NUM_OF_INPUT_PIXELS; i++)
		{
			wait(clk_p->posedge_event());
			sc_uint<DATA_WIDTH> io_in = io_p->read();
			pixel = io_in;
			pixel_fifo.write(pixel);
		}
	}
}

void RRAM_MNIST::read_weights(void)
{
	while(true)
	{
		wait();
		
		// Reset Neuron
		wait(clk_p->posedge_event());
		reset.write(false);
		while(true)
		{
			wait(clk_p->posedge_event());
			sc_uint<DATA_WIDTH> status_uint = status.read();
			if (!status_uint[STS_RESET]) {
				break;
			}
		}
		reset.write(true);
		
		// Start MAC
		wait(clk_p->posedge_event());
		enable.write(false);
		wait(clk_p->posedge_event());
		
		int row = 0;
		int col = 0;
		int beg = 0;
		
		int num_bit_pixel = NUM_OF_OUTPUT_NEURONS * DATA_WIDTH;
		
		for(int i=0; i<NUM_OF_INPUT_PIXELS; i++)
		{
			wait(READ_ARRAY_LATENCY, SC_NS);
			
			for(int j=0; j<NUM_OF_OUTPUT_NEURONS; j++)
			{
				sc_uint<DATA_WIDTH> weight_tmp;
				for(int k=0; k<DATA_WIDTH; k++)
				{
					weight_tmp[DATA_WIDTH-k-1] = data[row][col];
					col++;
				}
				page_buffer[j].write(weight_tmp);
			}
			
			row++;
			col = beg;
			
			wait(clk_p->posedge_event());
			valid.write(true);
			wait(clk_p->posedge_event());
			valid.write(false);
			
			if (row >= NUM_OF_ROWS)
			{
				row = 0;
				beg += num_bit_pixel;
				col = beg;
			}
		}
		
		wait(clk_p->posedge_event());
		status_register_1[0] = 0;
		status_register_1[1] = 0;
		enable.write(true);
	}
}
