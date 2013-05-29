/*************************************************************************//**
 * \file   twi.c
 * \brief  Low-level TWI functions for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090714 - last edit jjrosent 
 ****************************************************************************/

#include<stdlib.h>	// for NULL
#include<avr/interrupt.h>
#include<util/twi.h>

#include"twi.h"
#include"prog.h"
#include"bootloader.h"

/// Input port mask for the TWI slave address switchbank
#define TWI_ADDR_MASK ( _BV(PORTD5) | _BV(PORTD4) | _BV(PORTD3) | _BV(PORTD2) | _BV(PORTD1) | _BV(PORTD0) )

// TWI write states.
#define TWI_WRITE_ADDR_HI_BYTE              (0x00)
#define TWI_WRITE_ADDR_LO_BYTE              (0x01)
#define TWI_WRITE_DATA_BYTE                 (0x02)

// TWI state values.
static uint8_t twi_write_state;

// TWI data values.
static uint16_t twi_address;

/// Interrupt handler for all things TWI
BOOTLOADER_SECTION void twi_check_conditions(){

	if( (TWCR & _BV(TWINT)) == _BV(TWINT) ){
	
		switch( TW_STATUS ) {
			
		case TW_BUS_ERROR:
			TWCR |= _BV( TWSTO );
			//twi_error();
			break;	// FIXME: error led
			
		case TW_SR_SLA_ACK: //SLA+W received, ACK returned
		case TW_SR_GCALL_ACK:	// FIXME? do something different for GCALL?
            twi_write_state = TWI_WRITE_ADDR_HI_BYTE;
			break;
			
		case TW_SR_DATA_ACK: //data received, ACK returned
		case TW_SR_GCALL_DATA_ACK: // FIXME? something different for GCALL?
            if(twi_write_state == TWI_WRITE_ADDR_HI_BYTE){

	            //set address
	            twi_address = TWDR;

	            // Set the next state.
	            twi_write_state = TWI_WRITE_ADDR_LO_BYTE;

	        }else if(twi_write_state == TWI_WRITE_ADDR_LO_BYTE){

				// Set the address
				twi_address = (twi_address << 8) | TWDR;

				// Set the next state.
				twi_write_state = TWI_WRITE_DATA_BYTE;

				// Mark the bootloader as active.
				//bootloader_active = 1;

				// Check for the special address to exit the bootloader.
				if (twi_address != 0xffff){
					// Set the twi address.  This will load the corresponding page from
					// flash into the programming buffer for reading and writing.
					prog_buffer_set_address(twi_address);
				}

	        }else{
	            prog_buffer_set_byte(TWDR);
            }
            break;
			
		case TW_SR_STOP: //stop or repeated start condition received while selected
            // Check for the special address to exit the bootloader.
            if (twi_address == 0xffff){
	            // Set the flag to have the bootloader eixt.
	            //bootloader_exit = 1;
            }else{
	            // Update the programming buffer if needed.
	            prog_buffer_update();
            }
			break;
			
		case TW_ST_SLA_ACK: ///SLA+R received, ACK returned
		case TW_ST_DATA_ACK: //data transmitted, ACK received
            TWDR = prog_buffer_get_byte();
            break;
			
		case TW_ST_DATA_NACK: ///data transmitted, NACK received //arduino transmits nack on last byte..pg151?
		case TW_ST_LAST_DATA:  //last data byte transmitted, ACK received
            // set TWEA again to respond when addressed
            TWCR = (TWCR & ~_BV( TWINT )) | _BV( TWEA );
            break;
			
		default:	// FIXME: error led
			//dumpbyte(TW_STATUS);
			//twi_error();
			break;
		}

		// clear the interrupt flag to allow the TWI module to continue
		TWCR |= _BV( TWINT );
	}		
}


BOOTLOADER_SECTION void twi_init( void ){
	
	// enable the TWI clock
	PRR &= ~_BV( PRTWI );

	// set SCL and SDA pins to inputs with internal pullups enabled
	PORTC |= _BV( PORTC4 ) | _BV( PORTC5 );
	DDRC &= ~( _BV(DDC4) | _BV(DDC5) );

	TWAR = (DEFAULT_TWI_ADDRESS << 1) | _BV( TWGCE );
		
	// configure the TWI module
	TWCR =	_BV( TWEA ) |	// ack when addressed or data received
		_BV( TWEN ) |	// enable TWI
		_BV( TWIE );	// enable the TWI interrupt

	// no prescaler necessary, since master mode is never used
}

BOOTLOADER_SECTION void twi_deinit(void)
// De-initialise USI.
{
	// Reset SCL and SDA.
	PORTC &= ~( _BV( PORTC4 ) | _BV( PORTC5 )  );
	DDRC |=  _BV(DDC4) | _BV(DDC5) ;

	// Clear the USI registers.
	TWAR = 0x00;
	TWCR = 0x00;
	PRR &= ~_BV( PRTWI );

}
