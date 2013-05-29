/*************************************************************************//**
 * \file   twi.h
 * \brief  Low-level TWI function declarations for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090419 - initial version
 *         20090421 - add TW_STATUS_MASK fix
 ****************************************************************************/

#ifndef TWI_H
#define TWI_H

#include<util/twi.h>

#define DEFAULT_TWI_ADDRESS 34

void twi_init( void );		///<Set up the TWI module for slave operation
void twi_check_conditions( void );
void twi_deinit();
#endif