/*
 * sevseg.c
 *
 *  Created on: 14 may 2026
 *      Author: Admin
 */

#include "sevseg.h"

static uint16_t display_value = 0;
static uint8_t current_digit = 0;

/* * MAPA PARA ÁNODO COMÚN
 * Los bits son: [G F E D C B A]
 * 0 significa ENCENDIDO, 1 significa APAGADO
 */
const uint8_t digit_map_CA[10] = {
    0x40, // 0 -> 1000000 (Solo G apagado)
    0x79, // 1 -> 1111001 (A,D,E,F,G apagados)
    0x24, // 2
    0x30, // 3
    0x19, // 4
    0x12, // 5
    0x02, // 6
    0x78, // 7
    0x00, // 8 -> 0000000 (Todos encendidos)
    0x10  // 9
};

void SevSeg_UpdateValue(uint16_t val) {
    display_value = val;
}

void SevSeg_Multiplex_ISR(void) {
    // 1. Apagar todos los dígitos para evitar "efecto fantasma"
    // Ponemos PC0-PC3 en LOW (0V). Al ser Ánodo Común, esto corta el flujo.
    GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3) << 16;

    // 2. Obtener el número actual
    uint8_t num = 0;
    if (current_digit == 0) num = display_value % 10;
    else if (current_digit == 1) num = (display_value / 10) % 10;
    else if (current_digit == 2) num = (display_value / 100) % 10;
    else if (current_digit == 3) num = (display_value / 1000) % 10;

    // 3. Lógica de Segmentos con re-mapeo de PB3 a PB7
    uint8_t segmentos = digit_map_CA[num];

    // Filtramos bits 0, 1, 2 (A, B, C) y bits 4, 5, 6 (E, F, G)
    uint16_t salida_odr = (segmentos & 0x07) | (segmentos & 0x70);

    // Manejo especial del Segmento D (Bit 3 del mapa)
    // Si el bit 3 está en 1 (apagado en CA), ponemos PB7 en HIGH.
    // Si el bit 3 está en 0 (encendido en CA), PB7 se queda en LOW.
    if (segmentos & 0x08) {
        salida_odr |= GPIO_PIN_7;
    }

    // Escribimos en el puerto B sin afectar PB8 y PB9 (I2C)
    // La máscara 0x00F7 limpia los bits 0,1,2,4,5,6,7
    GPIOB->ODR = (GPIOB->ODR & ~0x00F7) | salida_odr;

    // 4. Activar el dígito correspondiente (HIGH para suministrar energía)
    if (current_digit == 0) GPIOC->BSRR = GPIO_PIN_0; // Unidades
    if (current_digit == 1) GPIOC->BSRR = GPIO_PIN_1; // Decenas
    if (current_digit == 2) GPIOC->BSRR = GPIO_PIN_2; // Centenas
    if (current_digit == 3) GPIOC->BSRR = GPIO_PIN_3; // Millares

    current_digit++;
    if (current_digit > 3) current_digit = 0;
}
