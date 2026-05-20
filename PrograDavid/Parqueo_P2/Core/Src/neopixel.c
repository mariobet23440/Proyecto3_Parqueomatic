/*
 * neopixel.c
 *
 *  Created on: 14 may 2026
 *      Author: Admin
 */

#include "neopixel.h"

// Variables para el Timer
static TIM_HandleTypeDef *pwm_timer;
static uint32_t pwm_channel;

// Buffer de colores RGB (8 LEDs x 3 colores)
static uint8_t LED_Data[MAX_LED][3];

// Buffer DMA: 24 bits por LED + 50 ciclos de Reset al final
static uint32_t pwmData[(24 * MAX_LED) + 50];

void NeoPixel_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    pwm_timer = htim;
    pwm_channel = channel;
}

void NeoPixel_SetColor(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b) {
    if(led_index < MAX_LED) {
        // Los WS2812B usan el orden Verde, Rojo, Azul (GRB)
        LED_Data[led_index][0] = g;
        LED_Data[led_index][1] = r;
        LED_Data[led_index][2] = b;
    }
}

void NeoPixel_Show(void) {
    uint32_t indx = 0;
    uint32_t color;

    for (int i = 0; i < MAX_LED; i++) {
        color = ((LED_Data[i][0] << 16) | (LED_Data[i][1] << 8) | (LED_Data[i][2]));

        for (int i = 23; i >= 0; i--) {
            if (color & (1 << i)) {
                pwmData[indx] = 68;  // ~64% de 104 (Lógico 1)
            } else {
                pwmData[indx] = 33;  // ~32% de 104 (Lógico 0)
            }
            indx++;
        }
    }

    // Rellenar el reset con ceros
    for (int i = 0; i < 50; i++) {
        pwmData[indx] = 0;
        indx++;
    }

    HAL_TIM_PWM_Start_DMA(pwm_timer, pwm_channel, (uint32_t *)pwmData, indx);
}
