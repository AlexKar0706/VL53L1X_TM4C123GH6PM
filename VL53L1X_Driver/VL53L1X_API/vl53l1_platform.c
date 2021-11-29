
/* 
* This file is part of VL53L1 Platform 
* 
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved 
* 
* License terms: BSD 3-clause "New" or "Revised" License. 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this 
* list of conditions and the following disclaimer. 
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution. 
* 
* 3. Neither the name of the copyright holder nor the names of its contributors 
* may be used to endorse or promote products derived from this software 
* without specific prior written permission. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
* 
*/

#include "vl53l1_platform.h"
#include "vl53l1_types.h"
#include "../I2C/i2c.h"
#include <string.h>
#include <time.h>
#include <math.h>

uint8_t buffer[10];

static void Load_Buffer(uint16_t index, uint32_t data, size_t dataSize) { int i;
	buffer[0] = (index&0xFF00)>>8;
	buffer[1] = index&0x00FF;
	for (i = 0; i < dataSize; i++)
		buffer[i+2] = (data&(0xFF<<(i*8))) >> (i*8);
};

static void Clear_Buffer() {int i;
	for (i = 0; i < 10; i++) buffer[i] = 0;
};

int8_t VL53L1_WriteMulti( uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
	int8_t status = 0;
	Load_Buffer(index, 0, 0);
	status |= I2C_StartTransmission(buffer[0], dev);
	status |= I2C_TransmitByte(buffer[1]);
	while (count--)
		status |= I2C_TransmitByte(*pdata++);
	status |= I2C_StopTransmission(); 
	Clear_Buffer();
	return status;
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count){
	int8_t status = 0;
	Load_Buffer(index, 0, 0);
	status = I2C_ReadBytes(pdata, count, buffer, 2, dev);
	Clear_Buffer();
	return status;
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data) {
	int8_t status = 0;
	Load_Buffer(index, data, sizeof(data));
	status = I2C_SendBytes(buffer, 3, dev);
	Clear_Buffer();
	return status;
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data) {
	int8_t status = 0;
	Load_Buffer(index, data, sizeof(data));
	status = I2C_SendBytes(buffer, 4, dev);
	Clear_Buffer();
	return status;
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data) {
	int8_t status = 0;
	Load_Buffer(index, data, sizeof(data));
	status = I2C_SendBytes(buffer, 6, dev);
	Clear_Buffer();
	return status;
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data) {
	int8_t status = 0;
	Load_Buffer(index, 0, 0);
	status = I2C_ReadByte(data, buffer, 2, dev);
	Clear_Buffer();
	return status;
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data) {
	int8_t status = 0;
	uint8_t tempBuffer[2];
	Load_Buffer(index, 0, 0);
	status = I2C_ReadBytes(tempBuffer, 2, buffer, 2, dev);
	*data = ((tempBuffer[1]&0xFF) << 8) + (tempBuffer[0]&0xFF);
	Clear_Buffer();
	return status;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data) { int i;
	int8_t status = 0;
	uint8_t tempBuffer[4];
	Load_Buffer(index, 0, 0);
	status = I2C_ReadBytes(tempBuffer, 4, buffer, 2, dev);
	for (i = 0; i < 4; i++) 
		*data += ((tempBuffer[i]&0xFF) << (i*8));
	Clear_Buffer();
	return status;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms){
	unsigned int ms;
	while (wait_ms--) {
		ms = 2667;
		while (ms--) {}
	}
	return 0;
}
