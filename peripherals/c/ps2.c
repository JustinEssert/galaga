// Copyright (c) 2014-16, Joe Krachey
// All rights reserved.
//
// Redistribution and use in binary form, with or without modification, 
// are permitted provided that the following conditions are met:
//
// 1. Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in 
//    the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ps2.h"


/*******************************************************************************
* Function Name: initialize_adc_gpio_pins
********************************************************************************
* Used to initialize the PS2 joystick GPIO pins for both the analog inputs
*
* Configuration Info
*		Fill out relevant information in ps2.h.  ps2.h defines 
*		how various peripherals are physically connected to the board.
*******************************************************************************/
static void initialize_adc_gpio_pins(void)
{
	uint32_t baseAddr = GPIOE_BASE;
	uint8_t pins = 1<<PS2_X_BIT_NUM | 1<<PS2_Y_BIT_NUM;
	
	gpio_enable_port(GPIOE_BASE);
	gpio_config_enable_input(baseAddr, pins);
	gpio_config_analog_enable(baseAddr, pins);
	gpio_config_alternate_function(baseAddr, pins);
	
}

/*******************************************************************************
* Function Name: ps2_initialize
********************************************************************************
* Initializes the GPIO pins connected to the PS2 Joystick.  It also configures
* ADC0 to use Sample Sequencer #3 to convert a programmable channel number.
*******************************************************************************/
void ps2_initialize(void)
{
	initialize_adc_gpio_pins();
	initialize_adc(PS2_ADC_BASE);
}

/*******************************************************************************
* Function Name: ps2_get_x
********************************************************************************
*Returns the most current reading of the X direction  Only the lower 12-bits
* contain data.
********************************************************************************/
uint16_t ps2_get_x(void)
{
  uint32_t adc_base = PS2_ADC_BASE;
	uint8_t channel = PS2_X_ADC_CHANNEL;
  return get_adc_value(adc_base, channel);
}

/*******************************************************************************
* Function Name: ps2_get_y
********************************************************************************
* Returns the most current reading of the Y direction.  Only the lower 12-bits
*  contain data.
********************************************************************************/
uint16_t ps2_get_y(void)
{
  uint32_t adc_base = PS2_ADC_BASE;
	uint8_t channel = PS2_Y_ADC_CHANNEL;
  return get_adc_value(adc_base, channel);
}

