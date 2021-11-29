#include "i2c.h"
#include "pll.h"
#include "../tm4c123gh6pm.h"

//--------------------------------------Private-----------------------------------------//

//---------------------------DEFINES---------------------------//

#define GPIO_PORTD_PD0 0x00000001
#define GPIO_PORTD_PD1 0x00000002

//---------------------------VARIABLES---------------------------//

static unsigned char mLow_transmission = 0;  //Low level transmission state


//--------------------------------------Public-----------------------------------------//

//---------------------------FUNCTIONS---------------------------//

//-------------------I2C initialisation------------------------
//This function is initialise I2C protocol of master device for microcontroller
//
//For this initialisation is taken third I2C module of TM4C123GH6PM (I2C3)
//I2C3 module use Port D for SDA/SCL lines:
//-> PD0 = SCL(clock) line of I2C3 module
//-> PD1 = SDA(data) line of I2C3 module 
//
//Function has following arguments:
//-> "status" is option to use PLL or not
//in .h file is made predefined values for this argument
//
//Function has following outputs:
//-> None
void I2C_Init(unsigned short status) {volatile unsigned long delay;
	if (status == I2C_WITH_PLL) {
		/* PLL 80MHz initialisation */
		PLL_Init(4);
	}
	
	/* Clock initialisation */
	SYSCTL_RCGCI2C_R   |= SYSCTL_RCGCI2C_R3;
	delay               = SYSCTL_RCGCI2C_R;
	SYSCTL_RCGC2_R     |= SYSCTL_RCGC2_GPIOD;
	delay               = SYSCTL_RCGC2_R;
	
	/* Port D Initialisation */
	GPIO_PORTD_AFSEL_R |=  ( GPIO_PORTD_PD0 |
				 GPIO_PORTD_PD1 );
	GPIO_PORTD_AMSEL_R &= ~( GPIO_PORTD_PD0 |
				 GPIO_PORTD_PD1 );
	GPIO_PORTD_DEN_R   |=  ( GPIO_PORTD_PD0 |
				 GPIO_PORTD_PD1 );
	GPIO_PORTD_ODR_R   |=    GPIO_PORTD_PD1;
	GPIO_PORTD_PCTL_R  |=  ( GPIO_PCTL_PD1_I2C3SDA |
				 GPIO_PCTL_PD0_I2C3SCL );
	
	/* I2C Initialisation */
	I2C3_MCR_R   |= 0x10;

	if (status == I2C_WITH_PLL) {
		I2C3_MTPR_R   = (I2C_MTPR_HS | 0x03); //TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
											  //TPR = (80 MHz/(2*(2+1)*3330000))-1;
											  //TPR = 3 + HS stands for High-speed mode (3.33 MHz)
	}

	if (status == I2C_WITHOUT_PLL) {
		I2C3_MTPR_R   = 0x02; 				  //TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
											  //TPR = (16 MHz/(2*(6+4)*300000))-1;
											  //TPR = 2 (300kB/s)
	}
}

//-------------------------------Low level of transmission------------------------------------//
//Here is given low level of data transmission, that is not fully handled by the file
//The algorithm to handle low level transmission is next:
//1. Start transmission by calling I2C_StartTransmission(byte, address).
//2. Transmit as many bytes, as it's needed by calling I2C_TransmitByte(byte).
//3. End transsmion to specifed address by calling I2C_StopTransmission().


//-------------------I2C start transmission------------------------
//This function starts I2C transmission with specifed address
//
//!IMPORTANT: This function is a part of low level transmission and
//therefore it is forbidden to use both functions of
//high level transmission and low level transmission
//at the same time!
//
//Function has following arguments:
//-> "byte" is start byte of data, that will be transmitted. 
//If it is not needed, dummy byte can be sended instead.
//-> "address" is an address of specific master/slave device
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_StartTransmission(unsigned const char byte,
                                          unsigned const char address) 
{
	if (mLow_transmission) return I2CERROR; //Check if low level transsmision is already active
	
	mLow_transmission = 1;
	
	//Load data and address in appropriate microcontroller registers
	I2C3_MSA_R  = address;
	I2C3_MDR_R  = byte;
	
	//Polling BUSBSY bit of I2C3 module control register
	while (I2C3_MCS_R&I2C_MCS_BUSBSY) {
	};
	
	//Start -> Send -> Continue I2C transmission
	I2C3_MCS_R = ( I2C_MCS_RUN   |
		       I2C_MCS_START );
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};
	
	if (I2C3_MCS_R&I2C_MCS_ERROR) return I2CERROR;
		
	return I2COK;
}

//-------------------I2C transmit byte------------------------
//This function continue I2C transmission with specifed address
//
//!IMPORTANT: This function is a part of low level transmission and
//            therefore it is forbidden to use both functions of
//            high level transmission and low level transmission
//            at the same time!
//
//Function has following arguments:
//-> "byte" is byte of data, that will be transmitted.
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_TransmitByte(unsigned char byte) {
	
	if (!mLow_transmission) return I2CERROR; //Check if low level transsmision is not active
	
	//Load data
	I2C3_MSA_R &= ~0x01;
	I2C3_MDR_R  =  byte;
	
	// Send -> Continue I2C transmission
	I2C3_MCS_R =  I2C_MCS_RUN;
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};

	if (I2C3_MCS_R&I2C_MCS_ERROR) {
		if (I2C3_MCS_R&I2C_MCS_ARBLST) I2C3_MCS_R = I2C_MCS_STOP;
		return I2CERROR;
	}
	
	return I2COK;
}

//-------------------I2C read byte------------------------
//This function continue I2C transmission with specifed address
//
//!IMPORTANT: This function is a part of low level transmission and
//            therefore it is forbidden to use both functions of
//            high level transmission and low level transmission
//            at the same time!
//
//Function has following arguments:
//-> "buffer" is a buffer, where will be stored 1 byte
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_ReciveByte(unsigned char* buffer) {
	
	if (!mLow_transmission) return I2CERROR; //Check if low level transsmision is not active
	
	// Read -> Continue I2C transmission
	I2C3_MSA_R |= 0x01;
	I2C3_MCS_R =  ( I2C_MCS_RUN | 
					I2C_MCS_ACK );
	
	//Store byte
	*buffer = I2C3_MDR_R&0xFF;
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};

	if (I2C3_MCS_R&I2C_MCS_ERROR) {
		if (I2C3_MCS_R&I2C_MCS_ARBLST) I2C3_MCS_R = I2C_MCS_STOP;
		return I2CERROR;
	}
	
	return I2COK;
}

//-------------------I2C stop transmission------------------------
//This function stops I2C transmission with specifed address
//
//!IMPORTANT: This function is a part of low level transmission and
//            therefore it is forbidden to use both functions of
//            high level transmission and low level transmission
//            at the same time!
//
//Function has following arguments:
//-> None
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_StopTransmission() {
	if (!mLow_transmission) return I2CERROR; //Check if low level transsmision is not active
	
	mLow_transmission = 0;
	
	// Stop I2C transmission
	I2C3_MCS_R =  I2C_MCS_STOP;
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};

	if (I2C3_MCS_R&I2C_MCS_ERROR) {
		if (I2C3_MCS_R&I2C_MCS_ARBLST) I2C3_MCS_R = I2C_MCS_STOP;
		return I2CERROR;
	}
	
	return I2COK;
}

//-------------------------------High level of transmission------------------------------------//
//Here is given high level of data transmission, that is fully handled by the file
//It means all the functions down below will handle start-transmit-stop algorithm by themselves


//-------------------I2C send byte------------------------
//This function send 1 byte via I2C transmission with specifed address
//
//Function has following arguments:
//-> "byte" is byte of data, that will be transmitted. 
//-> "address" is an address of specific master/slave device
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_SendByte(unsigned const char byte,
				 unsigned const char address)  
{
	if (mLow_transmission) return I2CERROR; //Check if low level transsmision is already active
	
	//Load data and address in appropriate microcontroller registers
	I2C3_MSA_R  = address & (~0x01);
	I2C3_MDR_R  = byte;
	
	//Polling BUSBSY bit of I2C3 module control register
	while (I2C3_MCS_R&I2C_MCS_BUSBSY) {};
		
	//Start -> Send -> Stop I2C transmission
	I2C3_MCS_R = ( I2C_MCS_RUN   |
		       I2C_MCS_START |
		       I2C_MCS_STOP  );
		 
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};
	
	if (I2C3_MCS_R&I2C_MCS_ERROR) return I2CERROR;
		
	return I2COK;
}


//-------------------I2C send bytes------------------------
//This function send bytes via I2C transmission with specifed address
//
//Function has following arguments:
//-> "bytes" is an array of bytes, that will be transmitted.
//-> "n_Bytes" is a number of bytes in array
//-> "address" is an address of specific master/slave device
//
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_SendBytes(unsigned const char* bytes,
                                  unsigned long        bufferSize,
				  unsigned const char  address)	
{
	if (mLow_transmission) return I2CERROR; //Check if low level transsmision is already active
	
	if (bufferSize == 1) return I2C_SendByte(*bytes, address); //Check if only 1 byte was passed
	
	//Load data and address in appropriate microcontroller registers
	I2C3_MSA_R  = address & (~0x01);
	I2C3_MDR_R  = *bytes++;
	
	//Polling BUSBSY bit of I2C3 module control register
	while (I2C3_MCS_R&I2C_MCS_BUSBSY) {};
	
	//Start -> Send -> Continue I2C transmission
	I2C3_MCS_R =  ( I2C_MCS_RUN   |
			I2C_MCS_START );

	while (bufferSize--) {
		//Wait till microcontroller goes in idle mode
		while (I2C3_MCS_R&I2C_MCS_BUSY) {};
			
		if (I2C3_MCS_R&I2C_MCS_ERROR) {
			if (I2C3_MCS_R&I2C_MCS_ARBLST) I2C3_MCS_R = I2C_MCS_STOP;
			return I2CERROR;
		}
		
		//Load data
		I2C3_MDR_R  = *bytes++;
		
		//Check if it is the last byte of data
		if (bufferSize == 1) break;
		
		//Send -> Continue I2C transmission
		I2C3_MCS_R =  I2C_MCS_RUN;
	}
	
	//Send -> Stop I2C transmission
	I2C3_MCS_R =  ( I2C_MCS_RUN  |
	                I2C_MCS_STOP );
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};
		
	if (I2C3_MCS_R&I2C_MCS_ERROR) return I2CERROR;
	
	return I2COK;
}

//-------------------I2C read bytes------------------------
//This function recive 1 byte via I2C transmission with specifed address
//
//
//Function has following arguments:
//-> "buffer" is the data buffer into which the data will be loaded
//-> "command" is the buffer with command for slave device
//-> "commandSize" is the size of command buffer
//-> "address" is an address of specific master/slave device
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_ReadByte(unsigned char*      buffer,
								unsigned char*	   command,
								unsigned long	   commandSize,
				unsigned const char address)  
{
	//Send command to slave device
	I2C_StartTransmission(*command, address);
	while(--commandSize)
		I2C_TransmitByte(*(++command));
	mLow_transmission = 0;
	
	//Start I2C read for master device
	I2C3_MSA_R  =  address | 0x01;
	//Start -> Read -> Stop I2C transmission
	I2C3_MCS_R  =  ( I2C_MCS_RUN   |
					 I2C_MCS_START |
			 I2C_MCS_STOP);
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {
	};
	
	//Check for Errors
	if (I2C3_MCS_R&I2C_MCS_ERROR) return I2CERROR;
		
	//Store data in buffer
	*buffer = I2C3_MDR_R&0xFF;
		
	return I2COK;
}

//-------------------I2C read bytes------------------------
//This function read bytes via I2C transmission with specifed address
//
//
//Function has following arguments:
//-> "buffer" is the data buffer into which the data will be loaded
//-> "bufferSize"
//-> "command" is the buffer with command for slave device
//-> "commandSize" is the size of command buffer
//-> "address" is an address of specific master/slave device
//
//Function has following outputs:
//-> "TransmissionStatus" enum value representing status of I2C transmission.
//Returns 0 (I2COK) if all is OK or returns (I2CERROR) error number
TransmissionStatus I2C_ReadBytes(unsigned char*       buffer,
								 unsigned long		 bufferSize,
								 unsigned char*		 command,
								 unsigned long		 commandSize,
				    unsigned const char  address)	
{int i;
	//Send command to slave device
	I2C_StartTransmission(*command, address);
	while(--commandSize)
		I2C_TransmitByte(*(++command));
	mLow_transmission = 0;
	
	//Start I2C read for master device
	I2C3_MSA_R  =  address | 0x01;
	//Start -> Read -> Continue I2C transmission
	I2C3_MCS_R  =  ( I2C_MCS_RUN   |
					 I2C_MCS_START |
			 I2C_MCS_ACK   );
	for (i = 0; i < bufferSize; i++) {
		
		//Wait till microcontroller goes in idle mode
		while (I2C3_MCS_R&I2C_MCS_BUSY) {
		};
			
		
		if (I2C3_MCS_R&I2C_MCS_ERROR) {
			if (I2C3_MCS_R&I2C_MCS_ARBLST) I2C3_MCS_R = I2C_MCS_STOP;
			return I2CERROR;
		}
		
		//Store data in buffer
		buffer[i] = I2C3_MDR_R&0xFF;
		
		//Check if it is the last byte of data
		if (i+2 >= bufferSize) break;
		
		//Read -> Continue I2C transmission
		I2C3_MCS_R  =  ( I2C_MCS_RUN   |
				 I2C_MCS_ACK   );
	}
	
	//Read -> Stop I2C transmission
	I2C3_MCS_R =  ( I2C_MCS_RUN  |
	                I2C_MCS_STOP );
	
	//Wait till microcontroller goes in idle mode
	while (I2C3_MCS_R&I2C_MCS_BUSY) {};
		
	if (I2C3_MCS_R&I2C_MCS_ERROR) return I2CERROR;
		
	//Store last byte in buffer
	buffer[i+1] = I2C3_MDR_R&0xFF;
	
	return I2COK;
}

