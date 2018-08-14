#include "Neuron.h"

void Neuron::reset(void) // asynchronous reset
{
	while(true)
	{
		wait(reset_p->negedge_event());
		
		wait(clk_p->posedge_event());
		status = status_p.read();
		status[STS_RESET] = 1;
		status_p.write(status);
		
		wait(RESET_LATENCY, SC_NS);
		for(int j=0; j<NUM_OF_OUTPUT_NEURONS; j++) {
			activation[j] = 0;
		}
		
		wait(clk_p->posedge_event());
		status = status_p.read();
		status.range(DATA_WIDTH-1, STS_CLASS) = 0;
		status[STS_RESET] = 0;
		status_p.write(status);
		
		while(!reset_p.read())
			wait(reset_p->default_event());
	}
}

void Neuron::MAC(void)
{
	sc_uint<DATA_WIDTH> pixel;
	
	while(true)
	{
		wait(enable_p->negedge_event());
		
		wait(clk_p->posedge_event());
		status = status_p.read();
		status[STS_MAC] = 1;
		status_p.write(status);
		
		for(int i=0; i<NUM_OF_INPUT_PIXELS; i++)
		{
			while(true)
			{
				wait(clk_p->posedge_event());
				wait(SC_ZERO_TIME);
				if (valid_p->read()) {
					break;
				}
			}
			
			if (pixel_fifo_p->num_available())
			{
				pixel = pixel_fifo_p->read();
			}
			else
			{
				wait(pixel_fifo_p->data_written_event());
				pixel = pixel_fifo_p->read();
			}
			
			long pixel_long = pixel;
			float *pixel_pointer = (float *)&pixel_long;
			float pixel_float = *pixel_pointer;
			
			for(int j=0; j<NUM_OF_OUTPUT_NEURONS; j++)
			{
				long weight_long = weight_p[j].read();
				float *weight_pointer = (float *)&weight_long;
				float weight_float = *weight_pointer;
				
				activation[j] += weight_float * pixel_float;
			}
		}
		
		int max_index = 0;
		float max_value = activation[0];
		for(int j=1; j<NUM_OF_OUTPUT_NEURONS; j++)
		{
			if(activation[j] > max_value)
			{
				max_value = activation[j];
				max_index = j;
			}
		}
		
		wait(clk_p->posedge_event());
		for(int j=0; j<NUM_OF_OUTPUT_NEURONS; j++)
		{
			long activation_long = *((long *)&activation[j]);
			sc_uint<DATA_WIDTH> activation_sc_uint = activation_long;
			activation_p[j].write(activation_sc_uint);
		}
		
		status.range(DATA_WIDTH-1, STS_CLASS) = max_index;
		status[STS_MAC] = 0;
		status_p.write(status);
	}
}
