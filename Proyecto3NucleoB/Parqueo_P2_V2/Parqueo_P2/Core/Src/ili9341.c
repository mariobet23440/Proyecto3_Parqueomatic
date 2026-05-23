/*
 * ili9341.c
 *
 *  Created on: Aug 20, 2024
 *      Author: Pablo Mazariegos
 */
#include <stdlib.h> // malloc()
#include <string.h> // memset()
#include <stdbool.h>
#include "pgmspace.h"
#include "ili9341.h"
#include "main.h"

/*  RST - PC1
 *  RD  - PA0
 *  WR  - PA1
 *  RS  - PA4
 *  CS  - PB0
 *
 *  D0 - PA9
 *  D1 - PC7
 *  D2 - PA10
 *  D3 - PB3
 *  D4 - PB5
 *  D5 - PB4
 *  D6 - PB10
 *  D7 - PA8
 * */



typedef struct {
	uint32_t bsrrA;  // valores para GPIOA->BSRR
	uint32_t bsrrB;  // valores para GPIOB->BSRR
	uint32_t bsrrC;  // valores para GPIOC->BSRR
} GPIO_Lookup;

GPIO_Lookup busLookup[256];

// ——— Máscaras del bus de datos ———
static const uint32_t DATA_PA = LCD_D0_Pin | LCD_D2_Pin | LCD_D7_Pin;
static const uint32_t DATA_PB = LCD_D3_Pin | LCD_D4_Pin | LCD_D5_Pin
		| LCD_D6_Pin;
static const uint32_t DATA_PC = LCD_D1_Pin;

extern const uint8_t smallFont[1140];
extern const uint16_t bigFont[1520];

// Definir color transparente (ejemplo: magenta 0xF81F)
#define TRANSPARENT_COLOR 0xF81F

typedef struct {
	const uint16_t *bitmap;
	uint32_t *transparent_mask;  // Bitmask para píxeles transparentes
	unsigned int width;
	unsigned int height;
} TransparentBitmap;

//***************************************************************************************************************************************
// Función para inicializar tabla para pines
//***************************************************************************************************************************************
void initLookup() {
	for (int val = 0; val < 256; val++) {
		GPIO_Lookup entry = { 0, 0, 0 };

		// D0 -> PA9
		if (val & 0x01)
			entry.bsrrA |= (1 << 9);          // set
		else
			entry.bsrrA |= (1 << (9 + 16));   // reset
		// D1 -> PC7
		if (val & 0x02)
			entry.bsrrC |= (1 << 7);
		else
			entry.bsrrC |= (1 << (7 + 16));
		// D2 -> PA10
		if (val & 0x04)
			entry.bsrrA |= (1 << 10);
		else
			entry.bsrrA |= (1 << (10 + 16));
		// D3 -> PB3
		if (val & 0x08)
			entry.bsrrB |= (1 << 3);
		else
			entry.bsrrB |= (1 << (3 + 16));
		// D4 -> PB5
		if (val & 0x10)
			entry.bsrrB |= (1 << 5);
		else
			entry.bsrrB |= (1 << (5 + 16));
		// D5 -> PB4
		if (val & 0x20)
			entry.bsrrB |= (1 << 4);
		else
			entry.bsrrB |= (1 << (4 + 16));
		// D6 -> PB10
		if (val & 0x40)
			entry.bsrrB |= (1 << 10);
		else
			entry.bsrrB |= (1 << (10 + 16));
		// D7 -> PA8
		if (val & 0x80)
			entry.bsrrA |= (1 << 8);
		else
			entry.bsrrA |= (1 << (8 + 16));
		busLookup[val] = entry;
	}
}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
	initLookup();
	//****************************************
	// Secuencia de Inicialización
	//****************************************
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_SET);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin | LCD_WR_Pin | LCD_RS_Pin,
			GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_RESET);

	//****************************************
	LCD_CMD(0xE9);  // SETPANELRELATED
	LCD_DATA(0x20);
	//****************************************
	LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
	HAL_Delay(100);
	//****************************************
	LCD_CMD(0xD1);    // (SETVCOM)
	LCD_DATA(0x00);
	LCD_DATA(0x71);
	LCD_DATA(0x19);
	//****************************************
	LCD_CMD(0xD0);   // (SETPOWER)
	LCD_DATA(0x07);
	LCD_DATA(0x01);
	LCD_DATA(0x08);
	//****************************************
	LCD_CMD(0x36);  // (MEMORYACCESS)
	LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
	//****************************************
	LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
	LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
	//****************************************
	LCD_CMD(0xC1);    // (POWERCONTROL2)
	LCD_DATA(0x10);
	LCD_DATA(0x10);
	LCD_DATA(0x02);
	LCD_DATA(0x02);
	//****************************************
	LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
	LCD_DATA(0x00);
	LCD_DATA(0x35);
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0x01);
	LCD_DATA(0x02);
	//****************************************
	LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
	LCD_DATA(0x04); // 72Hz
	//****************************************
	LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
	LCD_DATA(0x01);
	LCD_DATA(0x44);
	//****************************************
	LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
	LCD_DATA(0x04);
	LCD_DATA(0x67);
	LCD_DATA(0x35);
	LCD_DATA(0x04);
	LCD_DATA(0x08);
	LCD_DATA(0x06);
	LCD_DATA(0x24);
	LCD_DATA(0x01);
	LCD_DATA(0x37);
	LCD_DATA(0x40);
	LCD_DATA(0x03);
	LCD_DATA(0x10);
	LCD_DATA(0x08);
	LCD_DATA(0x80);
	LCD_DATA(0x00);
	//****************************************
	LCD_CMD(0x2A); // Set_column_address 320px (CASET)
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0x01);
	LCD_DATA(0x3F);
	//****************************************
	LCD_CMD(0x2B); // Set_page_address 240px (PASET)
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0xF0);
	//LCD_DATA(0x01);
	//LCD_DATA(0xE0);
	//  LCD_DATA(0x8F);
	LCD_CMD(0x29); //display on
	LCD_CMD(0x2C); //display on

	LCD_CMD(ILI9341_INVOFF); //Invert Off
	HAL_Delay(120);
	LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
	HAL_Delay(120);
	LCD_CMD(ILI9341_DISPON);    //Display on
	HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
	LCD_RS_L();
	LCD_WR_L();
	GPIOA->BSRR |= busLookup[cmd].bsrrA;
	GPIOB->BSRR |= busLookup[cmd].bsrrB;
	GPIOC->BSRR |= busLookup[cmd].bsrrC;
	LCD_WR_H();
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
	LCD_RS_H();
	LCD_WR_L();
	GPIOA->BSRR |= busLookup[data].bsrrA;
	GPIOB->BSRR |= busLookup[data].bsrrB;
	GPIOC->BSRR |= busLookup[data].bsrrC;
	LCD_WR_H();
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {
	LCD_CMD(0x2a); // Set_column_address 4 parameters
	LCD_DATA(x1 >> 8);
	LCD_DATA(x1);
	LCD_DATA(x2 >> 8);
	LCD_DATA(x2);
	LCD_CMD(0x2b); // Set_page_address 4 parameters
	LCD_DATA(y1 >> 8);
	LCD_DATA(y1);
	LCD_DATA(y2 >> 8);
	LCD_DATA(y2);
	LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
	unsigned int x, y;
	LCD_CMD(0x02c); // write_memory_start
	//HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	LCD_RS_H();
	LCD_CS_L();
	SetWindows(0, 0, 319, 239); // 479, 319);
	for (x = 0; x < 320; x++)
		for (y = 0; y < 240; y++) {
			LCD_DATA(c >> 8);
			LCD_DATA(c);
		}
	LCD_CS_H();
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
	unsigned int i;
	LCD_CMD(0x02c); //write_memory_start
	//HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	LCD_RS_H();
	LCD_CS_L();

	//l = l + x;
	SetWindows(x, y, l + x, y);
	//j = l; // * 2;
	for (i = 0; i < l; i++) {
		LCD_DATA(c >> 8);
		LCD_DATA(c);
	}
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	LCD_CS_H();
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
	unsigned int i;
	LCD_CMD(0x02c); //write_memory_start
	//HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	LCD_RS_H();
	LCD_CS_L();
	//l = l + y;
	SetWindows(x, y, x, y + l);
	//j = l; //* 2;
	for (i = 1; i <= l; i++) {
		LCD_DATA(c >> 8);
		LCD_DATA(c);
	}
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	LCD_CS_H();
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h,
		unsigned int c) {
	H_line(x, y, w, c);
	H_line(x, y + h, w, c);
	V_line(x, y, h, c);
	V_line(x + w, y, h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h,
		unsigned int c) {
	LCD_CMD(0x02c); // write_memory_start
	//HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	LCD_CS_L();
	SetWindows(x, y, x + w - 1, y + h - 1);
	LCD_RS_H();
	unsigned int k = w * h * 2 - 1;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			LCD_DATA(c >> 8);
			LCD_DATA(c);

			//LCD_DATA(bitmap[k]);
			k = k - 2;
		}
	}
	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	LCD_CS_H();
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRectFast(unsigned int x, unsigned int y, unsigned int w,
		unsigned int h, unsigned int c) {
	//Se prepara el dato c (color)
	uint8_t hi = c >> 8;
	uint8_t lo = c & 0xFF;

	/*
	 *  Una vez configurada la ili9341 en formato de color de 16 bits (RGB565), espera que cada píxel se le envíe
	 *  en dos bytes seguidos, primero el “byte alto” (hi) y luego el “byte bajo” (lo), por eso es que el codigo está segmentado así
	 *  Se separa el color en parte hi y parte lo, para posteriormente enviarlo con simples comparaciones y cambiando el bit correspondiente,
	 *  ahorrandonos así el uso de HAL.
	 */

	LCD_CS_L();              	// Se inicia la comunicación con la pantalla.
	SetWindows(x, y, x + w - 1, y + h - 1); // Envía el comando 0x2A, que es para definir un rango de columnas y luego 0x2B para definir un rango de filas.
	LCD_RS_H();              					// RS = 1, para ya mandar datos.

	for (unsigned int j = 0; j < h; j++) { // Recorrer cada fila del rectángulo que vamos a llenar.
		for (unsigned int i = 0; i < w; i++) { // Por cada fila, recorre cada columna.
			// ——— Byte alto ———
			LCD_WR_L(); //WR = 0 es para indicar que se le va a mandar un bus de datos
			GPIOA->ODR = (GPIOA->ODR & ~DATA_PA) //Pone en 0 los bits  D0, D2 y D7 sin alterar los demás bits del registro
					| (((hi >> 0) & 1) ? LCD_D0_Pin : 0) //Compara con los bits de la parte hi para poner en 1 o dejar en 0 el registro
					| (((hi >> 2) & 1) ? LCD_D2_Pin : 0)
					| (((hi >> 7) & 1) ? LCD_D7_Pin : 0);
			GPIOB->ODR = (GPIOB->ODR & ~DATA_PB)
					| (((hi >> 3) & 1) ? LCD_D3_Pin : 0)
					| (((hi >> 4) & 1) ? LCD_D4_Pin : 0)
					| (((hi >> 5) & 1) ? LCD_D5_Pin : 0)
					| (((hi >> 6) & 1) ? LCD_D6_Pin : 0);
			GPIOC->ODR = (GPIOC->ODR & ~DATA_PC)
					| (((hi >> 1) & 1) ? LCD_D1_Pin : 0);
			LCD_WR_H();

			// ——— Byte bajo ———
			LCD_WR_L();
			GPIOA->ODR = (GPIOA->ODR & ~DATA_PA)
					| (((lo >> 0) & 1) ? LCD_D0_Pin : 0)
					| (((lo >> 2) & 1) ? LCD_D2_Pin : 0)
					| (((lo >> 7) & 1) ? LCD_D7_Pin : 0);
			GPIOB->ODR = (GPIOB->ODR & ~DATA_PB)
					| (((lo >> 3) & 1) ? LCD_D3_Pin : 0)
					| (((lo >> 4) & 1) ? LCD_D4_Pin : 0)
					| (((lo >> 5) & 1) ? LCD_D5_Pin : 0)
					| (((lo >> 6) & 1) ? LCD_D6_Pin : 0);
			GPIOC->ODR = (GPIOC->ODR & ~DATA_PC)
					| (((lo >> 1) & 1) ? LCD_D1_Pin : 0);
			LCD_WR_H();
		}
	}
	LCD_CS_H();  // CS = 1
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(char *text, int x, int y, int fontSize, int color,
		int background) {

	int fontXSize;
	int fontYSize;

	if (fontSize == 1) {
		fontXSize = fontXSizeSmal;
		fontYSize = fontYSizeSmal;
	}
	if (fontSize == 2) {
		fontXSize = fontXSizeBig;
		fontYSize = fontYSizeBig;
	}
	if (fontSize == 3) {
		fontXSize = fontXSizeNum;
		fontYSize = fontYSizeNum;
	}

	char charInput;
	int cLength = strlen(text);
	//Serial.println(cLength, DEC);
	int charDec;
	int c;
	//int charHex;
	char char_array[cLength + 1];
	for (int i = 0; text[i] != '\0'; i++) {
		char_array[i] = text[i];
	}

	//text.toCharArray(char_array, cLength + 1);

	for (int i = 0; i < cLength; i++) {
		charInput = char_array[i];
		//Serial.println(char_array[i]);
		charDec = (int) charInput;
		LCD_CS_L();  // CS = 0
		SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1,
				y + fontYSize);
		long charHex1;
		for (int n = 0; n < fontYSize; n++) {
			if (fontSize == 1) {
				charHex1 = pgm_read_word_near(
						smallFont + ((charDec - 32) * fontYSize) + n);
			}
			if (fontSize == 2) {
				charHex1 = pgm_read_word_near(
						bigFont + ((charDec - 32) * fontYSize) + n);
			}
			for (int t = 1; t < fontXSize + 1; t++) {
				if ((charHex1 & (1 << (fontXSize - t))) > 0) {
					c = color;
				} else {
					c = background;
				}
				LCD_DATA(c >> 8);
				LCD_DATA(c);
			}
		}
		LCD_CS_H();  // CS = 1
	}
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width,
		unsigned int height, const uint16_t *bitmap) {
	LCD_CMD(0x02c); // write_memory_start
	LCD_CS_L();
	SetWindows(x, y, x + width - 1, y + height - 1);
	LCD_RS_H();
	unsigned int total_pixels = width * height;

	// Enviar datos en bloques si es posible
	for (unsigned int i = 0; i < total_pixels; i++) {
		uint16_t pixel = bitmap[i];
		LCD_DATA(pixel >> 8);
		LCD_DATA(pixel & 0xFF);
	}
	LCD_CS_H();
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits) con color transparente
//***************************************************************************************************************************************
void LCD_BitmapTransparent(uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, const uint16_t *bitmap, uint16_t transparentColor) {
	for (unsigned int j = 0; j < height; j++) {
		for (unsigned int i = 0; i < width; i++) {
			unsigned int index = j * width + i;
			uint16_t pixel = bitmap[index];

			if (pixel != transparentColor) {
				// Comandos mínimos para un solo píxel
				LCD_CS_L();
				SetWindows(x + i, y + j, x + i, y + j);
				LCD_CMD(0x02c);
				LCD_RS_H();
				LCD_DATA(pixel >> 8);
				LCD_DATA(pixel & 0xFF);
				LCD_CS_H();
			}
		}
	}
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_BitmapFast(unsigned int x, unsigned int y, unsigned int width,
		unsigned int height, const uint8_t *bitmap) {

	LCD_CS_L();
	SetWindows(x, y, x + width - 1, y + height - 1);
	LCD_RS_H();

	for (unsigned int row = 0; row < height; row++) {

		uint32_t base = row * width * 2;
		for (unsigned int col = 0; col < width; col++) {
			uint8_t hi = bitmap[base + col * 2];
			uint8_t lo = bitmap[base + col * 2 + 1];

			// ——— Byte alto ———
			LCD_WR_L();
			GPIOA->ODR = (GPIOA->ODR & ~DATA_PA)
					| ((hi & 0x01) ? LCD_D0_Pin : 0)
					| (((hi >> 2) & 0x01) ? LCD_D2_Pin : 0)
					| (((hi >> 7) & 0x01) ? LCD_D7_Pin : 0);
			GPIOB->ODR = (GPIOB->ODR & ~DATA_PB)
					| (((hi >> 3) & 0x01) ? LCD_D3_Pin : 0)
					| (((hi >> 4) & 0x01) ? LCD_D4_Pin : 0)
					| (((hi >> 5) & 0x01) ? LCD_D5_Pin : 0)
					| (((hi >> 6) & 0x01) ? LCD_D6_Pin : 0);
			GPIOC->ODR = (GPIOC->ODR & ~DATA_PC)
					| (((hi >> 1) & 0x01) ? LCD_D1_Pin : 0);
			LCD_WR_H();

			// ——— Byte bajo ———
			LCD_WR_L();
			GPIOA->ODR = (GPIOA->ODR & ~DATA_PA)
					| ((lo & 0x01) ? LCD_D0_Pin : 0)
					| (((lo >> 2) & 0x01) ? LCD_D2_Pin : 0)
					| (((lo >> 7) & 0x01) ? LCD_D7_Pin : 0);
			GPIOB->ODR = (GPIOB->ODR & ~DATA_PB)
					| (((lo >> 3) & 0x01) ? LCD_D3_Pin : 0)
					| (((lo >> 4) & 0x01) ? LCD_D4_Pin : 0)
					| (((lo >> 5) & 0x01) ? LCD_D5_Pin : 0)
					| (((lo >> 6) & 0x01) ? LCD_D6_Pin : 0);
			GPIOC->ODR = (GPIOC->ODR & ~DATA_PC)
					| (((lo >> 1) & 0x01) ? LCD_D1_Pin : 0);
			LCD_WR_H();
		}
	}

	// 3) Cierro CS
	LCD_CS_H();
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, const uint16_t *bitmap,
                int columns, int index, char flip, char offset) {
    LCD_CMD(0x02c); // write_memory_start
    LCD_RS_H();
    LCD_CS_L();

    SetWindows(x, y,  x + width - 1, y + height - 1);

    int ancho = width * columns;
    int k = 0;

    if (flip) {
        for (int j = 0; j < height; j++) {
            k = j * ancho + index * width - 1 - offset;
            k = k + width;
            for (int i = 0; i < width; i++) {
                uint16_t pixel = bitmap[k];
                LCD_DATA(pixel >> 8);
                LCD_DATA(pixel & 0xFF);
                k--;
            }
        }
    } else {
        for (int j = 0; j < height; j++) {
            k = j * ancho + index * width + 1 + offset;
            for (int i = 0; i < width; i++) {
                uint16_t pixel = bitmap[k];
                LCD_DATA(pixel >> 8);
                LCD_DATA(pixel & 0xFF);
                k++;
            }
        }
    }
    LCD_CS_H();
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Draw_Sliced_RAM(int x, int y, int w_frame, int h, uint16_t *bitmap, int total_cols, int index, uint16_t transparent_color) {

    // El Stride es el ancho total de la imagen original en el binario (ej: 312 píxeles)
    int stride = w_frame * total_cols;

    for (int j = 0; j < h; j++) {
        // Calculamos dónde empieza la fila del frame actual dentro del buffer gigante
        int base_row_frame = (j * stride) + (index * w_frame);

        int i = 0;
        while (i < w_frame) {
            uint16_t color = bitmap[base_row_frame + i];

            // Si el color es transparente, saltamos
            if (color == transparent_color) {
                i++;
                continue;
            }

            // Si no es transparente, buscamos cuántos píxeles sólidos siguen
            int start_i = i;
            int len = 0;
            while (i < w_frame && bitmap[base_row_frame + i] != transparent_color) {
                i++;
                len++;
            }

            // Dibujamos la "tira" de píxeles sólidos a máxima velocidad
            LCD_CS_L();
            SetWindows(x + start_i, y + j, x + start_i + len - 1, y + j);
            LCD_RS_H();
            for (int k = 0; k < len; k++) {
                uint16_t p = bitmap[base_row_frame + start_i + k];
                // Enviamos bytes directo al bus de 8 bits
                LCD_DATA(p >> 8);
                LCD_DATA(p & 0xFF);
            }
            LCD_CS_H();
        }
    }
}


void LCD_Sprite_Transparent(int x, int y, int width, int height,
		const uint16_t *bitmap, uint8_t columns, uint8_t index, uint8_t flip,
		uint8_t offset, uint16_t transparent_color) {
	LCD_RS_H();

	int ancho = width * columns;
	int k = 0;

	if (flip) {
		for (int j = 0; j < height; j++) {
			k = j * ancho + index * width - 1 - offset;
			k = k + width;
			for (int i = 0; i < width; i++) {
				uint16_t pixel = bitmap[k];
				// Dibujar solo si no es transparente
				if (pixel != transparent_color) {
					LCD_CS_L();
					SetWindows(x + (width - 1 - i), y + j, x + (width - 1 - i),
							y + j);
					LCD_CMD(0x02c);
					LCD_DATA(pixel >> 8);
					LCD_DATA(pixel & 0xFF);
					LCD_CS_H();
				}
				k--;
			}
		}
	} else {
		for (int j = 0; j < height; j++) {
			k = j * ancho + index * width + 1 + offset;
			for (int i = 0; i < width; i++) {
				uint16_t pixel = bitmap[k];
				// Dibujar solo si no es transparente
				if (pixel != transparent_color) {
					LCD_CS_L();
					SetWindows(x + i, y + j, x + i, y + j);
					LCD_CMD(0x02c);
					LCD_DATA(pixel >> 8);
					LCD_DATA(pixel & 0xFF);
					LCD_CS_H();
				}
				k++;
			}
		}
	}
}

// Dibuja saltando los píxeles transparentes mediante "tiras" para máxima velocidad
void LCD_Draw_Transparent_Fast(int x, int y, int w, int h, uint16_t *buffer, uint16_t trans_color) {
    for (int j = 0; j < h; j++) {
        int i = 0;
        while (i < w) {
            uint16_t pixel = buffer[j * w + i];
            if (pixel == trans_color) {
                i++;
                continue;
            }
            int start_i = i;
            int len = 0;
            while (i < w && buffer[j * w + i] != trans_color) {
                i++;
                len++;
            }
            SetWindows(x + start_i, y + j, x + start_i + len - 1, y + j);
            LCD_CMD(0x2C);
            for (int k = 0; k < len; k++) {
                uint16_t p = buffer[j * w + start_i + k];
                LCD_DATA(p >> 8);
                LCD_DATA(p & 0xFF);
            }
        }
    }
}

void LCD_Fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	FillRectFast(x, y, w, h, color);
}

// Dibuja una imagen completa en pantalla
void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* image) {
    for (uint16_t j = 0; j < height; j++) {
        for (uint16_t i = 0; i < width; i++) {
            LCD_DrawPixel(x + i, y + j, image[j * width + i]);
        }
    }
}

// Dibuja solo una sección de una imagen grande (RÁPIDO)
void LCD_DrawImageArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* image, uint16_t totalWidth) {
    LCD_CS_L();
    SetWindows(x, y, x + width - 1, y + height - 1);
    LCD_RS_H();
    for (uint16_t j = 0; j < height; j++) {
        for (uint16_t i = 0; i < width; i++) {
            uint16_t pixel = image[(y + j) * totalWidth + (x + i)];
            LCD_DATA(pixel >> 8);
            LCD_DATA(pixel & 0xFF);
        }
    }
    LCD_CS_H();
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    // 1. Usamos tu función SetWindows para decirle a la pantalla qué punto queremos
    // SetWindows ya incluye el comando 0x2C (Write Memory Start) al final
    SetWindows(x, y, x, y);

    // 2. Enviamos los 16 bits de color usando tus funciones de datos
    LCD_DATA(color >> 8);   // Byte alto
    LCD_DATA(color & 0xFF); // Byte bajo
}

