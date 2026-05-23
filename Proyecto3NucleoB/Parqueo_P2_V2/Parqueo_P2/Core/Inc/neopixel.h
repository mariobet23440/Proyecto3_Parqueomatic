/*
 * neopixel.h
 *
 *  Created on: 14 may 2026
 *      Author: Admin
 */

#ifndef INC_NEOPIXEL_H_
#define INC_NEOPIXEL_H_

#include "main.h"
#include "math.h"

// Numero de LEDs
#define numPixels 4

// Ancho de pulso de 0 y 1
#define CCR_0 34
#define CCR_1 67

// Timer y Canal
extern TIM_HandleTypeDef htim8;
#define neoPixel_timer htim8
#define neoPixel_canal TIM_CHANNEL_3

// Brillo con Gamma
#define GAMMA_CORRECTION 2.2f // Correción adaptada a ojo humano
#define GAMMA 2.2f // Corrección de gamma
#define MAX_BRIGHTNESS 255 // Valor máximo de brillo

// Funciones de manejo de Neopixel
void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void setBrightness(uint8_t b);
void pixelShow(void);
void pixelClear(void);

uint8_t Gamma_correccion(uint8_t color, float brillo_);

#endif /* INC_NEOPIXEL_H_ */
