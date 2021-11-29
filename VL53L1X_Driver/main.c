#include <stdint.h>
#include "../tm4c123gh6pm.h"
#include "I2C/i2c.h"
#include "SSD1306/SSD1306.h"
#include "VL53L1X_API/VL53L1X_api.h"

void Delay(uint16_t ms) {
	uint16_t time;
	while (ms--) {
		time = 2667;
		while (time--);
	}
}

uint16_t GetDistance() {
	uint8_t dataReady = 0;
	uint16_t distance = 0;
	while (dataReady == 0)
		VL53L1X_CheckForDataReady(0x52, &dataReady);
	VL53L1X_GetDistance(0x52, &distance);
	VL53L1X_ClearInterrupt(0x52);
	return distance;
}

void ClearBuff (char* buffer, size_t size) {int i;
	for(i = 0; i < size; i++)
		buffer[i] = '\0';
}

void Convert(uint16_t num, char* buffer) {
	int i = 3;
	while (i >= 0) {
		if (num) {
			buffer[i--] = num%10 + '0';
			num /= 10;
		} else buffer[i--] = ' ';
	}
}

void Boot() {
	uint8_t bootState = 0;
	I2C_Init(I2C_WITHOUT_PLL);
	SSD1306_Init();
	SSD1306_SetCursor(1, 3);
	SSD1306_WriteStr("Waiting for VL53L1X");
	
	while (bootState == 0) {
		VL53L1X_BootState(0x52, &bootState);
		Delay(100);
	}
	
	SSD1306_ClearDisplay();
	SSD1306_SetCursor(0, 3);
	SSD1306_WriteStr("VL53L1X is setting up");
	Delay(1000);
	VL53L1X_SensorInit(0x52);
	VL53L1X_SetTimingBudgetInMs(0x52, 200);
	VL53L1X_SetDistanceMode(0x52, 2);
	VL53L1X_StartRanging(0x52);
}

int main() {
	uint16_t distance = 0;
	char buffer[4];
	Boot();
	while (1) {
		SSD1306_ClearDisplay();
		ClearBuff(buffer, sizeof(buffer));
		distance = GetDistance();
		Convert(distance, buffer);
		SSD1306_SetCursor(3, 3);
		SSD1306_WriteStr(buffer);
		SSD1306_WriteStr(" mm");
		Delay(100);
	}
}
