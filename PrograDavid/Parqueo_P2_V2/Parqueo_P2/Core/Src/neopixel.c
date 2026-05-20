/*
 * neopixel.c
 *
 *  Created on: 14 may 2026
 *      Author: Admin
 */

#include "Neopixel.h"
#include "main.h"
#include <math.h>

// Matríz para almacenar datos del # led y RGB color
uint8_t LED_Data[numPixels][4];

// Matríz para brillo por LED
uint8_t LED_Mod[numPixels][4];

// Buffer para PWM
uint16_t pwmData[(24 * numPixels) + 50];

uint8_t datasentflag = 0;

extern float brilloled;

//Cargar un RGB_Color deseado al LED deseado
void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b){
	LED_Data[n][0] = n;
	LED_Data[n][1] = g;
	LED_Data[n][2] = r;
	LED_Data[n][3] = b;
}

void setBrightness(uint8_t b){
	if (b > 100)
		b = 100;
	brilloled = b;
	for (int i = 0; i < numPixels; i++){
		LED_Mod[i][0] = LED_Data[i][0];
		for (uint8_t j = 1; j < 4; j++){
			// Aplicar corrección de gamma y brillo
			int8_t brill = Gamma_correccion(LED_Data[i][j], brilloled);
			LED_Mod[i][j] = brill;
		}
	}
}

void pixelShow(void){
	uint32_t RGB_Color = 0;
	uint32_t indx = 0;

	for (int i = 0; i < numPixels; i++){
		LED_Mod[i][0] = LED_Data[i][0];
		for (uint8_t j = 1; j < 4; j++){
			// Aplicar corrección de gamma y brillo
			int8_t brill = Gamma_correccion(LED_Data[i][j], brilloled);
			LED_Mod[i][j] = brill;
		}

		RGB_Color = ((LED_Mod[i][1] << 16) | (LED_Mod[i][2] << 8) | (LED_Mod[i][3]));

		for (int i = 23; i >= 0; i--){
			if (RGB_Color & (1<<i)){
				pwmData[indx] = CCR_1; // Escribir 1
			}else
				pwmData[indx] = CCR_0; // Escribir 0
			indx++;
		}
	}
	for (int i = 0; i < 50; i++){
		pwmData[indx] = 0;
		indx++;
	}

	// Enviar todos los datos por el PWM
	HAL_TIM_PWM_Start_DMA(&neoPixel_timer, neoPixel_canal, (uint32_t*) pwmData, indx);

	// Esperar a que se envíen todos los datos
	while (!datasentflag);
	datasentflag = 0;
}

void pixelClear(void){
	for (uint8_t i = 0; i < numPixels; i++){
		LED_Data[i][0] = i;
		LED_Data[i][1] = 0;
		LED_Data[i][2] = 0;
		LED_Data[i][3] = 0;
	}
}

uint8_t Gamma_correccion(uint8_t c, float b){
	//Limitar el brillo de 0 a 100
	if (b > 100.0f)
		b = 100.0f;
	if (b < 0.0f)
		b = 0.0f;

	// Normalizar el brillo al rango de 0.0 a 1.0
	float brillo_Factor = b / 100.0f;

	// Aplicar el factor
	float adjustedColor = c * brillo_Factor;

	// Fórmula de escalamiento gamma
	float gammaCorrectedColor = pow((adjustedColor / 255.0f), GAMMA) * 255.0f;

	// Analiza que esté dentro de límites
	if (gammaCorrectedColor > 255.0f)
		gammaCorrectedColor = 255.0f;
	if (gammaCorrectedColor < 0.0f)
		gammaCorrectedColor = 0.0f;

	// Regresar como 8 bit escalado
	return (uint8_t) gammaCorrectedColor;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	HAL_TIM_PWM_Stop_DMA(&neoPixel_timer, neoPixel_canal);
	datasentflag = 1;
}
