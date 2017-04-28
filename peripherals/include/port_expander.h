#ifndef __EEPROM_H__
#define __EEPROM_H__


#include <stdint.h>
#include "i2c.h"
#include "gpio_port.h"

#define MCP23017_DEV_ID				0x27
#define PEXP_TEST_NUM_BYTES    	20

//*****************************************************************************
// Defining IS2C configuration
//*****************************************************************************
#define   PEXP_GPIO_BASE         	GPIOA_BASE
#define   PEXP_I2C_BASE          	I2C1_BASE
#define   PEXP_I2C_SCL_PIN       	PA6
#define   PEXP_I2C_SDA_PIN       	PA7
#define   PEXP_I2C_SCL_PCTL_M     GPIO_PCTL_PA6_M
#define   PEXP_I2C_SCL_PIN_PCTL 	GPIO_PCTL_PA6_I2C1SCL
#define   PEXP_I2C_SDA_PCTL_M     GPIO_PCTL_PA7_M
#define   PEXP_I2C_SDA_PIN_PCTL  	GPIO_PCTL_PA7_I2C1SDA

//*****************************************************************************
// Defining push button pin configuration
//*****************************************************************************
#define		PEXP_BUTTON_M						0xF
#define		PEXP_BUTTON_RIGHT				0x8
#define		PEXP_BUTTON_LEFT				0x4
#define		PEXP_BUTTON_DOWN				0x2
#define		PEXP_BUTTON_UP					0x1

//*****************************************************************************
// Defining port expander configuration
//*****************************************************************************
#define		PEXP_GPIOB_ADDR				0x13
#define		PEXP_GPIOB_DIR				0x01
#define		PEXP_GPIOB_PU					0x0D
#define		PEXP_GPIOB_POL				0x04

//*****************************************************************************
// Reads a single byte of data from the  MCP24LC32AT EEPROM.  
//
// Paramters
//    i2c_base:   a valid base address of an I2C peripheral
//
//    address:    16-bit address of the byte being read.  Only the lower
//                12 bits is used by the EEPROM
//
//    data:       data read from the EEPROM is returned to a uint8_t pointer.
//
// Returns
// I2C_OK if the byte was read from the EEPROM.
//*****************************************************************************
i2c_status_t pexp_button_read 
(
	uint32_t  i2c_base,
	uint8_t   *data 
);





//*****************************************************************************
// Initialize the Port Expander peripheral
//*****************************************************************************
bool port_expander_init(void);
i2c_status_t pexp_button_config ( uint32_t  i2c_base );


#endif
