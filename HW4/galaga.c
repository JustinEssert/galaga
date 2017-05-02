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
uint32_t level;

uint32_t high_scores[5];

uint32_t hs_initials[5];
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
bool update_LCD () {
	uint32_t x, i;
	bool toggle = false;
	static short count = 0;
	char banner1[] = "HIGH         ";
	char banner2[] = "1UP          ";
	char lost_life[] = " TRY AGAIN ";

	itoa(high_score, (banner1 + 5));
	itoa(player_score, (banner2 + 5));
	// Display Boundries
	for(x=0; x < 240; x++){		
			lcd_draw_px(x, 29, LCD_COLOR_WHITE);
	}
	for(x=0; x < 240; x++){	
		lcd_draw_px(x, 284, LCD_COLOR_WHITE);
	}
	
	//Print Score Info
	lcd_print_stringXY(banner1, 0, 0, LCD_COLOR_RED, LCD_COLOR_BLACK );
	lcd_print_stringXY(banner2, 0, 1, LCD_COLOR_RED, LCD_COLOR_BLACK );
	
	if(units[0].active){
		lcd_print_Image(units[0].pos.x, units[0].pos.y, units[0].type, units[0].dir);
		//Print Extra Lives
		for(i=0; i<player_lives-1;i++){
			lcd_clear_Image(210 - i*30, 0);
			lcd_print_Image(210 - i*30, 0, 0, DIR_U);
		}
	}
	else{
		if(count == 0){
			player_lives -=1;		
			if(player_lives<=0) return false;
		}
		if(count<10){
			lcd_clear_Image(units[0].pos.x, units[0].pos.y);
			lcd_print_stringXY(lost_life, 0, 10, LCD_COLOR_RED, LCD_COLOR_BLACK );
			count++;
		}else {
			lcd_clear_screen(LCD_COLOR_BLACK);
			count = 0;
			units[0].pos.x = units[0].home_pos.x;
			units[0].pos.y = units[0].home_pos.y;
			units[0].move_state = INIT_FORMATION;
			units[0].health ++;
			units[0].active = true;
			lcd_print_Image(units[0].pos.x, units[0].pos.y, units[0].type, units[0].dir);
		}
	}
	return true;
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
	player_lives = PLAYER_START_LIVES;
	level = 1;
	player_score = 0;
	
	// Make sure high scores is up to date
	pull_high_scores();
	
	// Get the highest scores
	high_score = high_scores[0];
	for(i=1; i<NUM_HIGH_SCORES; i++) {
		if(high_score < high_scores[i]) high_score = high_scores[i];
	}
	
	// Initialize all bullets to inactive
	for(i=0;i<NUM_PLAYER_BULLETS;i++) player_bullets[i].active = false;
	for(i=0;i<NUM_ENEMY_BULLETS;i++)	enemy_bullets[i].active = false;

}

void level_up(){
	short i;
	lcd_clear_screen(LCD_COLOR_BLACK);
	initialize_units();
	update_LCD();
	level++;
	
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
// 			rand_num:						random 32 bit in used for shooting.
//
//*****************************************************************************
void fire_bullet (bool player_bullet, uint32_t rand_num) {
	short i, j, fire, freq;
	
	if((units[0].active)){
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
			fire = (int)(((rand_num)%100)/(95 - level*5));
			j = rand_num %(NUM_UNITS-1) + 1;
			if(fire && units[j].active){
				for(i=0;i<NUM_ENEMY_BULLETS;i++){
					if(!enemy_bullets[i].active){
						enemy_bullets[i].pos.x = units[j].pos.x + UNIT_SIZE/2;
						enemy_bullets[i].pos.y = units[j].pos.y;
						enemy_bullets[i].active = true;
						i=NUM_ENEMY_BULLETS;
					}
				}
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
	uint16_t x_old, y_old, num_enemies = 0;
	char test[10];
  long dX, dY;

	for(i=1; i<NUM_UNITS; i++) {
		x_old = units[i].pos.x;
		y_old = units[i].pos.y;
		
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
		dX = (long)units[i].pos.x - x_old;
		dY = (long)units[i].pos.y - y_old;
		// Update Direction based on the move
		
		if((dX > 0)&&(dY > 0))			units[i].dir = DIR_UL;
		else if((dX > 0)&&(dY < 0))	units[i].dir = DIR_DL;
		else if((dX < 0)&&(dY > 0))		units[i].dir = DIR_UR;
		else if((dX < 0)&&(dY < 0)) 	units[i].dir = DIR_DR;
		else if((dY < 0))						units[i].dir = DIR_D;
		else 													units[i].dir = DIR_U;
			
		
		
		if(units[i].active && units[i].move_state!=EXPLOSION)
		{
			if(units[i].type!=GALAGA || units[i].health!=1)
				lcd_print_Image(units[i].pos.x, units[i].pos.y, units[i].type, units[i].dir);
			else 
				lcd_print_Image(units[i].pos.x, units[i].pos.y, (short)(units[i].type)+1, units[i].dir);
		} 
		else if(units[i].active && units[i].move_state==EXPLOSION) {
			if(units[i].formation_index>=0){
				lcd_draw_explosion(units[i].pos.x, units[i].pos.y);
				units[i].formation_index--;
			} else {
				units[i].active = false;
			}
		}
		if(units[i].active) num_enemies++;
	}
	if(num_enemies <= 0) return true;
	return false;
}

//*****************************************************************************
// Function Name: update_player_score
//*****************************************************************************
//	Summary: Updates the players score after a hit
// 
//  Parameters:
//
//			defeated_player_index:		array index of the player that was defeated
//
//*****************************************************************************
void update_player_score
(
    short defeated_player_index
)
{
	uint16_t points;
	unitType_t type = units[defeated_player_index].type;
	
	// Check the type of character
	if(type==BUTTERFLY) 		points = 160;
	else if(type==BEE)  		points = 50;
	else if (type==GALAGA)	points = 400;
	
	// If still in initial configuration, 2x points
	if(units[defeated_player_index].move_state == INIT_FORMATION) points *= 2;
	
	// update player_score
	player_score += points;
}


//*****************************************************************************
// Function Name: update_bullets
//*****************************************************************************
//	Summary: Updates the position of all bullets and checks for collisions.
// 
//*****************************************************************************
void update_bullets(){
	short i,j,dX, dY, track_index;
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
							//Set the bullet to inactive
							player_bullets[i].active = false;
							
							units[j].health--;
							if(units[j].health == 0){
								// Change to an explosion and set formation_index to leave the explosion for 2 cycles
								units[j].move_state = EXPLOSION;
								units[j].formation_index = 2;
								// Draw the explosion
								lcd_draw_explosion(units[j].pos.x,units[j].pos.y);
								
								// Update Player's score
								update_player_score(j);
							}
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
			
			
			enemy_bullets[i].pos.y -= BULLET_SPEED;
			
			dX = enemy_bullets[i].pos.x-UNIT_SIZE/2 - units[0].pos.x;
			dY = enemy_bullets[i].pos.y-UNIT_SIZE/2 - units[0].pos.y;
			
			
			if(level>1){
				track_index++;
				if(track_index<=5-level){
					if(dX>0) enemy_bullets[i].pos.x 			-= TRACKING_SPEED;
					else if(dX<0) enemy_bullets[i].pos.x 	+= TRACKING_SPEED;
					track_index = 0;
				}
			}
			
			dX = enemy_bullets[i].pos.x-UNIT_SIZE/2 - units[0].pos.x;
			dY = enemy_bullets[i].pos.y-UNIT_SIZE/2 - units[0].pos.y;
			
			if(enemy_bullets[i].pos.y<BOUNDRY_Y_BOTTOM){
				enemy_bullets[i].active = false;
			} else if((dX>=HITBOX_BUFFER) && (dY>=HITBOX_BUFFER) && (dX<=UNIT_SIZE - HITBOX_BUFFER) && (dY<=UNIT_SIZE - HITBOX_BUFFER)){
				units[0].health--;
				if(units[0].health == 0){
					lcd_clear_Image(units[0].pos.x, units[0].pos.y);
					units[0].move_state = EXPLOSION;
					units[0].active = false;
				}
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

	if((units[0].active)){
		lcd_clear_Image(units[0].pos.x, units[0].pos.y);
		if(left && units[0].pos.x<=210) 			units[0].pos.x += 5;
		else if(!left &&units[0].pos.x>=5)		units[0].pos.x -= 5;
		
		lcd_print_Image(units[0].pos.x, units[0].pos.y, units[0].type, units[0].dir);
	}
}



//*****************************************************************************
// Function Name: print_main_menu
//*****************************************************************************
//	Summary: prints text on main menu
// 
//*****************************************************************************	
void print_main_menu(){
	char title[] = "GALAGA";
	char start[] = "START GAME";
	char scores[] = "HIGH SCORE";
  
	// Print title and menu items
	lcd_print_stringXY(title, 4, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(start, 2, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(scores, 2,14, GALAGA_COLOR_2, LCD_COLOR_BLACK );
}

//*****************************************************************************
// Function Name: print_game_over
//*****************************************************************************
//	Summary: prints text on game over screen
// 
//*****************************************************************************	
void print_game_over(){
	static bool state = false;
	char gameOver[] = "GAME  OVER";
	char score[] = "SCORE";
	char msg[] = "TAP TO CONT";
	char score_value[9];
	
	// convert player's score to a string that can be printeds
	itoa(player_score, score_value);
	

	// Print score and user prompt	
	lcd_print_stringXY(gameOver, 2, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(score, 0, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(score_value, 6, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg, 1,16, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	
		
	
	
}


//*****************************************************************************
// Function Name: print_high_scores
//*****************************************************************************
//	Summary: prints text on high scores screen
// 
//*****************************************************************************	
void print_high_scores(){
	char banner[] = "HIGH SCORES";
	char msg[] = "MAIN MENU";
	char you[] = "YOU";
	char* initials;
	char score_value[9];
	int i;
	
	// Print banner messages
	lcd_print_stringXY(banner, 1, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg, 2,18, GALAGA_COLOR_2, LCD_COLOR_BLACK );

	// Update scores before printing
	pull_high_scores();
	
	
	// For each score
	for (i = 0; i < NUM_HIGH_SCORES; i++){
		
		// cast initials back to char array
		initials = (char*)(hs_initials+i);
		
		// convert score to printable string
		itoa( high_scores[i] ,score_value);
		
		// print name next to score
		lcd_print_stringXY(initials, 1,(7+i), GALAGA_COLOR_3, LCD_COLOR_BLACK );
		lcd_print_stringXY(score_value, 5,(7+i), GALAGA_COLOR_3, LCD_COLOR_BLACK );	
	}
	
	// convert player score to printable string
	itoa( player_score ,score_value);
	
	// print player score below leaderboards
	lcd_print_stringXY(you, 1,(7+6), GALAGA_COLOR_3, LCD_COLOR_BLACK );
	lcd_print_stringXY(score_value, 5,(7+6), GALAGA_COLOR_3, LCD_COLOR_BLACK );

	
}

//*****************************************************************************
// Function Name: pull_high_scores
//*****************************************************************************
//	Summary: Reads high scores and names from EEPROM to highscores[] and
//					 initials[] respectively
// 
//*****************************************************************************	
void pull_high_scores(){
	uint8_t read_val;
	uint16_t addr;
	char* initials;
	int i,j;
	
	// set start address
	addr = ADDR_START;
	
	// for each high score slot
	for( i = 0; i < NUM_HIGH_SCORES; i++){
		
		// clear old data
		high_scores[i] = 0;
		
		// write each byte the correct location within the uint32
		for( j = 0; j < 4; j++){	
			eeprom_byte_read(I2C1_BASE,addr, &read_val);
			high_scores[i] |= read_val << (j*8);
			addr++;
		}
		
		// wipe old name data (sets all to null terminator)
		hs_initials[i] = 0;
		
		// cast hs_initials[i] to char array for easier access
		initials = (char*)&hs_initials[i];
		
		// write name data (initials) to first three indexes
		for( j = 0; j < 3; j++){
			eeprom_byte_read(I2C1_BASE,addr, &read_val);
			initials[j] = (char)read_val;
			addr++;
		}
	}
	
}

//*****************************************************************************
// Function Name: push_high_scores
//*****************************************************************************
//	Summary: writes player score and name to EEPROM to after verifying
//					 player's score is highest than lowest on leaderboard
//
//	Params:
//					 initials - the playser's entered initials
//
//	Returns:
//					 true - score written
//					 false - score not high enough to be written
//
//*****************************************************************************	
bool push_high_scores(char* initials){
	uint32_t low_score = 0xFFFFFFFF; //value of lowest high score
	int low_index = 0; //index of lowest high score
	uint8_t write_val;
	uint16_t addr;
	int i,j;
	
	// Upadate high scores before making changes
	pull_high_scores();
	
	// Search for value and index of lowest high score
	for(i = 0; i < NUM_HIGH_SCORES; i++){
		if( high_scores[i] < low_score){
			low_index = i;
			low_score = high_scores[i];
		}
	}

	// Only update if player score is greater than lowest score on list
	if ( player_score > low_score ){
		
		// Calculate address and offset of lowest score
		addr = ADDR_START;
		addr += 7*low_index;
		
		// send each byte of the uint32_t to the EEPROM
		for(i = 0; i < 4; i++){
			write_val = ( player_score >> (8*i) ) & 0xFF;
			eeprom_byte_write(I2C1_BASE,addr, write_val);
			addr++;
		}
		
		// send the first three bytes of name containing char data to EEPROM
		for(i = 0; i < 3; i++){
			write_val = (uint8_t) initials[i];
			eeprom_byte_write(I2C1_BASE,addr, (int)initials[i]);
			addr++;
		}
		return true;
	} else {
		return false;
	}
	
}
//*****************************************************************************
// Function Name: print_new_record
//*****************************************************************************
//	Summary: prints text on new high score/name entry screen
// 
//*****************************************************************************	
void print_new_record(){
	char banner[] = "NEW RECORD";
	char msg0[] = "USE STICK";
	char msg1[] = "AND R BTN";
	char msg2[] = "TO ENTER";
	char msg3[] = "YOUR NAME";
	char score[] = "SCORE";
	
	char score_value[9];
	
	// convert player's score to a string that can be printed
	itoa(player_score, score_value);
	
	// Print title and text entry directions for user
	lcd_print_stringXY(banner, 2, 3, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg0, 2,5, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg1, 2,6, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg2, 2,7, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(msg3, 2,8, GALAGA_COLOR_2, LCD_COLOR_BLACK );

	// Print players score
	lcd_print_stringXY(score, 0, 15, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(score_value, 6, 15, GALAGA_COLOR_2, LCD_COLOR_BLACK );

}


//*****************************************************************************
// Function Name: print_pause
//*****************************************************************************
//	Summary: prints text on pause screen
// 
//*****************************************************************************
void print_pause(){
	char title[] = "PAUSED";
	char start[] = "MAIN MENU";
	char scores[] = "RESUME";
  
	// Print title and menu items
	lcd_print_stringXY(title, 4, 5, GALAGA_COLOR_1, LCD_COLOR_BLACK );
	lcd_print_stringXY(start, 2, 11, GALAGA_COLOR_2, LCD_COLOR_BLACK );
	lcd_print_stringXY(scores, 3,14, GALAGA_COLOR_2, LCD_COLOR_BLACK );
}


