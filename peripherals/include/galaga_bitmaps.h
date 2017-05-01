#ifndef __GALAGA_BITMAPS_H__
#define __GALAGA_BITMAPS_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

#define       FONT_HEIGHT     16
#define       FONT_WIDTH      17

#define				SHIP_COLOR_1				0xF800 // Red
#define				SHIP_COLOR_2				0x001F // Blue
#define				SHIP_COLOR_3				0xFFFF // White

#define				GALAGA_COLOR_1			0xF88A // Red-Pink
#define				GALAGA_COLOR_2			0xFFE0 // Yellow
#define				GALAGA_COLOR_3			0x35EB // Green-Blue

#define				GALAGA_WEAK_COLOR_1	0xFAAA // Pink
#define				GALAGA_WEAK_COLOR_2	0xFFFF // White
#define				GALAGA_WEAK_COLOR_3	0x79DF // Purple

#define				BEE_COLOR_1					0xFFE0 // Yellow
#define				BEE_COLOR_2					0x501F // Blue
#define				BEE_COLOR_3					0xF800 // Red

#define				BUTTERFLY_COLOR_1		0x001F // Blue
#define				BUTTERFLY_COLOR_2		0xF800 // Red
#define				BUTTERFLY_COLOR_3		0xFFFF // White

#define				UNIT_WIDTH					24
#define				UNIT_HEIGHT					24

/* Font data for Sitka Small 12pt */
extern const uint8_t sitkaSmall_12ptBitmaps[];

/**********************************************************
* Function Name: lcd_print_stringXY
**********************************************************
* Summary: prints a string to the LCD screen at a specified
* XY location in specified foreground and background colors
* X will specify the number of characters across with X=0
* being the left most character position, and X=13 being
* the right most.  Y will specify the number of characters
* down the screen with Y=0 being the top of the screen and
* Y=19 being the bottom row of characters.  NOTE this is
* opposite of how columns and rows are specified for the
* IL9341.
* Returns:
*  Nothing
**********************************************************/
void lcd_print_stringXY(
    char *msg, 
    int8_t X,
		int8_t Y,
    uint16_t fg_color, 
    uint16_t bg_color
);


		
/*******************************************************************************
* Function Name: lcd_clear_Image
********************************************************************************
* Summary: clears a block of lcd that is the same size as one of the game sprites
*          
* Return:
*  Nothing
*******************************************************************************/ 
void lcd_clear_Image(
	  int16_t x,
		int16_t y
);

/*******************************************************************************
* Function Name: lcd_draw_explosion
********************************************************************************
* Summary: draws explotion at the given coordinates
*          
* Return:
*  Nothing
*******************************************************************************/ 
void lcd_draw_explosion(
	  int16_t x,
		int16_t y
);


/*******************************************************************************
* Function Name: lcd_print_Image
********************************************************************************
* Summary: draws sprite at given location. Sprite can be flipped over either
*					 axis and angled sprites are supported for units that have them.
*
* Params:		x			x coordinate of image corner
*						y			y coordinate of image corner
*						type	the unit type. determines sprite that will be drawn
*						dir		direction unit is facing. determins whether x and y will be
*										flipped, and whether the angled sprite will be used
*
*
* Return:
*  Nothing
*******************************************************************************/
void lcd_print_Image(
    int16_t x,
		int16_t y,
		short type,
		short dir
);

/*******************************************************************************
* Function Name: itoa
********************************************************************************
* Summary: int to string function. Takes a uint32 and converts the lower eight
*							decimel digits to an array of characters
*          
* Return:
*  char[9]	first 8 indexes are string, last index is null terminator
*******************************************************************************/ 
void itoa(
	uint32_t source,
	char* dest_ADDRESS_OF_SIZE_NINE_CHAR_ARRAY_PLEASE_AND_THANK_YOU
);
#endif
