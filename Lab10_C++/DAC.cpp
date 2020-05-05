// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "DAC.h"

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	volatile uint8_t delay = 0;
	
	//Using the PortB PB3-PB0 as the output
	SYSCTL_RCGCGPIO_R |= 0x02;
	delay = 0;
	GPIO_PORTB_DIR_R |= 0x0F;
	GPIO_PORTB_DEN_R |= 0x0F;
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none

void DAC_Out(uint8_t data){
	GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & 0xF0) + data;
}

