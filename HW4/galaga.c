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
	DIR_DR,
	DIR_D,
	DIR_DL,
	DIR_UL
} direction_t;

typedef enum unitType {
	PLAYER,
	BUTTERFLY,
	BEE,
	GALAGA	
} unitType_t;

typedef enum moveState {
	INIT_FORMATION,
	RETURN_HOME,
	HOME,
	ATTACK,
	EXPLOSION
} moveState_t;

typedef struct position{
	uint16_t x;
	uint16_t y;
} position_t;
typedef struct unit {
	direction_t dir;
	unitType_t type;
	position_t pos;
	position_t home_pos;
	short health;
	short formation_index;
	moveState_t move_state;
	bool active;
} unit_t;

typedef struct bullet{
	bool active;
	position_t pos;
} bullet_t;

short player_lives = PLAYER_START_LIVES;

uint32_t high_score = 0, player_score=0;
uint32_t num_enemies= NUM_UNITS-1;

uint32_t high_scores[5];

uint32_t high_scores[5];

unit_t units[NUM_UNITS];
bullet_t player_bullets[NUM_PLAYER_BULLETS];
bullet_t enemy_bullets[NUM_ENEMY_BULLETS];

// X and Y change patterns for the entry of the second and third wave of enemies
short formY_1[FORMATION_1_LENGTH] = {  0,		0,	 5,	  5,   5,   0,   0,  -5,  -5, -5, -5, -5, -5,  0};
short formX_1[FORMATION_1_LENGTH] = {-10,	-20, -10, -20, -10, -10, -10, -10,  -5,	 0,  5,  5, 10, 10};

// X and Y change patterns for the entry of the first wave of enemies	
short formY_2[FORMATION_2_LENGTH] = {-10, -20, -10, -10, -5, -5,  0,  0,  5, 5, 10, 10,  5,   5,   0,  0, -5, -5, -10, -10, -10, -5, -5,  0};
short formX_2[FORMATION_2_LENGTH] = {  0,   0,   0,   5,	5, 10, 10,  5,  5, 0,  0, -5, -5, -10, -10, -5, -5,  0,   0,   0,   5,  5, 10, 10};

	
//*****************************************************************************
// Function Name: initialize_units
//*****************************************************************************
//	Summary: Initializes an array of character units
//
//*****************************************************************************
void initialize_units(){
	uint8_t i;

	for(i=0; i<NUM_UNITS; i++) {
		
		units[i].move_state					= INIT_FORMATION;
		units[i].health							= 1;
		units[i].active							= false;
		
		// Set type and home attributes
		if(i==0)
		{
			units[i].type 			= PLAYER;
			units[i].pos.x 			= PLAYER_START_X;
			units[i].pos.y 			= PLAYER_START_Y;
			units[i].home_pos.x = PLAYER_START_X;
			units[i].home_pos.y = PLAYER_START_Y;
			units[i].dir 				= DIR_U;
			units[i].active			= true;
		} 
		else if (i>0 && i<=4) 
		{
			units[i].type 			= GALAGA;
			units[i].home_pos.x = ROW_1_START - (SPACING*(i-1));
			units[i].home_pos.y = BOUNDRY_Y_TOP - SPACING;
			units[i].health			+= 1;
		} 
		else if (i>4 && i<=10) 
		{
			units[i].type 			= BUTTERFLY;
			units[i].home_pos.x = ROW_2_START - (SPACING*(i-5));
			units[i].home_pos.y = BOUNDRY_Y_TOP - SPACING*2;
		} 
		else if (i>10 && i<=16) 
		{
			units[i].type 			= BEE;
			units[i].home_pos.x = ROW_3_START - (SPACING*(i-11));
			units[i].home_pos.y = BOUNDRY_Y_TOP - SPACING*3;
		}
		
		// Set Initial Conditions based on the following configuration:
		// 											1		2		3		4		<--Formation 2 (first wave)
		//  								------|				|-------
		//	Formation 1 -->	5		6	|	7		8	|	9		10 <-- Formation 1
		//	(second wave)		11	12|	13	14|	15	16		(third wave)
		
		// First wave - left
		if(i==1 || i==2 || i==7 || i==13)
		{
			units[i].dir 							= DIR_D; 
			units[i].pos.x						= FORMATION_1_LEFT_START_X;
			units[i].pos.y 						= FORMATION_1_START_Y;
			if(i==7)
				units[i].formation_index 	= 0 + DELAY_SMALL*(3);
			else if(i==13)
				units[i].formation_index 	= 0 + DELAY_SMALL*(4);
			else
				units[i].formation_index 	= 0 + DELAY_SMALL*(i-1);
		} 
		// First wave - right
		else if(i==3 || i==4|| i==8 || i==14)
		{
			units[i].dir 							= DIR_D; 
			units[i].pos.x						= FORMATION_1_RIGHT_START_X;
			units[i].pos.y 						= FORMATION_1_START_Y;
			if(i==8)
				units[i].formation_index 	= 0 + DELAY_SMALL*(3);
			else if(i==14)
				units[i].formation_index 	= 0 + DELAY_SMALL*(4);
			else
			units[i].formation_index 	= 0 + DELAY_SMALL*(i-3);
		} 
		// Second wave
		else if (i == 5 || i==6 || i==11 || i==12) 
		{
			units[i].dir 							= DIR_UR; 
			units[i].pos.x						= FORMATION_2_LEFT_START_X;
			units[i].pos.y 						= FORMATION_2_START_Y;
			units[i].formation_index 	= DELAY_LARGE*2  + DELAY_SMALL*(i-5);
		} 
		// Third wave
		else if (i == 9 || i==10 || i==15 || i==16) 
		{
			units[i].dir 							= DIR_UL; 
			units[i].pos.x						= FORMATION_2_RIGHT_START_X;
			units[i].pos.y 						= FORMATION_2_START_Y;
			units[i].formation_index 	= DELAY_LARGE*2 + DELAY_SMALL*(i-9);
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
	
	lcd_print_Image(units[0].pos.x, units[0].pos.y, units[0].type, units[0].dir);
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
	short i;
	lcd_clear_screen(LCD_COLOR_BLACK);
	initialize_units();
	update_LCD();
	
	// Initialize all bullets to inactive
	for(i=0;i<NUM_PLAYER_BULLETS;i++) player_bullets[i].active = false;
	for(i=0;i<NUM_ENEMY_BULLETS;i++)	enemy_bullets[i].active = false;

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

//*****************************************************************************
// Function Name: fire_bullet
//*****************************************************************************
//	Summary: Creates a bullet unless all available bullets are active
//
//  Parameters:
//			player_bullet:			true if the player fires the bullet
//													false if an enemie fired the bullet
//			
// 			x										starting x coordinate of bullet
//
//			y										starting y coordinate of bullet
//*****************************************************************************
void fire_bullet (bool player_bullet, uint16_t x, uint16_t y) {
	short i;
	if(player_bullet){
		for(i=0;i<NUM_PLAYER_BULLETS;i++){
			if(!player_bullets[i].active){
				player_bullets[i].pos.x = units[0].pos.x + UNIT_SIZE/2;
				player_bullets[i].pos.y = units[0].pos.y + UNIT_SIZE;
				player_bullets[i].active = true;
				i=NUM_PLAYER_BULLETS;
			}
		}
	} else{
		for(i=0;i<NUM_ENEMY_BULLETS;i++){
			if(!enemy_bullets[i].active){
				enemy_bullets[i].pos.x = x;
				enemy_bullets[i].pos.y = y;
				enemy_bullets[i].active = true;
				i=NUM_ENEMY_BULLETS;
			}
		}
	}
}


//*****************************************************************************
// Function Name: update_enemies
//*****************************************************************************
//	Summary: Updates all of the enemies with respect to their move state
// 
//*****************************************************************************
bool update_enemies() {
	uint8_t i;
	uint16_t x_old, y_old;

	for(i=1; i<NUM_UNITS; i++) {
		
		if(units[i].active){
			num_enemies++;
			lcd_clear_Image(units[i].pos.x, units[i].pos.y);
		}
		
		if(units[i].move_state == INIT_FORMATION)
		{
			// If still inactive and has not entered screen
			if(units[i].formation_index < 0)
			{
				units[i].formation_index++;
			} 
			
			// If in initial formation
			else
			{
				if(units[i].formation_index == 0) units[i].active = true;
				if(i==1 || i==2 || i==7 || i==13) 
				{
						units[i].pos.x +=  formX_2[units[i].formation_index];
						units[i].pos.y +=  formY_2[units[i].formation_index];
						
					units[i].formation_index ++;
						if(units[i].formation_index == FORMATION_2_LENGTH) units[i].move_state = RETURN_HOME;
				} 
				else if (i==3 || i==4 || i==8 || i==14)
				{
						units[i].pos.x -=  formX_2[units[i].formation_index];
						units[i].pos.y +=  formY_2[units[i].formation_index];
						
						units[i].formation_index++;
						if(units[i].formation_index == FORMATION_2_LENGTH) units[i].move_state = RETURN_HOME;
				} 
				else if (i==5 || i==6 || i==11 || i==12) 
				{
						units[i].pos.x +=  formX_1[units[i].formation_index];
						units[i].pos.y +=  formY_1[units[i].formation_index];
						
						units[i].formation_index++;
						if(units[i].formation_index == FORMATION_1_LENGTH) units[i].move_state = RETURN_HOME;
				} 
				else 
				{
						units[i].pos.x -=  formX_1[units[i].formation_index];
						units[i].pos.y +=  formY_1[units[i].formation_index];
						
						units[i].formation_index++;
						if(units[i].formation_index == FORMATION_1_LENGTH) units[i].move_state = RETURN_HOME;
				}
			} 
		}
		// If returning to its home state
		else if(units[i].move_state == RETURN_HOME) 
		{
				if(move_to_destination(i, units[i].home_pos.x, units[i].home_pos.y) )
				units[i].formation_index++;
		}
		// If in the home state
		else if(units[i].move_state == HOME) 
		{
			
		} 
		// If in the attacking state
		else if(units[i].move_state == ATTACK) 
		{
			
		}

		// Update Direction based on the move
		if(units[i].pos.x - x_old > 0){
			if(units[i].pos.y - y_old >= 0) units[i].dir = DIR_UR;
			else 														units[i].dir = DIR_DR;
		}
		else if(units[i].pos.x - x_old < 0){
			if(units[i].pos.y - y_old >= 0) units[i].dir = DIR_UL;
			else 														units[i].dir = DIR_DL;
		} 
		else {
			if(units[i].pos.y - y_old >= 0) units[i].dir = DIR_U;
			else 														units[i].dir = DIR_D;
		}
		
		if(units[i].active && units[i].move_state!=EXPLOSION){
			lcd_print_Image(units[i].pos.x, units[i].pos.y, units[i].type, units[i].dir);
		} else if(units[i].active && units[i].move_state==EXPLOSION) {
			if(units[i].formation_index>=0){
				lcd_draw_explosion(units[i].pos.x, units[i].pos.y);
				units[i].formation_index--;
			} else {
				units[i].active = false;
				num_enemies--;
			}
		}
	}
	if(num_enemies <= 0) return true;
	return false;
}

//*****************************************************************************
// Function Name: update_bullets
//*****************************************************************************
//	Summary: Updates the position of all bullets and checks for collisions.
// 
//*****************************************************************************
void update_bullets(){
	short i,j,dX, dY;
	for(i=0;i<NUM_PLAYER_BULLETS;i++){
		if(player_bullets[i].active){
			//Reset the previous position to the background color
			lcd_draw_bullet(player_bullets[i].pos.x, BULLET_WIDTH, player_bullets[i].pos.y, BULLET_HEIGHT, LCD_COLOR_BLACK);
			
			//Update the position of the bullet based on BULLET_SPEED
			player_bullets[i].pos.y += BULLET_SPEED;
			
			//If the bullet has reached the top, set to inactive.
			if(player_bullets[i].pos.y>=BOUNDRY_Y_TOP){
				player_bullets[i].active = false;
			} 
			else {
				//Check each Enemy (if active)
				for(j=1;j<NUM_UNITS ;j++){
					if(units[j].active){
						
						// Calculate difference in position
						dX = player_bullets[i].pos.x - units[j].pos.x;
						dY = player_bullets[i].pos.y - units[j].pos.y;
						
						// If within the hitbox of unit J
						if((dX>HITBOX_BUFFER-BULLET_WIDTH) && (dY>HITBOX_BUFFER) && (dX<UNIT_SIZE - HITBOX_BUFFER) && (dY<UNIT_SIZE - HITBOX_BUFFER)){
							// Change to an explosion and set formation_index to leave the explosion for 2 cycles
							units[j].move_state = EXPLOSION;
							units[j].formation_index = 2;
							// Draw the explosion
							lcd_draw_explosion(units[j].pos.x,units[j].pos.y);
							
							//Set the bullet to inactive
							player_bullets[i].active = false;
						} else{
							
						}
					}
				}
				//If still active, reprint
				if(player_bullets[i].active) lcd_draw_bullet(player_bullets[i].pos.x, BULLET_WIDTH, player_bullets[i].pos.y, BULLET_HEIGHT,PLAYER_BULLET_COLOR);
			}
			
		}
	}
	for(i=0;i<NUM_ENEMY_BULLETS;i++){
		if(enemy_bullets[i].active){
			
			lcd_draw_bullet(enemy_bullets[i].pos.x, BULLET_WIDTH, enemy_bullets[i].pos.y, BULLET_HEIGHT,LCD_COLOR_BLACK);
			
			dX = enemy_bullets[i].pos.x - units[0].pos.x;
			dY = enemy_bullets[i].pos.y - units[0].pos.y;
			
			enemy_bullets[i].pos.y -= BULLET_SPEED;
			
			if(dX>0) enemy_bullets[i].pos.x += TRACKING_SPEED;
			else if(dX<0) enemy_bullets[i].pos.x -= TRACKING_SPEED;
			
			if(enemy_bullets[i].pos.y<BOUNDRY_Y_BOTTOM){
				enemy_bullets[i].active = false;
			} else if((dX>=HITBOX_BUFFER) && (dY>=HITBOX_BUFFER) && (dX<=UNIT_SIZE - HITBOX_BUFFER) && (dY<=UNIT_SIZE - HITBOX_BUFFER)){
				units[0].move_state = EXPLOSION;
				lcd_draw_explosion(units[0].pos.x,units[0].pos.y);
				enemy_bullets[i].active = false;
			} else{
				lcd_draw_bullet(enemy_bullets[i].pos.x, BULLET_WIDTH, enemy_bullets[i].pos.y, BULLET_HEIGHT,ENEMY_BULLET_COLOR);
			}
		}
	}
}

//*****************************************************************************
// Function Name: update_player
//*****************************************************************************
//	Summary: Updates the position of player
// 
//*****************************************************************************
void update_player(bool left) {
	lcd_clear_Image(units[0].pos.x, units[0].pos.y);
	
	if(left && units[0].pos.x<=210) 			units[0].pos.x += 5;
	else if(!left &&units[0].pos.x>=5)		units[0].pos.x -= 5;
	
	lcd_print_Image(units[0].pos.x, units[0].pos.y, units[0].type, units[0].dir);
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

	pull_high_scores();
	
	for (i = 0; i < NUM_HIGH_SCORES; i++){
		initials = (char*) &high_scores[i];
		itoa( (high_scores[i]&0xFFFF) ,score_value);
		lcd_print_stringXY(initials, 1,(7+i), GALAGA_COLOR_3, LCD_COLOR_BLACK );
		lcd_print_stringXY(score_value, 5,(7+i), GALAGA_COLOR_3, LCD_COLOR_BLACK );
}
	
	
}
void pull_high_scores(){
	uint8_t read_val;
	uint16_t addr;
	int i;
	
	addr = ADDR_START;
	for( i = 0; i < NUM_HIGH_SCORES; i++){
		high_scores[i] = 0;
		eeprom_byte_read(I2C1_BASE,addr, &read_val);
		high_scores[i] |= read_val;
		addr++;
			
		eeprom_byte_read(I2C1_BASE,addr, &read_val);
		high_scores[i] |= read_val << 4;
		addr++;
			
		eeprom_byte_read(I2C1_BASE,addr, &read_val);
		high_scores[i] |= (read_val & 0xFC ) << 12; // sets last char to null
		addr++;	
	}
	
}
bool push_high_scores(char* initials){
	int low_score = 0xFFFF; //value of lowest high score
	int low_index = 0; //index of lowest high score
	uint8_t write_val;
	uint16_t addr;
	int i;
	
	pull_high_scores();
	
	for(i = 0; i < NUM_HIGH_SCORES; i++){
		if( (high_scores[i] & 0xFFFF) < low_score){
			low_index = i;
			low_score = high_scores[i] & 0xFFFF;
		}
	}
	printf("low index: %d; low score:\n\r",low_score);

	if ( player_score > low_score ){
		addr = ADDR_START;
		addr += 3*low_index;
		write_val = player_score & 0xFF;
		eeprom_byte_write(I2C1_BASE,addr, write_val);
		addr++;
		
		write_val = (player_score >> 4) & 0xFF;
		eeprom_byte_write(I2C1_BASE,addr, write_val);
		addr++;
		
		write_val = (uint8_t) *initials;
		write_val &= 0xFC;	 // sets last char to null term regardless of current value
		eeprom_byte_write(I2C1_BASE,addr, write_val);
		printf("return true");
		return true;
		
	} else {
		printf("return false");
		return false;
	}
	
}


void high_scores_test(){
	pull_high_scores();
	player_score = 99999999;
	push_high_scores("kek");
}
