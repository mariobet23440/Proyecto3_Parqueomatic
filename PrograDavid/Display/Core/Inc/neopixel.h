/*
 * neopixel.h
 *
 *  Created on: 14 may 2026
 *      Author: Admin
 */

#ifndef INC_NEOPIXEL_H_
#define INC_NEOPIXEL_H_

#include "main.h"

#define MAX_LED 4

void NeoPixel_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void NeoPixel_SetColor(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b);
void NeoPixel_Show(void);

#endif /* INC_NEOPIXEL_H_ */
