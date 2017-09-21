/************************************************************

A Program to demonstrate the use of I2C RTC (Real Time Clock)
(DS3231). Here the DS3231 RTC Module is connected with an
AVR ATmega8A MCU by I2C Serial Bus.

This program reads time from the DS3231 and displays it in 
16x2 LCD Module. The system also has two buttons for user 
interaction. The 3 buttons are

1)MENU/Enter/Move selection.(Connected to PB2)
2)LEFT/Increase(Connected to PB1)
3)RIGHT/Decrease(Connected to PB0)

When powered up the display is like this.

|----------------|
|     DS3231     |
|  06:07:48: PM  |
|----------------|

Then you can press 1) Menu Key to bring up the main menu.

|----------------|
|    Main Menu   |
|< Set Time     >|
|----------------|

The Main Menu Has following options.

1)Set Time
2)Set On Time
3)Set Off Time
4)Quit

By Using the "Set Time" option the user can set the time.

Hardware
********
|ATmega8 Running at 1MHz Internal Clock. Fuse Byte set as
|follows.
|
|->LCD Connection
|  **************
|  A standard 16x2 LCD Module is connected as follows.
|  LCD PIN | ATmega8 PIN
|  ---------------------
|  D4        PD0
|  D5        PD1
|  D6        PD2
|  D7        PD3
|  E         PD4
|  RS        PD6
|  RW        PD5
|
|->Push Button Connection
|  **********************
|  1)MENU/Enter/Move selection.(Connected to PB2)
|  2)LEFT/Increase(Connected to PB1)
|  3)RIGHT/Decrease(Connected to PB0)
| 
|  All Buttons Connected Like This
|  
|  i/o pin of mcu ----[Button]--->GND
|
|->DS3231 RTC
|  **********
|  
|  DS3231 PIN | ATmega8 PIN
|  ------------------------
|  SDA          SDA
|  SCL          SCL
|
|  Both PINs must be pulled up to 5v by 4.7K resistors.


PLEASE SEE WWW.EXTREMEELECTRONICS.CO.IN FOR DETAILED 
SCHEMATICS,USER GUIDE AND VIDOES.

COPYRIGHT (C) 2008-2009 EXTREME ELECTRONICS INDIA
************************************************************/

#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

#include <I2C.h>
#include <I2C.c>
#include <lcd.h>
#include <lcd.c>
#include <DS3231.h>
#include <DS3231.c>

void Wait()
{
	uint8_t i;
	for(i=0;i<10;i++)
		_delay_loop_2(0);
}

uint8_t PREV_PINB=0xFF;
/*

Function to test the current status of keys(1 to 3)

returns
0 if NOT pressed
1 if Pressed

*/
uint8_t GetKeyStatus(uint8_t key)
{
	return (!(PINB & (1<<key)));
}

/*

Function to test the previous status of keys(1 to 3)

returns
0 if NOT pressed
1 if Pressed

*/
uint8_t GetPrevKeyStatus(uint8_t key)
{
	return (!(PREV_PINB & (1<<key)));
}

void main()
{
	//Initialize I2C Bus
	I2CInit();
	
	DS3231Write(0x0F,0b00000000); //Status Register
		
	DS3231Write(0x0E,0b00000000); //Clear Control Register bit
	//Write to Control Register to enable INTCN and both alarm
	uint8_t tempo;
	tempo=0b00000111;
	DS3231Write(0x0E,tempo); //Control Register
	//Wait Until Other device startup
	_delay_loop_2(0);
	
			
	//Initialize the LCD Module
	LCDInit(LS_BLINK);

	//Enable Pull ups on keys
	PORTB|=((1<<PB3)|(1<<PB2)|(1<<PB1));

	//Clear CH bit of RTC
	#define CH 7

	uint8_t temp;
	DS3231Read(0x00,&temp);

	//Clear CH Bit
	temp&=(~(1<<CH));

	DS3231Write(0x00,temp);

	//Set 24 Hour Mode
	DS3231Read(0x02,&temp);

	//Set 24 Hour BIT
	temp|=(0b00000000);

	//Write Back to DS3231
	DS3231Write(0x02,temp);

	LCDClear();
	
	LCDWriteString("   RTC DS3231   ");
	
	char Time[12];	//hh:mm:ss AM/PM
	
	//Now Read and format time
	uint8_t data;
	
	while(1)
	{
		CheckOffTime();
		
		DS3231Read(0x00,&data);
		Time[8]='\0';
		Time[7]=48+(data & 0b00001111);
		Time[6]=48+((data & 0b01110000)>>4);
		Time[5]=':';
	
		DS3231Read(0x01,&data);
		Time[4]=48+(data & 0b00001111);
		Time[3]=48+((data & 0b01110000)>>4);
		Time[2]=':';
	
		DS3231Read(0x02,&data);
		Time[1]=48+(data & 0b00001111);
		Time[0]=48+((data & 0b00110000)>>4);

		LCDClear();
		
		LCDWriteString("   RTC DS3231   ");
	
		LCDWriteStringXY(4,1,Time);
        
		uint8_t i;
		for(i=0;i<2;i++)
		{
		
			if(GetKeyStatus(2))
			{
				//Go To Main Menu
				ShowMainMenu();

				_delay_loop_2(0);

			}
			_delay_loop_2(100);
		}
	}
}

void ShowMainMenu()
{
	//The Main Menu
	char *menu_items[]={ 	"Set Time",
							"Set On Time",
							"Set Off Time",
							"Set Alarm",
							"Quit"
						};
	uint8_t menu_count=5;
	uint8_t selected=0;

	_delay_loop_2(0);
	_delay_loop_2(0);

	while(1)
	{
		LCDClear();
		LCDWriteString("    Main Menu  ");

		LCDWriteStringXY(2,1,menu_items[selected]);

		LCDWriteStringXY(0,1,"<");
		LCDWriteStringXY(15,1,">");

		if(GetKeyStatus(1))
		{
			//Left Key(No 1) is pressed
			//Check that it was not pressed previously
			if(!GetPrevKeyStatus(1))
			{
				if(selected !=0)
					selected--;
			}
		}

		if(GetKeyStatus(2))
		{
			//Right Key(No 0) is pressed
			//Check that it was not pressed previously
			if(!GetPrevKeyStatus(2))
			{
				if(selected !=(menu_count-1))
					selected++;
			}
		}

		if(GetKeyStatus(3))
		{
			//Enter Key Pressed
			//Check that it was not pressed previously
			if(!GetPrevKeyStatus(3))
			{
				//Call Appropriate Function
				switch (selected)
				{
					case 0:
						SetTime();
						break;
					case 1:
						SetOnTime();
						break;
					case 2:
						SetOffTime();
						break;
					case 3:
					    SetAlarm();
						break;
					case 4:
						return;//Quit
				}
				
			}
		}

		PREV_PINB=PINB;

		_delay_loop_2(5000);
	}
}

void SetTime()
{
	uint8_t hr,min,sec,am_pm,temp;

	//Read the Second Register
	DS3231Read(0x00,&temp);
	sec=(((temp & 0b01110000)>>4)*10)+(temp & 0b00001111);

	//Read the Minute Register
	DS3231Read(0x01,&temp);
	min=(((temp & 0b01110000)>>4)*10)+(temp & 0b00001111);

	//Read the Hour Register
	DS3231Read(0x02,&temp);
	hr=(((temp & 0b00110000)>>4)*10)+(temp & 0b00001111);

	uint8_t sel=0;

	while(1)
	{
		LCDClear();

		LCDWriteString("00:00:00 <OK>");
		
		LCDWriteIntXY(0,0,hr,2);
		LCDWriteIntXY(3,0,min,2);
		LCDWriteIntXY(6,0,sec,2);
        
		//Draw Pointer
		LCDWriteStringXY(sel*3,1,"^^");

		//Input Up key
		if(GetKeyStatus(1))
		{
			if(!GetPrevKeyStatus(1))
			{
				if(sel==0)
				{	
					//Hour
					if(hr==23)
					{
						hr=0;
					}
					else
					{
						hr++;
					}
				}

				if(sel==1)
				{	
					//Min
					if(min==59)
					{
						min=0;
					}
					else
					{
						min++;
					}
				}

				if(sel==2)
				{	
					//Sec
					if(sec==59)
					{
						sec=0;
					}
					else
					{
						sec++;
					}
				}

				if(sel == 3)
				{
					//OK
					break;
				}
			}
		}

		//Input Down
		if(GetKeyStatus(2))
		{
			if(!GetPrevKeyStatus(2))
			{
				if(sel==0)
				{	
					//Hour
					if(hr==0)
					{
						hr=23;
					}
					else
					{
						hr--;
					}
				}

				if(sel==1)
				{	
					//Min
					if(min==0)
					{
						min=59;
					}
					else
					{
						min--;
					}
				}

				if(sel==2)
				{	
					//Sec
					if(sec==0)
					{
						sec=59;
					}
					else
					{
						sec--;
					}
				}

				if(sel == 3)
				{
					//OK
					break;
				}
			}
		}

		if(GetKeyStatus(3))
		{
			if(!GetPrevKeyStatus(3))
			{
				//Change Selection
				if(sel==3)
					sel=0;
				else
					sel++;
			}
		}

		PREV_PINB=PINB;

		_delay_loop_2(30000);

	}

	//Now write time back to RTC Module
	temp=((sec/10)<<4)|(sec%10);
	DS3231Write(0x00,temp);

	temp=((min/10)<<4)|(min%10);
	DS3231Write(0x01,temp);

	temp=((hr/10)<<4)|(hr%10);
	temp|=0b00000000; // 24 Hr Mode
	
	DS3231Write(0x02,temp);

	LCDClear();
	LCDWriteString("Message !");
	LCDWriteStringXY(0,1,"Main Time Set");

	uint8_t i;
	for(i=0;i<3;i++)
		_delay_loop_2(0);
}

void SetOnTime()
{
	uint8_t hrA,minA,secA,tempA;
	
	//Read the Second Register
	DS3231Read(0x07,&tempA);
	secA=(((tempA & 0b01110000)>>4)*10)+(tempA & 0b00001111);

	//Read the Minute Register
	DS3231Read(0x08,&tempA);
	minA=(((tempA & 0b01110000)>>4)*10)+(tempA & 0b00001111);

	//Read the Hour Register
	DS3231Read(0x09,&tempA);
	hrA=(((tempA & 0b00110000)>>4)*10)+(tempA & 0b00001111);
	
	uint8_t sel=0;
		
	while(1)
	{
		LCDClear();
		
		LCDWriteString("00:00:00 <OK>");
		
		LCDWriteIntXY(0,0,hrA,2);
		LCDWriteIntXY(3,0,minA,2);
		LCDWriteIntXY(6,0,secA,2);
        
		
		//Draw Pointer
		LCDWriteStringXY(sel*3,1,"^^");

		//Input Up key
		if(GetKeyStatus(1))
		{
			if(!GetPrevKeyStatus(1))
			{
				if(sel==0)
				{	
					//Hour
					if(hrA==23)
					{
						hrA=0;
					}
					else
					{
						hrA++;
					}
				}

				if(sel==1)
				{	
					//Min
					if(minA==59)
					{
						minA=0;
					}
					else
					{
						minA++;
					}
				}

				if(sel==2)
				{	
					//Sec
					if(secA==59)
					{
						secA=0;
					}
					else
					{
						secA++;
					}
				}

				if(sel == 3)
				{
					//OK
					break;
				}
			}
		}

		//Input Down
		if(GetKeyStatus(2))
		{
			if(!GetPrevKeyStatus(2))
			{
				if(sel==0)
				{	
					//Hour
					if(hrA==0)
					{
						hrA=23;
					}
					else
					{
						hrA--;
					}
				}

				if(sel==1)
				{	
					//Min
					if(minA==0)
					{
						minA=59;
					}
					else
					{
						minA--;
					}
				}

				if(sel==2)
				{	
					//Sec
					if(secA==0)
					{
						secA=59;
					}
					else
					{
						secA--;
					}
				}

				if(sel == 3)
				{
					//OK
					break;
				}
			}
		}

		if(GetKeyStatus(3))
		{
			if(!GetPrevKeyStatus(3))
			{
				//Change Selection
				if(sel==3)
					sel=0;
				else
					sel++;
			}
		}

		PREV_PINB=PINB;

		_delay_loop_2(30000);

	}
	

	//Now write time back to RTC Module to set alarm value
	tempA=((secA/10)<<4)|(secA%10);
	DS3231Write(0x07,tempA);

	tempA=((minA/10)<<4)|(minA%10);
	DS3231Write(0x08,tempA);

	tempA=((hrA/10)<<4)|(hrA%10);
	DS3231Write(0x09,tempA);

	tempA=0b10000000;
	DS3231Write(0x0A,tempA);
	
	LCDClear();
	LCDWriteString("Message !");
	LCDWriteStringXY(0,1,"OnTime Set");

	uint8_t i;
	for(i=0;i<3;i++)
		_delay_loop_2(0);
}


void SetOffTime()
{
	uint8_t hrB=0,minB=0,tempB;

	//Read the Minute Register
	DS3231Read(0x01,&tempB);
	minB=(((tempB & 0b01110000)>>4)*10)+(tempB & 0b00001111);

	//Read the Hour Register
	DS3231Read(0x02,&tempB);
	hrB=(((tempB & 0b00110000)>>4)*10)+(tempB & 0b00001111);


	uint8_t sel=0;
	
	
	while(1)
	{
		LCDClear();
		
		LCDWriteString("00:00 <OK>");
		
		LCDWriteIntXY(0,0,hrB,2);
		LCDWriteIntXY(3,0,minB,2);
        
		//Draw Pointer
		LCDWriteStringXY(sel*3,1,"^^");

		//Input Up key
		if(GetKeyStatus(1))
		{
			if(!GetPrevKeyStatus(1))
			{
				if(sel==0)
				{	
					//Hour
					if(hrB==23)
					{
						hrB=0;
					}
					else
					{
						hrB++;
					}
				}

				if(sel==1)
				{	
					//Min
					if(minB==59)
					{
						minB=0;
					}
					else
					{
						minB++;
					}
				}

				if(sel == 2)
				{
					//OK
					break;
				}
			}
		}

		//Input Down
		if(GetKeyStatus(2))
		{
			if(!GetPrevKeyStatus(2))
			{
				if(sel==0)
				{	
					//Hour
					if(hrB==0)
					{
						hrB=23;
					}
					else
					{
						hrB--;
					}
				}

				if(sel==1)
				{	
					//Min
					if(minB==0)
					{
						minB=59;
					}
					else
					{
						minB--;
					}
				}

				if(sel == 2)
				{
					//OK
					break;
				}
			}
		}

		if(GetKeyStatus(3))
		{
			if(!GetPrevKeyStatus(3))
			{
				//Change Selection
				if(sel==3)
					sel=0;
				else
					sel++;
			}
		}

		PREV_PINB=PINB;

		_delay_loop_2(30000);

	}

	//Now write time back to RTC Module

	tempB=((minB/10)<<4)|(minB%10);
	DS3231Write(0x0B,tempB);

	tempB=((hrB/10)<<4)|(hrB%10);
	tempB|=0b00000000; // 24 Hr Mode
	
	DS3231Write(0x0C,tempB);

	LCDClear();
	LCDWriteString("Message !");
	LCDWriteStringXY(0,1,"OffTime Set");

	uint8_t i;
	for(i=0;i<5;i++)
		_delay_loop_2(0);
}

void SetAlarm()
{
	//The Main Menu
	char *menu_items[]={ 	"Enable",
							"Disable"
						};
	uint8_t menu_count=2;
	uint8_t selected=0;

	_delay_loop_2(0);
    _delay_loop_2(0);

	while(1)
	{
		LCDClear();
		LCDWriteString("    Alarm    ");

		LCDWriteStringXY(2,1,menu_items[selected]);

		LCDWriteStringXY(0,1,"<");
		LCDWriteStringXY(15,1,">");

		if(GetKeyStatus(1))
		{
			//Left Key(No 1) is pressed
			//Check that it was not pressed previously
			if(!GetPrevKeyStatus(1))
			{
				if(selected !=0)
					selected--;
			}
		}

		if(GetKeyStatus(2))
		{
			//Right Key(No 0) is pressed
			//Check that it was not pressed previously
			if(!GetPrevKeyStatus(2))
			{
				if(selected !=(menu_count-1))
					selected++;
			}
		}

		if(GetKeyStatus(3))
		{
			//Enter Key Pressed
			//Check that it was not pressed previously
			if(!GetPrevKeyStatus(3))
			{
				//Call Appropriate Function
				switch (selected)
				{
					case 0:
						DS3231Write(0x0E,0b00000111); //Enabling Alarm
						DS3231Write(0x0F,0b00000000);
						return;
					case 1:
						DS3231Write(0x0E,0b00000100);
						DS3231Write(0x0F,0b00000000);
						return;
				}
			}
		}

		PREV_PINB=PINB;

		_delay_loop_2(1000);
	}
	return;
}

void CheckOffTime()
{
	uint8_t hr=0, cekHr=0, min=0, cekMin=0;
	
	DS3231Read(0x01,&min);
	DS3231Read(0x0B,&cekMin);
	DS3231Read(0x02,&hr);
	DS3231Read(0x0C,&cekHr);
	
	if((min==cekMin)&&(hr==cekHr))
	{
		DS3231Write(0x0F,0b00000000); //Clear Status
	}
}