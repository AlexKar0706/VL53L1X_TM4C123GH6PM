#include "SSD1306.h"
#include "../tm4c123gh6pm.h"
#include "../I2C/i2c.h"

//--------------------------------------Private-----------------------------------------//

//---------------------------DEFINES---------------------------//

#define WIDTH 128            //Width of the SSD1306 display
#define HEIGHT 64            //Height of the SSD1306 display
#define COMMANDBUFFERSIZE 64 //Size of the command buffer array
#define DATABUFFERSIZE 1024  //Size of the data buffer
#define SYMBOLWIDTH 6        //Width of the single character
#define MAXSYMBOLS 21        //Maximume number of symbols in one display line. This number is achived by int(128/6) = 21
#define MAXLINES 8           //Maximume number of display lines of the SSD1306 display

//----------------------------MACROS----------------------------//

#define CHECK_I2C(x) if (x) return SSD1306_I2C_ERR
#define CHECK_DATA(x) if (x) return SSD1306_WRONG_DATA

//---------------------------VARIABLES---------------------------//

static unsigned char mCommand_buffer[COMMANDBUFFERSIZE]; //Command buffer array

static unsigned char mData_buffer[DATABUFFERSIZE];       //Data buffer array

static unsigned short mCursor_X = 0; //X coordinate of text cursor
static unsigned short mCursor_Y = 0; //Y coordinate of text cursor

// This table contains the hex values that represent pixels
// for a font that is 5 pixels wide and 8 pixels high
static const unsigned char ASCII[][5] = {
   {0x00, 0x00, 0x00, 0x00, 0x00} // 20
  ,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
  ,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
  ,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
  ,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
  ,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
  ,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
  ,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
  ,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
  ,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
  ,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
  ,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
  ,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
  ,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
  ,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
  ,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
  ,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
  ,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
  ,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
  ,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
  ,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
  ,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
  ,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
  ,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
  ,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
  ,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
  ,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
  ,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
  ,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
  ,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
  ,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
  ,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
  ,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
  ,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
  ,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
  ,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
  ,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
  ,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
  ,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
  ,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
  ,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
  ,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
  ,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
  ,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
  ,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
  ,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
  ,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
  ,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
  ,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
  ,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
  ,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
  ,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
  ,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
  ,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
  ,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
  ,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
  ,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
  ,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
  ,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
  ,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
  ,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c '\'
  ,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
  ,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
  ,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
  ,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
  ,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
  ,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
  ,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
  ,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
  ,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
  ,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
  ,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
  ,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
  ,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
  ,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j
  ,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
  ,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
  ,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
  ,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
  ,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
  ,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
  ,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
  ,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
  ,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
  ,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
  ,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
  ,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
  ,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
  ,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
  ,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
  ,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
  ,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
  ,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
  ,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
  ,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
  ,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f DEL
  ,{0x1f, 0x24, 0x7c, 0x24, 0x1f} // 7f UT sign
};

//---------------------------FUNCTIONS---------------------------//

//-------------------Clear command buffer------------------------
//This function is setting all the value in the command_buffer to be equal to 0
//
//Function has following arguments:
//-> None
//
//Function has following outputs:
//-> None
static void _Clear_Command_Buffer() {
	int i;
	for (i = 0; i < COMMANDBUFFERSIZE; i++)
		mCommand_buffer[i] = 0x00;
}

//-------------------Clear data buffer------------------------
//This function is setting all the value in the data_buffer to be equal to 0
//
//Function has following arguments:
//-> None
//
//Function has following outputs:
//-> None
static void _Clear_Data_Buffer() {
	int i;
	for (i = 0; i < DATABUFFERSIZE; i++)
		mData_buffer[i] = 0x00;
}

//-------------------Delay 1ms------------------------
//This function simulates a 1 millisecond delay
//
//Function has following arguments:
//-> "ms" is a number of ms needed to be delayed
//
//Function has following outputs:
//-> None
static void _Delay1ms(unsigned long ms) {
	unsigned long time = 13333 * ms;
	while (time) 
		time--;
} 	

//--------------------------------------Public-----------------------------------------//

//---------------------------FUNCTIONS---------------------------//

//-------------------SSD1306 Send Command------------------------
//This function is handling command transmission to the SSD1306 display
//
//Function has following arguments:
//-> "bytes" is an array of SSD1306 commands (every command is a corresponding byte).
//An array must be range of 1-63 elements in total.
//-> "n_Bytes" is a number of commands in total in "bytes" array.
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_SendCommands(unsigned const char* bytes, 
				    unsigned long        n_Bytes)
{unsigned long i;
	
	_Clear_Command_Buffer();
	
	mCommand_buffer[0] = 0x00; //First transmission byte should be dummy byte
	
	for (i = 1; i < n_Bytes + 1; i++)
		mCommand_buffer[i] = bytes[i-1];
		
	CHECK_I2C(I2C_SendBytes(mCommand_buffer, n_Bytes + 1, SSD1306_ADDRESS));
	
	return SSD1306_OK;
}


//-------------------SDD1306 Initialisation------------------------
//This function is handling SSD1306 initialisation process.
//
//!IMPORTANT: This function has to be called before using any other function in the file!
//
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_Init() {
	
	//Initialisation of basic commands for the SSD1306 display
	unsigned char n_Commands[]    =  { SSD1306_DISPLAY_OFF,                        //Put display to the sleep mode for initialisation
                                           SSD1306_DISPLAY_CLOCK,  0x80,               //Set Fosc to 370kHz
                                           SSD1306_DISPLAY_OFFSET, 0x00,               //Set display offset to be equal to 0
					   SSD1306_MUX_RATIO,      0x3F,               //MUX ration to 1
					   SSD1306_SEG_REMAP_127,                      //Write data from left to right
					   SSD1306_COM_REMAP_REVERSE,                  //Write data from up to down
					   SSD1306_SET_COM_PINS,    (0x01 << 4) | 0x02,//Disable advanced COM remap
					   SSD1306_CONTRAST,        0x81,              //Set display contrast to 0x81 value
					   SSD1306_DISPLAY_START_LINE};                //Set start line to the display start
	unsigned char n_Commands2[]   =  { SSD1306_ADDR_MODE,       0x00,              //Set addressing mode to Horizontal mode
					   SSD1306_INVERSE_DISPLAY_OFF,                //Disable inversed display function
					   SSD1306_PRECHARGE,       0xF1,              //Set precharge period value to be equal to 0xF1
					   SSD1306_VCOMH_LVL,       0x30,              //Set Vcomh level to be equal to 0x30
					   SSD1306_CHARGE_PUMP_REGULATOR, 0x14,        //Enable charge pump regulation
					   SSD1306_ENTIRE_DISPLAY_OFF,                 //Disable entiry display function
					   SSD1306_SCROLL_STOP,                        //Disable scrolling function
					   SSD1306_DISPLAY_ON };                       //Activate display
	
	_Clear_Data_Buffer();    //Reset data buffer
	_Clear_Command_Buffer(); //Reset commands buffer
	
	CHECK_I2C(SSD1306_SendCommands(n_Commands, sizeof(n_Commands)));   //load first round of commands for the SSD1306
	CHECK_I2C(SSD1306_SendCommands(n_Commands2, sizeof(n_Commands2))); //load second round of commands for the SSD1306
																		 
	_Delay1ms(100);  //Wait 100ms for stabilization
	return SSD1306_Draw();  //Clear previous data of the display buffer
}

//-------------------SDD1306 Set cursor------------------------
//This function is needed to set cursor for text printing.
//
//!IMPORTANT: This function has only affect on text printing!
//
//Function has following arguments:
//-> "newX" is a new x coordinate for text printing. 
//This argument can be value from 0 to 20. (Max symbol number in 1 line)
//-> "newY" is a new y coordinate for text printing. 
//This argument can be value from 0 to 7. (Max lines number)
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_SetCursor(unsigned short newX, unsigned short newY) {
	CHECK_DATA(newX >= MAXSYMBOLS || newY >= MAXLINES);
	
	mCursor_X = newX * SYMBOLWIDTH;
	mCursor_Y = newY;
	
	return SSD1306_OK;
}

//-------------------SDD1306 Set pixel------------------------
//This function is needed to set 1 pixel in the display.
//
//!IMPORTANT: This function only load value to buffer.
//Draw() function should be called to display loaded pixels!						
//
//Function has following arguments:
//-> "x" is a X coordinate for the pixel. 
//This argument can be value from 0 to 127.
//-> "y" is a Y coordinate for the pixel. 
//This argument can be value from 0 to 63.
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_SetPixel(unsigned short x, unsigned short y) {
	CHECK_DATA(x >= WIDTH || y >= HEIGHT);
	
	mData_buffer[x + (y / MAXLINES) * WIDTH] |= (1 << (y & (MAXLINES - 1)));
	return SSD1306_OK;
}

//-------------------SDD1306 Draw------------------------
//This function transmit all the bytes to the SSD1306, 
//that was in data buffer to display them.		
//
//Function has following arguments:
//-> None
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_Draw() {
	int i = 0;
	unsigned long lineNumber = 0;
	unsigned char n_Commands[] = {0xE3, SSD1306_ADDR_PAGE_HV, 0x00, 0xFF,  //Set all SEG elements active in display
					    SSD1306_ADDR_COL_HV,  0x00, 0x7F}; //Set all COL elements active in display
	
	CHECK_I2C(SSD1306_SendCommands(n_Commands, sizeof(n_Commands)));																			
	
	//Start data buffer transmission, transmit 1 line (128 bytes), then stop and prepare to transmit next line																		 
	while (lineNumber < MAXLINES) {
		CHECK_I2C(I2C_StartTransmission(SSD1306_DISPLAY_START_LINE, SSD1306_ADDRESS));
		
		for (i = WIDTH*lineNumber; i < WIDTH + (WIDTH*lineNumber); i++)
			CHECK_I2C(I2C_TransmitByte(mData_buffer[i]));
		
		CHECK_I2C(I2C_StopTransmission());
		lineNumber++;
	}

	return SSD1306_OK;
}

//-------------------SDD1306 clear display------------------------
//This function clears SSD1306 display
//
//Function has following arguments:
//-> None
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_ClearDisplay(void) {
	_Clear_Data_Buffer();
	return SSD1306_Draw();
}

//-------------------SDD1306 Write Char------------------------
//This function load ASCII character to the data buffer.
// 
//!WARNING: If cursor reaches last line of the display, cursor will go to the start position
//and all the previous data will be cleared from the data buffer!
// 
//Function has following arguments:
//-> "ch" is a character, that is needed to be load to the buffer.
//This argument can be any ASCII character from [SPACE](20) to [DEL](127)
//If "ch" is a new line '\n' character, then function goes to the next line
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_WriteChar(const char ch) {int i;
	
	unsigned long dataIndex = mCursor_X + WIDTH * mCursor_Y;
	
	//Check if cursor Y reached it's end
	if (mCursor_Y >= MAXLINES) {
		mCursor_Y = mCursor_X = 0;
		_Clear_Data_Buffer();
	}
	
	//Check if character is '\n' or before [SPACE] character
	if (ch == '\n') {
		mCursor_X = 0;
		mCursor_Y++;
		return SSD1306_OK;
	} else if (ch < ' ') return SSD1306_WRONG_DATA;
	
	//Check if cursor X reached it's end
	if (mCursor_X + SYMBOLWIDTH > WIDTH) {
		mCursor_X = 0;
		mCursor_Y++;
		dataIndex = WIDTH * mCursor_Y;
	}
	
	//Use ASCII table to load character in data buffer
	for (i = 0; i < SYMBOLWIDTH-1; i++, mCursor_X++)
		mData_buffer[dataIndex + i] = ASCII[ch - ' '][i];
	mData_buffer[dataIndex + mCursor_X++] = 0x00;
	
	return SSD1306_OK;
}

//-------------------SDD1306 Write String------------------------
//This function load ASCII string to the data buffer, and then
//call Draw() function to send this buffer to the SSD1306 to display data.
//
//!WARNING: If cursor reaches last line of the display, cursor will go to the start position
//            and all the previous data will be cleared from the data buffer!	
//
//Function has following arguments:
//-> "str" is a string of characters, that is needed to be load to the buffer.
//This argument can be a string of any ASCII characters from [SPACE](20) to [DEL](127)
//If there is a new line '\n' character, then function goes to the next line
//
//Function has following outputs:
//-> "SSD1306Status" enum value representing status of function.
//Returns 0 (SSD1306_OK) if all is OK or returns error number 
SSD1306Status SSD1306_WriteStr(const char* str) {
	while(*str) {
		CHECK_DATA(SSD1306_WriteChar(*str));
		str++;
	}
	return SSD1306_Draw();
}
