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
#include "i2c.h"
#include "eeprom.h"
#include "galaga.h"
#include "ft6x06.h"
#include "port_expander.h"

typedef enum {
	MAIN_MENU,
	HIGH_SCORE,
	MAIN_GAME,
	GAME_OVER,
	NEW_RECORD
} gameState_t;

char group[] = "Group27";
char individual_1[] = "Justin Essert";
char individual_2[] = "James Mai";

// Global Vars ================================================================
static ADC0_Type* myADC = ((ADC0_Type *)PS2_ADC_BASE);
					

// Booleans to allow the handlers to communicate with main()
volatile bool interrupt_timerA = false;
volatile bool interrupt_timerB = false;
volatile bool interrupt_adc0ss2 = false;

uint16_t *adc_val_X;
uint16_t *adc_val_Y;

// Game Variables
uint8_t placeholder_image[20] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

//*****************************************************************************
// Function Name: initialize_hardware
//*****************************************************************************
// Summary: This function initializes the PS2, LCD, Timers, and Switches
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
// Function Name: ship_move
//*****************************************************************************
//	Summary: Either draws or erases active active pixel
// 
//	Parameters:
//
//	draw: 				Pass in true to draw or false to erase
//
//*****************************************************************************
void ship_move (uint32_t x_value, uint32_t y_value)
{
	static int xPos = 120;					// x coordinate of cursor
	static int yPos = 160;					// y coordinate of cursor
	uint32_t move = 0;
	
	// CHECK IF THE PS2 IS PAST ANY OF OUR THRESHOLDS ===========================
	// Check if the x value is >= 75% or <= 25%
	if(x_value >= 0xBFD)				move |= X_GREATER_THAN_3_4;
	else if(x_value<= 0x3FF)		move |= X_LESS_THAN_1_4;
	
	// Check if the y value is >= 75% or <= 25%
	if(y_value >= 0xBFD)				move |= Y_GREATER_THAN_3_4;
	else if(y_value <= 0x3FF)		move |= Y_LESS_THAN_1_4;
	
	// CHECK THE MODE ===========================================================
	// Clear pixel current position
	lcd_draw_image(
		xPos-6, 13, yPos-4, 9, placeholder_image, 
		LCD_COLOR_BLACK, LCD_COLOR_BLACK
  );
	
			
		
		// UPDATE THE X AND Y POSITIONS ===========================================
		
		// ** NOTE: THE X POSITION OF THE PS2 RELATES TO THE Y POSITION OF THE LCD **
		
		// If X (of the PS2) is greater than 75% of its max value, increment Y (of LCD).
		if(move&X_GREATER_THAN_3_4) 	yPos++;
		// If X (of the PS2) is less than 25% of its max value, decrement Y (of LCD).
		else if(move&X_LESS_THAN_1_4) yPos--;
		
		// If Y (of the PS2) is greater than 75% of its max value, increment X (of LCD).
		if(move&Y_GREATER_THAN_3_4) 	xPos++;
		// If Y (of the PS2) is less than 25% of its max value, decrement X (of LCD).
		else if(move&Y_LESS_THAN_1_4) xPos--;

		// Check bound of Y and hold position if it goes off of the screen
		if(yPos >= 316) yPos = 315;
		else if(yPos <=4 ) yPos = 5;

		// Check bound of Y and wrap around if it goes off of the screen
		if(xPos >=  234) xPos = 233;
		else if(xPos <= 5) xPos = 6;
		
	// Draw ship at new position 
	lcd_draw_image(
		xPos-6, 13, yPos-4, 9, placeholder_image, 
		LCD_COLOR_WHITE, LCD_COLOR_BLACK
  );
}





//*****************************************************************************
//*****************************************************************************
// INTERRUPT FLOW FUNCTIONS
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
// Function Name: sw1_debounce
//*****************************************************************************
//	Summary: This function tests if SW1 has been pressed for 60ms
//
//	Returns:
//
//	true				SW1 has been pressed for 60ms
//	false				SW1 has been pressed for less than 60ms or is not pressed
//
//*****************************************************************************
__INLINE static bool sw1_debounce()
{
  // Counter to determine the length of time that SW1 has been pressed
	static int cycle = 0;
  
	// If SW1 is pressed, increment cycle
  if(lp_io_read_pin(SW1_BIT)) cycle = (cycle+1);
	// If SW1 is not pressed, set cycle to 0
	else cycle = 0;
	
	// If the button has been pressed for >= 60ms, return true
  return (cycle == 6);
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
	i2c_status_t td_status;
	uint16_t x = 0;
	uint16_t y = 0;
	gameState_t state = NEW_RECORD;
	uint16_t addr;
	int i,j;
	
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
	
	
	
	
	for(addr = ADDR_START; addr <(ADDR_START+3*5); addr++)
  {
      printf("Writing %i\n\r",0x0);
      eeprom_byte_write(I2C1_BASE,addr, 0x0);
  }
	
	

  high_scores_test();
	lcd_clear_screen(LCD_COLOR_BLACK);
  while(1)
	{
		if(state == MAIN_MENU){
			if(interrupt_timerA){
				interrupt_timerA = false;
				td_status = ft6x06_read_td_status();
				print_main_menu();
			}
			
			if( td_status >0 )
				y = ft6x06_read_y();
			
			if ((y >120)&& (y < 170)){
				lcd_clear_screen(LCD_COLOR_BLACK);
				game_init();
				state = MAIN_GAME;
			}
			
			else if ((y > 70) && ( y < 120 )){
				lcd_clear_screen(LCD_COLOR_BLACK);
				pull_high_scores();
				state = HIGH_SCORE;
			}
		}
		
		if (state == HIGH_SCORE){
			
			if(interrupt_timerB){
				interrupt_timerB = false;
				td_status = ft6x06_read_td_status();
				
				counterB = ((counterB+1)%TIMER_B_CYCLES);
				if(counterB==0)
					print_high_scores();
			}
			
			if( td_status >0 )
				y = ft6x06_read_y();
			
			if ( (y < 60) && (y > 10) ){
				lcd_clear_screen(LCD_COLOR_BLACK);
				state = MAIN_MENU;
			}			
		}
		
		if(state == MAIN_GAME){
			//*************************************************************************
			// TIMER A INTERRUPT HANDLING
			//*************************************************************************
			if(interrupt_timerA)
			{		
				// CLEAR INTERRUPT INDICATOR ============================================
				interrupt_timerA = false;
				
				// TOGGLE LED ===========================================================
				// Increment the counter & reset to zero if it reached TIMER_A_CYCLES
				counterA = ((counterA+1)%TIMER_A_CYCLES);
		
				// Toggle Blue LED every time the counter resets
				if(counterA==0);
				
				
				// TOGGLE WRITE MODE ====================================================
				if(sw1_debounce()){
				}
			}
			
			//*************************************************************************
			// TIMER B INTERRUPT HANDLING
			//*************************************************************************
			if(interrupt_timerB)
			{					
				// CLEAR INTERRUPT INDICATOR ============================================
				interrupt_timerB = false;
				if(counterB==0) {
					update_enemies();
					update_LCD();
				}
				// TOGGLE LED & INITIALIZE ADC READ =====================================
				// Increment the counter & reset to zero if it reached TIMER_B_CYCLES
				counterB = ((counterB+1)%TIMER_B_CYCLES);
				
				// Initialize ADC Read
				myADC->PSSI = ADC_PSSI_SS2;
				
				if(counterB==0) {
				}
			}

			//*************************************************************************
			// ADC0SS2 INTERRUPT HANDLING
			//*************************************************************************
			if(interrupt_adc0ss2)
			{	
				// CLEAR INTERRUPT INDICATOR ============================================
				interrupt_adc0ss2 = false;

				x_value = (uint32_t)(myADC->SSFIFO2 & 0xFFF);
				y_value = (uint32_t)(myADC->SSFIFO2 & 0xFFF);
				
				ship_move(x_value, y_value);
			}
		}
		
		
		if(state == GAME_OVER){
			
			if(interrupt_timerB){
				interrupt_timerB = false;
				td_status = ft6x06_read_td_status();
				
				counterB = ((counterB+1)%TIMER_B_CYCLES);
				if(counterB==0)
					print_game_over();
			}
			
			if( td_status >0 )
				y = ft6x06_read_y();
			
			if ( (y < 120) && (y > 10) ){
				lcd_clear_screen(LCD_COLOR_BLACK);
				
				state = HIGH_SCORE;
				for(i = 0; i < NUM_HIGH_SCORES; i++){
					if(player_score > high_scores[i])
						print_new_record();
						cursor_pos = 0;
						selected_char = 0;
						state = NEW_RECORD;
				}		
			}			
		}
		
		if(state == NEW_RECORD){
			if(interrupt_timerB){					
				// CLEAR INTERRUPT INDICATOR
				interrupt_timerB = false;
				// Initialize ADC Read
				myADC->PSSI = ADC_PSSI_SS2;
			}
			
			if(interrupt_adc0ss2){	
				// CLEAR INTERRUPT INDICATOR
				interrupt_adc0ss2 = false;
				x_value = (uint32_t)(myADC->SSFIFO2 & 0xFFF);
				y_value = (uint32_t)(myADC->SSFIFO2 & 0xFFF);
				
					// Check if the x value is >= 75% or <= 25%
					// if(x_value >= 0xBFD)				;
					// else if(x_value<= 0x3FF)		;
	
					// Check if the y value is >= 75% or <= 25%
					if(y_value >= 0xBFD)	selected_char = (selected_char+1)%26;
					else if(y_value <= 0x3FF) selected_char = selected_char-1;
					if (selected_char < 0)
					initial[cursor_pos] = (char)(selected_char + 65);				
			}
			
			pexp_button_read( I2C1_BASE, &pbData );
			if ( pbData & 0x8 ){
				if(cursor_pos >=2 ){
					push_high_scores(initial);
					print_high_scores();
					state = HIGH_SCORE;
					continue;
				} else {
					initial[cursor_pos] = (char)(selected_char + 65);
					cursor_pos++;
				}
				
			} else if ( (pbData & 0x2) && (cursor_pos >0) ){
				initial[cursor_pos] = '\0';
				cursor_pos--;
				selected_char = ((int) initial[cursor_pos] )-65;
			}
			
			lcd_print_stringXY(initial, 12,7, GALAGA_COLOR_3, LCD_COLOR_BLACK );			
		}
	}
}

