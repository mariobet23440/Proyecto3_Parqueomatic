/*
 * LIBRERÍA PARA DISPLAY DE SIETE SEGMENTOS
 * AUTOR: Mario Alejandro Betancourt Franco
 * NOTAS: Adaptada para el HAL de STM32
 */

#include "SevenSegmentLib.h"

/*
 Bit order: g f e d c b a
 1 = segmento encendido
 0 = segmento apagado
*/

// LOOK UP TABLE (Tabla de búsqueda)
static const uint8_t SevenSegLUT[16] = {
    0b00111111, // 0
    0b00000110, // 1
    0b11011011, // 2
    0b11001111, // 3
    0b11100110, // 4
    0b11101101, // 5
    0b11111101, // 6
    0b00000111, // 7
    0b11111111, // 8
    0b11101111, // 9
    0b11110111, // A
    0b11111100, // b
    0b10111001, // C
    0b11011110, // d
    0b11111001, // E
    0b11110001  // F
};

// Función inline adaptada al HAL
static inline void setSeg(GPIO_TypeDef *port, uint16_t pin, uint8_t on)
{
	// El HAL usa GPIO_PIN_SET (1) y GPIO_PIN_RESET (0)
	HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Función para mostrar números en el display
void DisplayNumber(D7S *d, uint8_t number)
{
	uint8_t seg = SevenSegLUT[number & 0x0F]; // & 0x0F protege contra desbordamientos

	setSeg(d->port_a, d->pin_a, seg & 0x01);
	setSeg(d->port_b, d->pin_b, seg & 0x02);
	setSeg(d->port_c, d->pin_c, seg & 0x04);
	setSeg(d->port_d, d->pin_d, seg & 0x08);
	setSeg(d->port_e, d->pin_e, seg & 0x10);
	setSeg(d->port_f, d->pin_f, seg & 0x20);
	setSeg(d->port_g, d->pin_g, seg & 0x40);
}

void DisplayOff(D7S *d)
{
	setSeg(d->port_a, d->pin_a, 0);
	setSeg(d->port_b, d->pin_b, 0);
	setSeg(d->port_c, d->pin_c, 0);
	setSeg(d->port_d, d->pin_d, 0);
	setSeg(d->port_e, d->pin_e, 0);
	setSeg(d->port_f, d->pin_f, 0);
	setSeg(d->port_g, d->pin_g, 0);
}

/**
  * @brief  Ejecuta una secuencia de prueba para identificar y mapear los segmentos del display.
  * Enciende todos los LEDs por 1s y luego hace un barrido individual de la A a la G.
  * @param  d: Puntero a la estructura del display de 7 segmentos.
  * @retval None
  */
void Display_TestSequence(D7S *d)
{
    // 1. INICIO: Encender TODOS los segmentos simultáneamente
    HAL_GPIO_WritePin(d->port_a, d->pin_a, GPIO_PIN_SET);
    HAL_GPIO_WritePin(d->port_b, d->pin_b, GPIO_PIN_SET);
    HAL_GPIO_WritePin(d->port_c, d->pin_c, GPIO_PIN_SET);
    HAL_GPIO_WritePin(d->port_d, d->pin_d, GPIO_PIN_SET);
    HAL_GPIO_WritePin(d->port_e, d->pin_e, GPIO_PIN_SET);
    HAL_GPIO_WritePin(d->port_f, d->pin_f, GPIO_PIN_SET);
    HAL_GPIO_WritePin(d->port_g, d->pin_g, GPIO_PIN_SET);
    HAL_Delay(1000); // Pausa de 1 segundo con todos encendidos

    // Apagar todos para iniciar el barrido individual
    HAL_GPIO_WritePin(d->port_a, d->pin_a, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->port_b, d->pin_b, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->port_c, d->pin_c, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->port_d, d->pin_d, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->port_e, d->pin_e, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->port_f, d->pin_f, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->port_g, d->pin_g, GPIO_PIN_RESET);
    HAL_Delay(1000);

    // 2. SECUENCIA UNO A UNO: Enciende cada segmento por 1 segundo y lo apaga

    // Segmento A
    HAL_GPIO_WritePin(d->port_a, d->pin_a, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_a, d->pin_a, GPIO_PIN_RESET);

    // Segmento B
    HAL_GPIO_WritePin(d->port_b, d->pin_b, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_b, d->pin_b, GPIO_PIN_RESET);

    // Segmento C
    HAL_GPIO_WritePin(d->port_c, d->pin_c, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_c, d->pin_c, GPIO_PIN_RESET);

    // Segmento D
    HAL_GPIO_WritePin(d->port_d, d->pin_d, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_d, d->pin_d, GPIO_PIN_RESET);

    // Segmento E
    HAL_GPIO_WritePin(d->port_e, d->pin_e, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_e, d->pin_e, GPIO_PIN_RESET);

    // Segmento F
    HAL_GPIO_WritePin(d->port_f, d->pin_f, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_f, d->pin_f, GPIO_PIN_RESET);

    // Segmento G
    HAL_GPIO_WritePin(d->port_g, d->pin_g, GPIO_PIN_SET); HAL_Delay(1000);
    HAL_GPIO_WritePin(d->port_g, d->pin_g, GPIO_PIN_RESET);
}
