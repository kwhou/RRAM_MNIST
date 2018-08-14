#ifndef _CONFIG_H_
#define _CONFIG_H_

// RRAM Structure
#define INS_WIDTH 8
#define ADDR_WIDTH 16
#define DATA_WIDTH 32
#define STATUS_REG_WIDTH 8
#define NUM_OF_ROWS 256
#define NUM_OF_COLS 2048
 
// RRAM Instructions
#define INS_READ "0b000000011"
#define INS_WRITE_ENABLE "0b000000110"
#define INS_PAGE_WRITE "0b000001010"
#define INS_PAGE_ERASE "0b011011011"
#define INS_READ_STATUS_REG "0b000000101"

// RRAM Timings
#define READ_ARRAY_LATENCY 20 //ns
#define PAGE_WRITE_LATENCY 1000000 //ns
#define PAGE_ERASE_LATENCY 1000000 //ns

// RRAM Classifier
#define FIFO_SIZE 8

// RRAM Classifier Structure
#define NUM_OF_INPUT_PIXELS 784
#define NUM_OF_OUTPUT_NEURONS 10

// RRAM Classifier Instructions
#define INS_WEIGHT_WRITE "0b000010101"
#define INS_INFERENCE "0b000010100"
#define INS_READ_NEURON_VALUE "0b000010110"
#define INS_READ_NEURON_STATUS "0b000010111"

// NEURON Status Index
#define STS_RESET 0
#define STS_MAC 1
#define STS_CLASS 2

// NEURON Timings
#define RESET_LATENCY 20 //ns

#endif
