#include "port_expander.h"

//*****************************************************************************
// Reads the push buttons from the port expander.  
//
// Paramters
//    i2c_base:   a valid base address of an I2C peripheral
//
//    data:       Bitmask of pushbutton read as defined by port_expander.h.
//
// Returns
// I2C_OK if the byte was read from the port expander.
//*****************************************************************************
i2c_status_t pexp_read_buttons
( 
  uint32_t  i2c_base,
  uint8_t   *data
)
{
  i2c_status_t status;
 // *data = 0;
  // Before doing anything, make sure the I2C device is idle
   while ( I2CMasterBusy(i2c_base)) {};

  //==============================================================
  // Write the GPIOB address to the port expander
  //==============================================================
	
	// Set the Slave address as a write to send the GPIOB address
	status = i2cSetSlaveAddr(i2c_base, MCP23017_DEV_ID, I2C_WRITE);
	if(!status==I2C_OK) return status;

  // Send the address of GPIOB to the port expander
	status = i2cSendByte(i2c_base, PEXP_GPIOB_ADDR, I2C_MCS_START | I2C_MCS_RUN);
	if(!status==I2C_OK) return status;

  //==============================================================
  // Read from the port expander
  //==============================================================
	
	// Set the Slave address as a read to get the GPIOB values
	status = i2cSetSlaveAddr(i2c_base, MCP23017_DEV_ID, I2C_READ);
	if(!status==I2C_OK) return status;

  // Read the GPIOB values
	status = i2cGetByte(i2c_base, data, I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP);
	if(!status==I2C_OK) return status;
	
	*data = ~*data;
	
  return I2C_OK;
}


//*****************************************************************************
// Initializes the port expander's GPIO B ports to be inputs and pull-up
//
// Paramters
//    i2c_base:   a valid base address of an I2C peripheral
//
// Returns
// I2C_OK if completed without error.
//*****************************************************************************
i2c_status_t pexp_button_config
( 
  uint32_t  i2c_base
)
{
  i2c_status_t status;
  
  // Before doing anything, make sure the I2C device is idle
  while ( I2CMasterBusy(i2c_base)) 
	{
	};

  //==============================================================
  // Set the pins to be inputs
  //==============================================================
	
	// Set the Slave address as a write to send the direction address
	status = i2cSetSlaveAddr(i2c_base, MCP23017_DEV_ID, I2C_WRITE);
	if(!status==I2C_OK) return status;
	
	// Send the address of the direction register to the port expander
  status = i2cSendByte(i2c_base, PEXP_GPIOB_DIR, I2C_MCS_START | I2C_MCS_RUN);
	if(!status==I2C_OK) return status;

	// Set the DIR to be inputs by writing all ones
	status = i2cSendByte(i2c_base, 0xFF, I2C_MCS_RUN | I2C_MCS_STOP);
	if(!status==I2C_OK) return status;

  //==============================================================
  // Set the pins to be pull up
  //==============================================================
	
	// Set the Slave address as a write to send the pull-up address
	status = i2cSetSlaveAddr(i2c_base, MCP23017_DEV_ID, I2C_WRITE);
	if(!status==I2C_OK) return status;

	// Send the address of the pull-up to the port expander
  status = i2cSendByte(i2c_base, PEXP_GPIOB_PU, I2C_MCS_START | I2C_MCS_RUN);
	if(!status==I2C_OK) return status;

	// Set the pull-up to be inputs by writing all ones
	status = i2cSendByte(i2c_base, 0xFF, I2C_MCS_RUN | I2C_MCS_STOP);
	if(!status==I2C_OK) return status;
	
  return status;
}

//*****************************************************************************
// Initializes the port expander 
//
// Returns
// I2C_OK if the byte was read from the port expander.
//*****************************************************************************
bool port_expander_init(void)
{
  
  if(gpio_enable_port(PEXP_GPIO_BASE) == false)
  {
    return false;
  }
  
  // Configure SCL 
  if(gpio_config_digital_enable(PEXP_GPIO_BASE, PEXP_I2C_SCL_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_alternate_function(PEXP_GPIO_BASE, PEXP_I2C_SCL_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_port_control(PEXP_GPIO_BASE, PEXP_I2C_SCL_PCTL_M, PEXP_I2C_SCL_PIN_PCTL)== false)
  {
    return false;
  }
    

  
  // Configure SDA 
  if(gpio_config_digital_enable(PEXP_GPIO_BASE, PEXP_I2C_SDA_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_open_drain(PEXP_GPIO_BASE, PEXP_I2C_SDA_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_alternate_function(PEXP_GPIO_BASE, PEXP_I2C_SDA_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_port_control(PEXP_GPIO_BASE, PEXP_I2C_SDA_PCTL_M, PEXP_I2C_SDA_PIN_PCTL)== false)
  {
    return false;
  }
    
  //  Initialize the I2C peripheral
  if( initializeI2CMaster(PEXP_I2C_BASE)!= I2C_OK)
  {
    return false;
  }
	
	pexp_button_config(I2C1_BASE);
  
  return true;
  
}

