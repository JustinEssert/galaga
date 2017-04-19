#include "timers.h"


//*****************************************************************************
// Verifies that the base address is a valid GPIO base address
//*****************************************************************************
static bool verify_base_addr(uint32_t base_addr)
{
   switch( base_addr )
   {
     case TIMER0_BASE:
     case TIMER1_BASE:
     case TIMER2_BASE:
     case TIMER3_BASE:
     case TIMER4_BASE:
     case TIMER5_BASE:
     {
       return true;
     }
     default:
     {
       return false;
     }
   }
}

//*****************************************************************************
// Returns the RCGC and PR masks for a given TIMER base address
//*****************************************************************************
static bool get_clock_masks(uint32_t base_addr, uint32_t *timer_rcgc_mask, uint32_t *timer_pr_mask)
{
  // Set the timer_rcgc_mask and timer_pr_mask using the appropriate
  // #defines in ../include/sysctrl.h
  switch(base_addr)
  {
    case TIMER0_BASE:
    {
      *timer_rcgc_mask = SYSCTL_RCGCTIMER_R0;
      *timer_pr_mask = SYSCTL_PRTIMER_R0;
      break;
    }
    case TIMER1_BASE:
    {
      *timer_rcgc_mask = SYSCTL_RCGCTIMER_R1;
      *timer_pr_mask = SYSCTL_PRTIMER_R1;
      break;
    }
    case TIMER2_BASE:
    {
      *timer_rcgc_mask = SYSCTL_RCGCTIMER_R2;
      *timer_pr_mask = SYSCTL_PRTIMER_R2;
      break;
    }
    case TIMER3_BASE:
    {
      *timer_rcgc_mask = SYSCTL_RCGCTIMER_R3;
      *timer_pr_mask = SYSCTL_PRTIMER_R3;
      break;
    }
    case TIMER4_BASE:
    {
      *timer_rcgc_mask = SYSCTL_RCGCTIMER_R4;
      *timer_pr_mask = SYSCTL_PRTIMER_R4;
      break;
    }
    case TIMER5_BASE:
    {
      *timer_rcgc_mask = SYSCTL_RCGCTIMER_R5;
      *timer_pr_mask = SYSCTL_PRTIMER_R5;
      break;
    }
    default:
    {
      return false;
    }
  }
  return true;
}


//*****************************************************************************
// Waits for 'ticks' number of clock cycles and then returns.
//
//The function returns true if the base_addr is a valid general purpose timer
//*****************************************************************************
bool gp_timer_wait(uint32_t base_addr, uint32_t ticks)
{
  TIMER0_Type *gp_timer;
  
  // Verify the base address.
  if ( ! verify_base_addr(base_addr) )
  {
    return false;
  }

  // Type cast the base address to a TIMER0_Type struct
  gp_timer = (TIMER0_Type *)base_addr;

  //*********************    
  // ADD CODE
  //*********************
  // Checkpoint 6
	gp_timer->CTL &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);
	
	// Checkpoint 7
	gp_timer->TAILR = ticks;
	
	// Checkpoint 8
	gp_timer->ICR |= TIMER_ICR_TATOCINT;
	
	// Checkpoint 9
  gp_timer->CTL |= TIMER_CTL_TAEN;
	
	// Checkpoint 10
	while( !(gp_timer->RIS & TIMER_RIS_TATORIS) );
	
	
	return true;
}


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
bool gp_timer_config_32(uint32_t base_addr, uint32_t mode, bool count_up, bool enable_interrupts)
{
  uint32_t timer_rcgc_mask;
  uint32_t timer_pr_mask;
  TIMER0_Type *gp_timer;
  
  // Verify the base address.
  if ( ! verify_base_addr(base_addr) )
  {
    return false;
  }
  
  // get the correct RCGC and PR masks for the base address
  get_clock_masks(base_addr, &timer_rcgc_mask, &timer_pr_mask);
  
  // Turn on the clock for the timer
  SYSCTL->RCGCTIMER |= timer_rcgc_mask;

  // Wait for the timer to turn on
  while( (SYSCTL->PRTIMER & timer_pr_mask) == 0) {};
  
  // Type cast the base address to a TIMER0_Type struct
  gp_timer = (TIMER0_Type *)base_addr;
    
  //*********************    
  // ADD CODE
  //*********************
	// Checkpoint 1
	gp_timer->CTL &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);
	
	// Checkpoint 2
	gp_timer->CFG = TIMER_CFG_32_BIT_TIMER;
		
	// Checkpoint 3
	gp_timer->TAMR &= ~TIMER_TAMR_TAMR_M;
	gp_timer->TAMR |= TIMER_TAMR_TAMR_M & mode;
	
	// Checkpoint 4
	
	if (count_up) gp_timer->TAMR |= TIMER_TAMR_TACDIR;
	else gp_timer->TAMR &= ~TIMER_TAMR_TACDIR;
		
	// Checkpoint 5
	if (enable_interrupts) gp_timer->IMR |= TIMER_IMR_TATOIM;
	else gp_timer->IMR &= ~TIMER_IMR_TATOIM;

  return true;  
}
//*****************************************************************************
// Configure a general purpose timer to be two 16-bit timers.  
//
// Paramters
//  base_address          The base address of a general purpose timer
//
//  mode A/B              bit mask for Periodic, One-Shot, or Capture
//
//  count_up A/B          When true, the timer counts up.  When false, it counts
//                        down
//
//  enable_interrupts A/B When set to true, the timer generates and interrupt
//                        when the timer expires.  When set to false, the timer
//                        does not generate interrupts.
//
//The function returns true if the base_addr is a valid general purpose timer
//*****************************************************************************
bool gp_timer_config_16_HW3()
{
  
	uint32_t timer_rcgc_mask;
  uint32_t timer_pr_mask;
	uint32_t base_addr = TIMER0_BASE;
  TIMER0_Type *gp_timer;
  
  /* Verify not needed when hardcoded Verify the base address.
  if ( ! verify_base_addr(base_addr) )
  {
    return false;
  }
  */
  // get the correct RCGC and PR masks for the base address
  get_clock_masks(base_addr, &timer_rcgc_mask, &timer_pr_mask);
  
  // Turn on the clock for the timer
  SYSCTL->RCGCTIMER |= timer_rcgc_mask;

  // Wait for the timer to turn on
  while( (SYSCTL->PRTIMER & timer_pr_mask) == 0) {
	};
  
  // Type cast the base address to a TIMER0_Type struct
  gp_timer = (TIMER0_Type *)base_addr;
    
  //*********************    
  // ADD CODE
  //*********************
	// Disable timerA and timerB
	gp_timer->CTL &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);
	
	// Set mode to 16-bit mode
	gp_timer->CFG = TIMER_CFG_16_BIT;
		
	// TIMER CONFIG
	gp_timer->TAMR &= ~TIMER_TAMR_TAMR_M;
	gp_timer->TAMR |= TIMER_TAMR_TAMR_M & TIMER_TAMR_TAMR_PERIOD;
	gp_timer->TBMR &= ~TIMER_TBMR_TBMR_M;
	gp_timer->TBMR |= TIMER_TBMR_TBMR_M & TIMER_TBMR_TBMR_PERIOD;
	
	gp_timer->TAPR = 1000;
	gp_timer->TBPR = 1000;
	
	gp_timer->TAILR = 500;
	gp_timer->TBILR = 1500;
	
	gp_timer->ICR |= TIMER_ICR_TATOCINT | TIMER_ICR_TBTOCINT;
	gp_timer->CTL |= TIMER_CTL_TAEN | TIMER_CTL_TBEN;
	
	// Set the priority to 1
	NVIC_SetPriority(TIMER0A_IRQn, 1);
	// Enable the NVIC for the watchdog timer
	NVIC_EnableIRQ(TIMER0A_IRQn);
	
	// Set the priority to 1
	NVIC_SetPriority(TIMER0B_IRQn, 1);
	// Enable the NVIC for the watchdog timer
	NVIC_EnableIRQ(TIMER0B_IRQn);
	
  return true;  
}
