/*
 * ili9341.h
 *
 *  Created on: Aug 20, 2024
 *      Author: Pablo Mazariegos
 */

#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_

#include "lcd_registers.h"
#include "font.h"
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#define LCD_WR_PORT    GPIOA
#define LCD_WR_PIN     GPIO_PIN_1
#define LCD_WR_L()     (LCD_WR_PORT->BSRR = (LCD_WR_PIN<<16))
#define LCD_WR_H()     (LCD_WR_PORT->BSRR =  LCD_WR_PIN)

#define LCD_RS_PORT    GPIOA
#define LCD_RS_PIN     GPIO_PIN_4
#define LCD_RS_L()     (LCD_RS_PORT->BSRR = (LCD_RS_PIN<<16))	//
#define LCD_RS_H()     (LCD_RS_PORT->BSRR =  LCD_RS_PIN)		//Poner el modo RS en high es para mandar dato

#define LCD_CS_PORT    GPIOB
#define LCD_CS_PIN     GPIO_PIN_0
#define LCD_CS_L()     (LCD_CS_PORT->BSRR = (LCD_CS_PIN<<16)) 	//Pone el chip select en activo para empezar la comunicación con la pantalla
#define LCD_CS_H()     (LCD_CS_PORT->BSRR =  LCD_CS_PIN)		//

void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRectFast(unsigned int x, unsigned int y, unsigned int w,
		unsigned int h, unsigned int c);
void LCD_Print(char* text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const uint16_t *bitmap);
void LCD_BitmapTransparent(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap, uint16_t transparentColor);
void LCD_BitmapFast(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const uint8_t *bitmap);
void LCD_Sprite(int x, int y, int width, int height, const uint16_t *bitmap,
                int columns, int index, char flip, char offset);
//void LCD_Sprite2(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);
void LCD_Draw_Sliced_RAM(int x, int y, int w_frame, int h, uint16_t *bitmap, int total_cols, int index, uint16_t transparent_color);//void LCD_Sprite_Transparent(int x, int y, int width, int height, const uint16_t *bitmap, uint8_t columns, uint8_t index, uint8_t flip, uint8_t offset, uint16_t transparent_color);

// Dibuja saltando los píxeles transparentes mediante "tiras" para máxima velocidad
void LCD_Draw_Transparent_Fast(int x, int y, int w, int h, uint16_t *buffer, uint16_t trans_color);

void LCD_Fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/* Definiciones de colores en formato RGB565 */
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_MAGENTA     0xF81F
#define COLOR_CYAN        0x07FF
#define COLOR_GRAY        0x8410

// Prototipos necesarios para la GUI
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* image);
void LCD_DrawImageArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* image, uint16_t totalWidth);



#endif /* INC_ILI9341_H_ */
