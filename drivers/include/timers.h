#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <stdbool.h>
#include <stdint.h>

#include "driver_defines.h"


//*****************************************************************************
// Configure a general purpose timer to be a 32-bit timer.  
//
// Paramters
//  base_address          The base address of a general purpose timer
//
//  mode                  bit mask for Periodic, One-Shot, or Capture
//
//  count_up              When true, the timer counts up.  When false, it counts
//                        down
//
//  enable_interrupts     When set to true, the timer generates and interrupt
//                        when the timer expires.  When set to false, the timer
//                        does not generate interrupts.
//
//The function returns true if the base_addr is a valid general purpose timer
//*****************************************************************************
bool gp_timer_config_32(uint32_t base_addr, uint32_t mode, bool count_up, bool enable_interrupts);

//*****************************************************************************
//	Configures a general purpose timer to be two 16-bit periodic timers.  
//
//	Paramters:
//	base_address					The base address of a general purpose timer
//
//	prescaleA							The prescale value for Timer A
//
//	valueA								The interval load value for Timer A
//
//	prescaleB							The prescale value for Timer B
//
//	valueB								The interval load value for Timer B
//
//	The function returns true if the base_addr is a valid general purpose timer
//*****************************************************************************
bool gp_timer_config_16_periodic(uint32_t base_addr, uint8_t prescaleA, uint16_t loadA, uint8_t prescaleB, uint16_t loadB);

//*****************************************************************************
// Waits for 'ticks' number of clock cycles and then returns.
//
//The function returns true if the base_addr is a valid general purpose timer
//*****************************************************************************
bool gp_timer_wait(uint32_t base_addr, uint32_t ticks);

//*****************************************************************************
// Gets a random number by magic
//
// Parameters:
//			base_addr				base address of a timer
//
// Returns:
// 			val							a random uint32_t
//*****************************************************************************
uint32_t get_rand_num
(
	uint32_t base_addr
);
#endif
