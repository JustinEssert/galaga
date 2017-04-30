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
#include "galaga_bitmaps.h"
#include "galaga.h"
#include "eeprom.h"

typedef enum direction {
	DIR_U,
	DIR_UR,
	DIR_R,
	DIR_DR,
	DIR_D,
	DIR_DL,
	DIR_L,
	DIR_UL
} direction_t;

typedef enum unitType {
	PLAYER,
	BUTTERFLY,
	BEE,
	GALAGA	
} unitType_t;


typedef struct position{
	uint16_t x;
	uint16_t y;
} position_t;
typedef struct unit {
	direction_t dir;
	unitType_t type;
	short health;
	position_t pos;
	position_t home_pos;
	short formation_index;
} unit_t;

short player_lives = PLAYER_START_LIVES;

uint32_t high_score = 0, player_score=0;

uint32_t hs_initials[5];
uint32_t high_scores[5];

unit_t units[NUM_UNITS];

// X and Y change patterns for the entry of the second and third wave of enemies
short formY_1[16] = {0,		0,	5,	5,  5,  0,  0,  -5,  -5,-5, -5, -5, -5, 0};
short formX_1[16] = {-40,	-10,-10,-10,-10,-10,-10,-10, -5,	0, 	5, 5, 10, 10};

// X and Y change patterns for the entry of the first wave of enemies	
short formY_2[25] = {-10, -10, -10, -10, -5, -5,  0,  0,  5, 5, 10, 10, 5, 5,  0,   0,  -5,  -5, -10, -10, -10, -5, -5, 0};
short formX_2[25] = { 0, 0,  0,  5,	 5, 10, 10,  5,  5,  0, 0,   -5,  -5, -10, -10, -5, -5, 0,  0, 0,  5,  5, 10, 10};

	
//*****************************************************************************
// Function Name: initialize_units
//*****************************************************************************
//	Summary: Initializes an array of character units
//
//*****************************************************************************
void initialize_units(){
	uint8_t i;
	
	for(i=0; i<NUM_UNITS; i++) {
		if(i==0) 
		{
			units[i].type = PLAYER;
			units[i].health = 1;
			units[i].formation_index = 0;
			units[i].pos.x = PLAYER_START_X;
			units[i].pos.y = PLAYER_START_Y;
		}
		else if(i<=4 && i!=0) {
			units[i].type = GALAGA;
			units[i].health = 0;
			if(i<=2) {
				units[i].pos.x = FORMATION_1_LEFT_START_X;
				units[i].pos.y = FORMATION_1_START_Y;
				units[i].formation_index = 4*(1-i);
			} else {
				units[i].pos.x = FORMATION_1_RIGHT_START_X;
				units[i].pos.y = FORMATION_1_START_Y;
				units[i].formation_index = 4*(i-4);
			}
		}
		else if(i>4&&i<=10) {
			units[i].type = BUTTERFLY;
			units[i].health = 0;
			if(i==7) {
					units[i].pos.x = FORMATION_1_LEFT_START_X;
					units[i].pos.y = FORMATION_1_START_Y;
					units[i].formation_index = -20;
			} else if(i==8) {
					units[i].pos.x = FORMATION_1_RIGHT_START_X;;
					units[i].pos.y = FORMATION_1_START_Y;
					units[i].formation_index = -20;
			} else if(i<=6) {
					units[i].pos.x = FORMATION_2_LEFT_START_X;
					units[i].pos.y = FORMATION_2_START_Y;
					units[i].formation_index = -24 + 4*(5-i);
			} else if(i>=9) {
					units[i].pos.x = FORMATION_2_RIGHT_START_X;
					units[i].pos.y = FORMATION_2_START_Y;
					units[i].formation_index = -24 + 4*(9-i);
			}
		}
		else if(i>10&&i<=16) {
			units[i].type = BEE;
			units[i].health = 0;
			if(i==13) {
				units[i].pos.x = FORMATION_1_LEFT_START_X;
				units[i].pos.y = FORMATION_1_START_Y;
				units[i].formation_index = -40;
			} else if(i==14) {
				units[i].pos.x = FORMATION_1_RIGHT_START_X;;
				units[i].pos.y = FORMATION_1_START_Y;
				units[i].formation_index = -40;
			} else if(i<=12) {
				units[i].pos.x = FORMATION_2_LEFT_START_X;
				units[i].pos.y = FORMATION_2_START_Y;
				units[i].formation_index = -40 + 4*(9-i);
			} else if(i>=15) {
				units[i].pos.x = FORMATION_2_RIGHT_START_X;
				units[i].pos.y = FORMATION_2_START_Y;
				units[i].formation_index = -40 * 4*(13-i);
			}
		}
		if(i==0) {
		}
		else if(i<=4){
				units[i].home_pos.x = 180 - i*30;
				units[i].home_pos.y = BOUNDRY_Y_TOP - 30;
		} else if (i<=10){
				units[i].home_pos.x = 180 - (i-5)*30;
				units[i].home_pos.y = BOUNDRY_Y_TOP - 60;
		} else {
				units[i].home_pos.x = 180 - (i-11)*30;
				units[i].home_pos.y = BOUNDRY_Y_TOP - 90;
		}
	}
};

//*****************************************************************************
// Function Name: update_LCD
//*****************************************************************************
//	Summary: prints a new frame to the LCD
//
//*****************************************************************************
void update_LCD () {
	uint32_t x, i;	
	char banner[] = "1UP    HIGH";
	char one_up[] = "1UP";
	
//	lcd_clear_screen(LCD_COLOR_BLACK);
	
	// Display Boundries
	for(x=0; x < 240; x++){		
			lcd_draw_px(x, 29, LCD_COLOR_WHITE);
	}
	for(x=0; x < 240; x++){		
		lcd_draw_px(x, 284, LCD_COLOR_WHITE);
	}
	
	//Print Extra Lives
	for(i=0; i<player_lives-1;i++){
		lcd_print_Image(210 - i*30, 0, 0, DIR_U);
	}
	
	//Print Score Info
	lcd_print_stringXY(banner, 1, 0, LCD_COLOR_RED, LCD_COLOR_BLACK );
	
	//Print units
	for(i=0;i<NUM_UNITS;i++){
		if(units[i].health > 0){
			lcd_print_Image(units[i].pos.x, units[i].pos.y, units[i].type, units[i].dir);
		}
	}
}


//*****************************************************************************
// Function Name: game_init
//*****************************************************************************
//	Summary: Initializes an array of character units
// 
//	Parameters:
//
//	draw: 				Pass in true to draw or false to erase
//
//*****************************************************************************
void game_init() {
	lcd_clear_screen(LCD_COLOR_BLACK);
	initialize_units();
	update_LCD();
}


//*****************************************************************************
// Function Name: move_to_destination
//*****************************************************************************
//	Summary: Updates position of a unit to move toward a destionation (x,y)
// 
//	Parameters:
//
//		unit_num:		Index of the unit to be updated
//
//		x:					x coordinate of destination
//
//		y:					y coordinate of destination
//
//	Return:
//
//		true: 			Unit is at destination
//		false:			Unit is not at destination
//
//*****************************************************************************
bool move_to_destination(int unit_num, uint16_t x, uint16_t y){
	bool x_reached = false;
	bool y_reached = false;
	int step = STEP;
	
	if((x - units[unit_num].pos.x) < step && (units[unit_num].pos.x - x) < step) {
			units[unit_num].pos.x = x;
			x_reached = true;
	} else if(units[unit_num].pos.x < x){
			units[unit_num].pos.x += step;
	}	else {
			units[unit_num].pos.x -= step;
	}
	
	if((y - units[unit_num].pos.y) < step && (units[unit_num].pos.y - y) < step) {
			units[unit_num].pos.y = y;
			y_reached = true;
	} else if(units[unit_num].pos.y < y){
			units[unit_num].pos.y += step;
	}	else {
			units[unit_num].pos.y -= step;
	}
	return (x_reached && y_reached);
}

void update_enemies() {
	uint8_t i;
	for(i=1; i<NUM_UNITS; i++) {
		
		if(units[i].health > 0){
			lcd_clear_Image(units[i].pos.x, units[i].pos.y);
		}
		// If still inactive and has not entered screen
		if(units[i].formation_index < 0)
		{
			units[i].formation_index++;
			// If the formation index is no longer negative, set the unit
			// to active by giving it a value for health.
			if(units[i].formation_index == 0) {
				units[i].health = 1;
				if(units[i].type == GALAGA) units[i].health ++;
			}
		} 
		
		// If in initial formation
		else if(units[i].formation_index < 25) 
		{
			if(i<=2 || i==7 || i==8) {
					units[i].pos.x +=  formX_2[units[i].formation_index];
					units[i].pos.y +=  formY_2[units[i].formation_index];
					units[i].formation_index ++;
			} else if (i<=4 || i==13 || i==14) {
					units[i].pos.x -=  formX_2[units[i].formation_index];
					units[i].pos.y +=  formY_2[units[i].formation_index];
				  units[i].formation_index ++;
			} else if (i==5 || i==6 || i==13 || i==14) {
				  units[i].pos.x +=  formX_1[units[i].formation_index];
					units[i].pos.y +=  formY_1[units[i].formation_index];
					if(units[i].formation_index < 15) units[i].formation_index ++;
				  else units[i].formation_index = 25;
			} else {
				  units[i].pos.x -=  formX_1[units[i].formation_index];
					units[i].pos.y +=  formY_1[units[i].formation_index];
					if(units[i].formation_index < 15) units[i].formation_index ++;
				  else units[i].formation_index = 25;
			}
		} 
		
		// If returning to its home state
		else if(units[i].formation_index==25) 
		{
				if(move_to_destination(i, units[i].home_pos.x, units[i].home_pos.y) )
					units[i].formation_index++;
		}
		
		// If in the general move state
		else
		{
			
		}
		if(units[i].health > 0){
			lcd_print_Image(units[i].pos.x, units[i].pos.y, units[i].type, units[i].dir);
		}
	}
}


void print_main_menu(){
	char title[] = "--GALAGA--";
	char start[] = "START GAME";
	char scores[] = "HIGH SCORE";



	lcd_print_stringXY(title, 2, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(start, 2, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(scores, 2,14, GALAGA_COLOR_2, LCD_COLOR_BLACK );
}
void print_game_over(){
	static bool state = false;
	char gameOver[] = "-GAME  OVER-";
	char score[] = "SCORE:";
	char msg[] = "TAP TO CONT";
	char score_value[9];
	
	itoa(player_score, score_value);
	
	if (state){
			lcd_print_stringXY(gameOver, 1, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
			state = false;
	} else {
			lcd_print_stringXY(gameOver, 1, 5, LCD_COLOR_BLACK, LCD_COLOR_BLACK );
			state = true;
	}
	
	lcd_print_stringXY(score, 0, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(score_value, 6, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );

	lcd_print_stringXY(msg, 1,16, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	
		
	
	
}

void print_high_scores(){
	char banner[] = "HIGH SCORES";
	char msg[] = "MAIN MENU";
	char* initials;
	char score_value[9];
	int i;
	
	lcd_print_stringXY(banner, 1, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg, 2,18, GALAGA_COLOR_2, LCD_COLOR_BLACK );

	
	for (i = 0; i < NUM_HIGH_SCORES; i++){
		//put_string(hs_initials[i]);
		
		initials = (char*)(hs_initials+i);
		itoa( high_scores[i] ,score_value);
		lcd_print_stringXY(initials, 1,(7+i), GALAGA_COLOR_3, LCD_COLOR_BLACK );
		lcd_print_stringXY(score_value, 5,(7+i), GALAGA_COLOR_3, LCD_COLOR_BLACK );
}
		itoa( player_score ,score_value);
		lcd_print_stringXY(score_value, 5,(7+6), GALAGA_COLOR_3, LCD_COLOR_BLACK );

	
}
void pull_high_scores(){
	uint8_t read_val;
	uint16_t addr;
	char* initials;
	int i,j;
	
	addr = ADDR_START;
	for( i = 0; i < NUM_HIGH_SCORES; i++){
		
		high_scores[i] = 0;
		
		for( j = 0; j < 4; j++){	
			eeprom_byte_read(I2C1_BASE,addr, &read_val);
			high_scores[i] |= read_val << (j*8);
			addr++;
		}
		
		
		hs_initials[i] = 0;
		initials = (char*)&hs_initials[i];
		for( j = 0; j < 3; j++){
			eeprom_byte_read(I2C1_BASE,addr, &read_val);
			initials[j] = (char)read_val;
			addr++;
		}
	}
	
}

void high_scores_test(){
	char kek[4]= "kek";
	pull_high_scores();
	player_score = 12543678;
	push_high_scores(kek);
	player_score = 23654789;
	push_high_scores(kek);
}

bool push_high_scores(char* initials){
	uint32_t low_score = 0xFFFFFFFF; //value of lowest high score
	int low_index = 0; //index of lowest high score
	uint8_t write_val;
	uint16_t addr;
	int i,j;
	
	pull_high_scores();
	
	for(i = 0; i < NUM_HIGH_SCORES; i++){
		if( high_scores[i] < low_score){
			low_index = i;
			low_score = high_scores[i];
		}
	}
	printf("low index: %d; low score:\n\r",low_score);

	if ( player_score > low_score ){
		addr = ADDR_START;
		addr += 7*low_index;
		
		
		for(i = 0; i < 4; i++){
			write_val = ( player_score >> (8*i) ) & 0xFF;
			eeprom_byte_write(I2C1_BASE,addr, write_val);
			addr++;
		}
		*initials = 'k';
		for(i = 0; i < 3; i++){
			write_val = (uint8_t) initials[i];
			eeprom_byte_write(I2C1_BASE,addr, (int)'Z');
			addr++;
		}
		printf("return true");
		return true;
		
	} else {
		printf("return false");
		return false;
	}
	
}



