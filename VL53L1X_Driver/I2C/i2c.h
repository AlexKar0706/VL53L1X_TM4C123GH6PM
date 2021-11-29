#ifndef I2C_H
#define I2C_H

#define I2C_WITH_PLL 1
#define I2C_WITHOUT_PLL 0

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _I2C_MCS_R_Err_Stat  { I2COK, I2CERROR } TransmissionStatus;

TransmissionStatus I2C_StartTransmission(unsigned const char byte,
                                          unsigned const char address);

TransmissionStatus I2C_TransmitByte(unsigned const char byte);

TransmissionStatus I2C_ReciveByte(unsigned char* buffer);

TransmissionStatus I2C_StopTransmission(void);

TransmissionStatus I2C_SendByte(unsigned const char byte, 
				 unsigned const char address);

TransmissionStatus I2C_SendBytes (unsigned const char* buffer,
                                  unsigned long        bufferSize,
                                  unsigned const char  address);
																	
TransmissionStatus I2C_ReadByte (unsigned char*       buffer,
								 unsigned char*	      command,
								 unsigned long	      commandSize,
                                 unsigned const char  address);

TransmissionStatus I2C_ReadBytes (unsigned char*       buffer,
								  unsigned long		   bufferSize,
								  unsigned char*	   command,
								  unsigned long		   commandSize,
                                  unsigned const char  address);

void I2C_Init(unsigned short status);

#ifdef __cplusplus
}
#endif

#endif
