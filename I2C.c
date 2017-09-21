/****************************************************

Low Level I2C Data Transreceiveing APIs

PLEASE SEE WWW.EXTREMEELECTRONICS.CO.IN FOR DETAILED 
SCHEMATICS,USER GUIDE AND VIDOES.

COPYRIGHT (C) 2008-2009 EXTREME ELECTRONICS INDIA

****************************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "I2C.h"


void I2CInit()
{
	//Set up TWI Module
	TWBR = 2;
	TWSR |=((1<<TWPS1)|(1<<TWPS0));

	//Enable the TWI Module
	TWCR|=(1<<TWEN);


}

void I2CClose()
{
	//Disable the module
	TWCR&=(~(1<<TWEN));
}


void I2CStart()
{
	//Put Start Condition on Bus
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

}

void I2CStop()
{
	//Put Stop Condition on bus
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
	//Wait for STOP to finish
	while(TWCR & (1<<TWSTO));
	//_delay_loop_2(250);
}

uint8_t I2CWriteByte(uint8_t data)
{
	
	TWDR=data;	

	//Initiate Transfer
	TWCR=(1<<TWEN)|(1<<TWINT);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check Status
	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40)
	{
		//SLA+W Transmitted and ACK received
		//or
		//SLA+R Transmitted and ACK received
		//or
		//DATA Transmitted and ACK recived

		return TRUE;
	}
	else
		return FALSE;	//Error
}

uint8_t I2CReadByte(uint8_t *data,uint8_t ack)
{
	//Set up ACK
	if(ack)
	{
		//return ACK after reception
		TWCR|=(1<<TWEA);
	}
	else
	{
		//return NACK after reception
		//Signals slave to stop giving more data
		//usually used for last byte read.
		TWCR&=(~(1<<TWEA));
	}

	//Now enable Reception of data by clearing TWINT
	TWCR|=(1<<TWINT);

	//Wait till done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) == 0x58 || (TWSR & 0xF8) == 0x50)
	{
		//Data received and ACK returned
		//	or
		//Data received and NACK returned

		//Read the data

		*data=TWDR;
		return TRUE;
	}
	else
		return FALSE;	//Error
	
}



