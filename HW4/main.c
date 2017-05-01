// Copyright (c) 2015-16, Joe Krachey
// All rights reserved.
//
// Redistribution and use in source or binary form, with or without modification, 
// are permitted provided that the following conditions are met:
//
// 1. Redistributions in source form must reproduce the above copyright 
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

#include "main.h"
#include "timers.h"
#include "ps2.h"
#include "launchpad_io.h"
#include "lcd.h"
#include "port_expander.h"
#include "i2c.h"
#include "eeprom.h"
#include "galaga.h"
#include "ft6x06.h"
#include "port_expander.h"

// Game states used in main program loop
typedef enum {
	MAIN_MENU,
	HIGH_SCORE,
	MAIN_GAME,
	GAME_OVER,
	NEW_RECORD,
	PAUSE
} gameState_t;

// Var to keep track of game state
gameState_t state;

// True if entering a new state
bool new_state = true;

char group[] = "Group27";
char individual_1[] = "Justin Essert";
char individual_2[] = "James Mai";

// Global Vars ================================================================
static ADC0_Type* myADC = ((ADC0_Type *)PS2_ADC_BASE);
					

// Booleans to allow the handlers to communicate with main()
volatile bool interrupt_timerA = false;
volatile bool interrupt_timerB = false;
volatile bool interrupt_adc0ss2 = false;



// Game Variables
uint8_t placeholder_image[20] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

//*****************************************************************************
// Function Name: initialize_hardware
//*****************************************************************************
// Summary: This function initializes the PS2, LCD, Timers, Switches
//					Touchscreen, and Port expander, 
//
//*****************************************************************************
void initialize_hardware(void)
{
  // INITIALIZE SERIAL DEBUG ==================================================
	initialize_serial_debug();

	
	// INITIALIZE PS2 ===========================================================
	ps2_initialize_HW3();
	
	
	// INITIALIZE LCD ===========================================================
	// Configure the LCD to connect to GPIOB & GPIOC
	lcd_config_gpio();
	
	
	// Configure the LCD Controller
	lcd_config_screen();
	
	
	// Clear the screen to all black
	lcd_clear_screen(LCD_COLOR_BLACK);
	
	
	// INITIALIZE LEDS AND SWITCH BUTTONS =======================================
	lp_io_init();
	port_expander_init();
	
	
	// INITIALIZE TIMER =========================================================
	// Configure Timer0 to be two 16-bit periodic timers
	gp_timer_config_16_periodic(TIMER0_BASE, TIMER_TAPR_PRESCALE, TIMER_TAILR_10MS_W_PRESCALE, TIMER_TBPR_PRESCALE, TIMER_TBILR_30MS_W_PRESCALE);
	
	// Enable Timer A and B
	TIMER0->CTL |= TIMER_CTL_TAEN | TIMER_CTL_TBEN;
	
	
	// INITIALIZE TOUCH SCREEN
	DisableInterrupts();
  ft6x06_init();
  EnableInterrupts();
	
	// INITIALIZE PUSH BUTTONS
	
	port_expander_init();
	
}

//*****************************************************************************
//*****************************************************************************
// INTERRUPT FLOW FUNCTIONS
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
// Function Name: sw1_debounce
//*****************************************************************************
//	Summary: This function tests if SW1 has been pressed for 6 cycles
//
//	Returns:
//
//	true				SW1 lifted after pressed for 6 cycles
//	false				All other states
//
//*****************************************************************************
__INLINE static bool sw1_debounce()
{
  // Counter to determine the length of time that SW1 has been pressed
	static volatile long long cycle = 0;
  
	// If SW1 is pressed, increment cycle, return false
  if(!lp_io_read_pin(SW1_BIT)){
		cycle++;
		return false;
	}
	// If the button is lifted and has been pressed for > 8 cycles, return true
  else if(cycle > 6){
		cycle = 0;
		return true;
	}
	// If SW1 is not pressed, set cycle to 0
	else {
		cycle = 0;
		return false;
	}
	
	
}
//*****************************************************************************
// Function Name: pbRight_debounce
//*****************************************************************************
//	Summary: This function tests if PB right has been pressed for 6 cycles
//
//	Returns:
//
//	true				Pushbutton (R) has been released after being pressed for 6 cycles
//	false				all other states
//
//*****************************************************************************
__INLINE static bool pbRight_debounce()
{
  // Counter to determine the length of time that PBR has been pressed
	static int cycle = 0;
	uint8_t data;
	
	// read data
  pexp_read_buttons(I2C1_BASE, &data);
	
	// If right PB is pressed, increment cycle and return false
  if(data & PEXP_BUTTON_RIGHT){
		cycle = (cycle+1);
		return false;
	}
	// If right PB is not pressed, but cycle is greater than 6, return true
	else if ( cycle > 6 ){
		cycle = 0;
		return true;
	}
	// reset cycleelse return false
	else {
		cycle = 0;
		return false;
	}
}
//*****************************************************************************
// Function Name: pbDown_debounce
//*****************************************************************************
//	Summary: This function tests if PB down has been pressed for 6 cycles
//
//	Returns:
//
//	true				Pushbutton (D) has been released after being pressed for 6 cycles
//	false				all other states
//
//*****************************************************************************

__INLINE static bool pbDown_debounce()
{
  // Counter to determine the length of time that PBD has been pressed
	static int cycle = 0;
	uint8_t data;
	
	// read data
  pexp_read_buttons(I2C1_BASE, &data);
	
	// If down PB is pressed, increment cycle and return false
  if(data & PEXP_BUTTON_DOWN){
		cycle = (cycle+1);
		return false;
	}
	// If down PB is not pressed, but cycle is greater than 6, return true
	else if ( cycle > 6 ){
		cycle = 0;
		return true;
	}
	// else reset cycle and return false
	else {
		cycle = 0;
		return false;
	}
}

//*****************************************************************************
//*****************************************************************************
// TIMER ISR Handler
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
// TIMER0A Interrupt Service handler
//*****************************************************************************
void TIMER0A_Handler(void)
{
	// SIGNAL MAIN() THAT THE INTERRUPT OCCURRED ================================
	interrupt_timerA = true;
	
	// CLEAR THE TIMER A INTERRUPT ==============================================
	TIMER0->ICR |= TIMER_ICR_TATOCINT;
}


//*****************************************************************************
// TIMER0B Interrupt Service handler
//*****************************************************************************
void TIMER0B_Handler(void)
{
	// SIGNAL MAIN() THAT THE INTERRUPT OCCURRED ================================
	interrupt_timerB = true;

	// CLEAR THE TIMER B INTERRUPT ==============================================
	TIMER0->ICR |= TIMER_ICR_TBTOCINT;
}


//*****************************************************************************
// ADC0 Sample Sequencer 2 Interrupt Service handler
//*****************************************************************************
void ADC0SS2_Handler(void)
{
	// SIGNAL MAIN() THAT THE INTERRUPT OCCURRED ================================
	interrupt_adc0ss2 = true;
	
	// CLEAR THE ADC0SS2 INTERRUPT ==============================================
	myADC->ISC |= ADC_ISC_IN2;
  return;	
	
}


//*****************************************************************************
//*****************************************************************************
// MAIN FUNCTION
//*****************************************************************************
//*****************************************************************************
int main(void)
{
  int counterA = 0;		// Counter for TimerA's Interrupt Handler
	int counterB = 0;		// Counter for TimerB's Interrupt Handler
	uint32_t x_value;
	uint32_t y_value;
	uint8_t data;
	i2c_status_t td_status;
	uint16_t x = 0;
	uint16_t y = 0;
	gameState_t state;
	uint16_t addr;
	int i,j;
	bool toggle;
	char prev[4] = "   ";
	
	int cursor_pos = 0;  // cursor position
	int selected_char = 0; // A is 0, B is 1...
	uint8_t pbData = 0;
	char initial[4];
	bool foo = false;
	
	
	// INITIALIZE FUNCTIONS =====================================================
	initialize_hardware();
	
	// DISPLAY ON CONSOLE =======================================================
	put_string("\n\r");
  put_string("************************************\n\r");
  put_string("ECE353 - Fall 2016 HW3\n\r  ");
  put_string(group);
  put_string("\n\r     Name:");
  put_string(individual_1);
  put_string("\n\r     Name:");
  put_string(individual_2);
  put_string("\n\r");  
  put_string("************************************\n\r");
	

	/* USE TO WIPE EEPROM
	addr = ADDR_START;
	for ( i = 0; i < 7*5; i++){
		eeprom_byte_write(I2C1_BASE,addr, 0);
		addr++;
	}
	*/

	// Set state to Main Menu and start the while loop
	state=MAIN_MENU;
	new_state = true;
  while(1)
	{
		if(new_state){
			new_state=false;
			
			lcd_clear_screen(LCD_COLOR_BLACK);
			
			// If state is main menu, print the menu
			if(state==MAIN_MENU) print_main_menu();
			// If state is high score, print the high score screen
			else if(state==HIGH_SCORE) print_high_scores();
			// If state is main game the initialize the game
			else if(state==MAIN_GAME) game_init();
			// If state is game over the initialize the game over menu
			else if(state==GAME_OVER) print_game_over();
			// If state is new record, print the new record screen
			else if(state==NEW_RECORD) print_new_record();
			// If state is new pause, print the pause screen
			else if(state==PAUSE) print_pause();
		}
		if(interrupt_timerA){
			interrupt_timerA = false;
			counterA = ((counterA+1)%TIMER_A_CYCLES);
			
			// Check for Touchscreen press
			td_status = ft6x06_read_td_status();
			// If Touchscreen event occured, read the Y value
			if( td_status >0 ){
				y = ft6x06_read_y();

				if(state==MAIN_MENU){
					// If the Y is the START GAME button then start the game
					if ((y >120)&& (y < 170)){
						state = MAIN_GAME;
						new_state = true;
					}
					// If the Y is the HIGH SCORE button then got to high scores
					else if ((y > 70) && ( y < 120 )){
						state = HIGH_SCORE;
						new_state = true;
					}
				} 
				// If the Y is the MAIN_MENU button then got to the main menu
				else if (state==HIGH_SCORE){
					if ( (y < 60) && (y > 10) ){
						state = MAIN_MENU;
						new_state = true;
					}		
				}
				// If the Y is the high score button then got to high scores
				else if(state == GAME_OVER){
          if ( (y < 120) && (y > 10) ){
				    state = HIGH_SCORE;
            new_state = true;
				    for(i = 0; i < NUM_HIGH_SCORES; i++){
					    if(player_score > high_scores[i]){
                print_new_record();
                cursor_pos = 0;
                selected_char = 0;
                state = NEW_RECORD;
              }
				    }		
			    }
				}	else if(state == PAUSE){
					// If the Y is the MAIN MENU button then go to MAIN MENU
					if ((y >120)&& (y < 170)){
						state = MAIN_MENU;
						new_state = true;
					}
					// If the Y is the RESUME button then return to MAIN GAME
					else if ((y > 70) && ( y < 120 )){
						lcd_clear_screen(LCD_COLOR_BLACK);
						state = MAIN_GAME;
					}
				}
			
 
			}
			// if SW1 is pressed whiled paused, resume MAIN_GAME
			if (state==PAUSE  && sw1_debounce()  ){
					lcd_clear_screen(LCD_COLOR_BLACK);
					state = MAIN_GAME;
			}
			if (state==MAIN_GAME && !new_state){
				if(sw1_debounce()){
					state = PAUSE;
					new_state  = true;
					put_string("dddd");
				}
				// Update bullet positions
				update_bullets();
				
				//If increment of 5 read ADC
				if(counterA%5==0) {
					// Initialize ADC Read
					myADC->PSSI = ADC_PSSI_SS2;
					//update_enemies();
					if(update_enemies()) {
						state = MAIN_MENU;
						new_state = true;
					}
					update_LCD();
				}
				// If new interrupt count read PEXP buttons
				if(counterA==0){
					if(pexp_read_buttons(I2C1_BASE, &data) != I2C_OK){
						put_string("error reading port expander");
						continue;
					}else if(data & PEXP_BUTTON_DOWN) fire_bullet(true, 0, 0);
				}	
			}
	}
		//*************************************************************************
		// TIMER B INTERRUPT HANDLING
		//*************************************************************************
		if(interrupt_timerB)
		{					
			// CLEAR INTERRUPT INDICATOR ============================================
			interrupt_timerB = false;
			
			// Increment the counter & reset to zero if it reached TIMER_B_CYCLES
			counterB = ((counterB+1)%TIMER_B_CYCLES);
				
			if(state == MAIN_GAME){
				if(counterB==0) {
					update_LCD();
				}	
			} else if (state == NEW_RECORD){
				myADC->PSSI = ADC_PSSI_SS2;
			}

			
		}
		if(state == NEW_RECORD){			
			// If right is pressed
			if (pbRight_debounce()){
				// If cursor is at position 2, submit score and enter HIGH_SCORE
				if( cursor_pos ==2 ){
					push_high_scores(initial);
					pull_high_scores();
					print_high_scores();
					state = HIGH_SCORE;
					new_state = true;
					continue;
				// Otherwise set selected character and increment cursor
				} else {
					initial[cursor_pos] = (char)selected_char + 'A';
					selected_char = 0;
					cursor_pos++;
				}
			
			// If down is pressed and cursor is not at position 0
			} else if ( pbDown_debounce() && (cursor_pos >0) ){
				// Delete curret value and decrement cursor position
				initial[cursor_pos] = ' ';
				cursor_pos--;
				selected_char = ((int) initial[cursor_pos] -'A');
			}
			// Print entered characters

				lcd_print_stringXY(initial, 5,11, GALAGA_COLOR_3, LCD_COLOR_BLACK );		
		}
		//*************************************************************************
		// ADC0SS2 INTERRUPT HANDLING
		//*************************************************************************
		if(interrupt_adc0ss2)
		{	
			// CLEAR INTERRUPT INDICATOR ============================================
			interrupt_adc0ss2 = false;
			
			y_value = (uint32_t)(myADC->SSFIFO2 & 0xFFF);
			x_value = (uint32_t)(myADC->SSFIFO2 & 0xFFF);
			
			if (state==MAIN_GAME){
				if(x_value >= 0xBFD)
					update_player(true);
				else if (x_value <= 0x3FF)
					update_player(false);
			} else if(state == NEW_RECORD){
          // Check if the y value is >= 75% increment currently selected character
					if(y_value >= 0xBFD)	selected_char = (selected_char+1)%26;
					// if y value is <=25%, decrement instead
					else if(y_value <= 0x3FF) selected_char = selected_char-1;
					if (selected_char < 0)
						selected_char = 25;
					// Set new character in cursor position
					if(cursor_pos>0) prev[cursor_pos-1] = initial[cursor_pos-1];
					initial[cursor_pos] = (char)selected_char+'A';	
      }
		}
	}
}

