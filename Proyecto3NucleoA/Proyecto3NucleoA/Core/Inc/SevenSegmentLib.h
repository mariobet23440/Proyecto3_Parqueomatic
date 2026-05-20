/*
 * SevenSegmentLib.h
 *
 * Autor: Mario Alejandro Betancourt Franco
 * Notas: Adaptada para STM32 HAL (STM32F446RE)
 * Librería optimizada con structs para evitar hardcoding.
 */

#ifndef SEVENSEGMENTLIB_H_
#define SEVENSEGMENTLIB_H_

#include <stdint.h>
#include "stm32f4xx_hal.h" // Librería HAL principal de la familia F4

// Estructura para configurar cualquier display sin importar en qué puertos estén
typedef struct {
	GPIO_TypeDef *port_a;
	GPIO_TypeDef *port_b;
	GPIO_TypeDef *port_c;
	GPIO_TypeDef *port_d;
	GPIO_TypeDef *port_e;
	GPIO_TypeDef *port_f;
	GPIO_TypeDef *port_g;

	uint16_t pin_a;
	uint16_t pin_b;
	uint16_t pin_c;
	uint16_t pin_d;
	uint16_t pin_e;
	uint16_t pin_f;
	uint16_t pin_g;
} D7S;

void DisplayNumber(D7S *d, uint8_t number);
void DisplayOff(D7S *d);
void Display_TestSequence(D7S *d);

#endif /* SEVENSEGMENTLIB_H_ */
