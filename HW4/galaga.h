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
#ifndef __GALAGA_H__
#define __GALAGA_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "TM4C123.h"
#include "galaga_bitmaps.h"


#define STEP	5;

#define FORMATION_1_LEFT_START_X		150
#define FORMATION_1_RIGHT_START_X		60
#define FORMATION_1_START_Y					250
#define FORMATION_2_LEFT_START_X		210
#define FORMATION_2_RIGHT_START_X		0
#define FORMATION_2_START_Y					100

#define BOUNDRY_Y_TOP								280
#define BOUNDRY_Y_BOTTOM						30

#define PLAYER_START_LIVES					3
#define PLAYER_START_X							108
#define PLAYER_START_Y							30

#define NUM_UNITS										17

#define UNIT_SIZE										24

#define SPACING											30
#define ROW_1_START									150
#define ROW_2_START									180
#define ROW_3_START									180

#define NUM_HIGH_SCORES							5

extern uint32_t player_score;
extern uint32_t high_scores[5];


#define DELAY_SMALL									-5
#define DELAY_LARGE									-40

#define FORMATION_1_LENGTH					14
#define FORMATION_2_LENGTH					24

#define NUM_PLAYER_BULLETS					5
#define PLAYER_BULLET_COLOR					0xF800
#define NUM_ENEMY_BULLETS						10
#define ENEMY_BULLET_COLOR					0xFFE0
#define BULLET_WIDTH								2
#define BULLET_HEIGHT								8
#define BULLET_SPEED								5
#define TRACKING_SPEED							1
#define HITBOX_BUFFER								1

void game_init();
bool update_enemies();
void update_player(bool left);

//*****************************************************************************
// Function Name: update_bullets
//*****************************************************************************
//	Summary: Updates the position of all bullets and checks for collisions.
// 
//*****************************************************************************
void update_bullets();
//void update_LCD();



void print_main_menu();
void print_game_over();

void pull_high_scores();
bool push_high_scores(char* initials);
void print_high_scores();
void print_new_record();

void high_scores_test();

#endif
