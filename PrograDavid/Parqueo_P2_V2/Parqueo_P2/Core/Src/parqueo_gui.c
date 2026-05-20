#include "parqueo_gui.h"
#include "ili9341.h"
#include "assets.h"
#include <stdio.h>

// --- CONFIGURACIÓN DE COORDENADAS ---
// Panel lateral disponible: X[165, 319], Y[0, 239]
// Centro del panel en X = 165 + (154 / 2) = 242
#define UI_PANEL_START_X 165
#define UI_CENTER_X      242

// Definición de colores adicionales si no están en ili9341.h
#define COLOR_GREY 0x7BEF

// Estructura de coordenadas: {X_inicio, Y_inicio, Ancho, Alto}
// Basado en tus mediciones exactas
static const uint16_t coords_cajones[8][4] = {
    {21, 52, 30, 43}, {55, 52, 30, 43}, {89, 52, 30, 43}, {123, 52, 30, 43}, // A1, A2, A3, A4
    {21, 145, 30, 43}, {55, 145, 30, 43}, {89, 145, 30, 43}, {123, 145, 30, 43} // B1, B2, B3, B4
};

static uint8_t estado_anterior[8] = {0,0,0,0,0,0,0,0};
static uint8_t libres_anterior = 255; // Para detectar cambios en el conteo total
static uint8_t primer_dibujo = 1;

/**
 * @brief Redibuja una sección específica del fondo (Parqueo) para borrar elementos.
 */
void RedibujarFondo(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    LCD_CS_L();
    SetWindows(x, y, x + w - 1, y + h - 1);
    LCD_RS_H();
    for(uint16_t j = 0; j < h; j++) {
        for(uint16_t i = 0; i < w; i++) {
            // Se calcula el índice en el arreglo unidimensional del fondo 320x240
            uint16_t pixel = Parqueo[(y + j) * 320 + (x + i)];
            LCD_DATA(pixel >> 8);
            LCD_DATA(pixel & 0xFF);
        }
    }
    LCD_CS_H();
}

// --- FUNCIÓN PARA NÚMEROS GRANDES (Respaldo si LCD_Print falla) ---
// Esta función dibuja el número usando rectángulos, lo cual es más robusto
void GUI_DibujarNumeroGigante(int num, uint16_t x_centrado, uint16_t y, uint8_t size, uint16_t color, uint16_t bg) {
    char buf[2];
    sprintf(buf, "%d", num);

    // Si tu librería tiene problemas con tamaños > 2,
    // asegúrate de que el X no empuje el texto fuera de 319.
    // Para un solo número en tamaño 6, el ancho es ~36px.
    // Intentemos con tamaño 5 que suele ser el límite estable.
    if (size > 5) size = 5;

    LCD_Print(buf, x_centrado, y, size, color, bg);
}

void GUI_Inicializar(void) {
    LCD_Bitmap(0, 0, 320, 240, Parqueo);

    // Línea blanca central (17, 118 a 156, 121)
    FillRectFast(17, 118, 139, 4, COLOR_WHITE);

    // Dibujar fondo estático del panel de control (Opcional: un recuadro negro)
    FillRectFast(UI_PANEL_START_X, 0, 155, 240, COLOR_GRAY);

    primer_dibujo = 1;
    libres_anterior = 255;
}

void GUI_Actualizar(uint8_t* estado_actual) {
    int libres = 0;

    for(int i = 0; i < 8; i++) {
        uint16_t x_cajon = coords_cajones[i][0];
        uint16_t y_cajon = coords_cajones[i][1];
        uint16_t w_cajon = coords_cajones[i][2];
        uint16_t h_cajon = coords_cajones[i][3];

        if(estado_actual[i] == 0) libres++;

        // Detección de cambio de estado
        if(estado_actual[i] != estado_anterior[i] || primer_dibujo) {

            // --- CÁLCULO DE POSICIÓN DEL LED ---
            uint16_t y_led;
            if (i < 4) { // Fila A (Superior)
                y_led = 108; // Debajo del cajón A, arriba de la línea blanca
            } else { // Fila B (Inferior)
                y_led = 126; // Arriba del cajón B, debajo de la línea blanca
            }

            if(estado_actual[i] == 1) { // --- ESTADO: OCUPADO ---
                // 1. Dibujar el Auto centrado en el cajón (CAR_RD es 24x40)
                // Ajustamos X+3 y Y+1 para centrarlo en el espacio de 30x43
                LCD_BitmapTransparent(x_cajon + 3, y_cajon + 1, 24, 40, CAR_RD, 0xF81F);

                // 2. Dibujar indicador Rojo (LED)
                FillRectFast(x_cajon + 7, y_led, 16, 6, COLOR_RED);
            }
            else { // --- ESTADO: LIBRE ---
                // 1. Restaurar fondo del cajón para eliminar el carro
                RedibujarFondo(x_cajon, y_cajon, w_cajon, h_cajon);

                // 2. Dibujar indicador Verde (LED)
                FillRectFast(x_cajon + 7, y_led, 16, 6, COLOR_GREEN);
            }
            estado_anterior[i] = estado_actual[i];
        }
    }

    // 2. ACTUALIZAR PANEL DE TEXTO (Solo si el número de libres cambió)
    if (libres != libres_anterior || primer_dibujo) {

        // Limpiar área del contador para evitar sobreescritura (fantasmas)
        FillRectFast(UI_PANEL_START_X + 5, 40, 145, 120, COLOR_GRAY);

        // --- ETIQUETA "LIBRES" ---
        // Tamaño 2: cada letra mide aprox 12px de ancho. 6 letras * 12 = 72px.
        // Centro X: 242 - (72/2) = 206
        LCD_Print("LIBRES", 190, 50, 2, COLOR_BLACK, COLOR_GRAY);

        // --- NÚMERO GRANDE ---
        char buffer_num[2];
        sprintf(buffer_num, "%d", libres);

        // Tamaño 6: cada número mide aprox 36px de ancho.
        // Centro X: 242 - (36/2) = 224
        LCD_Print(buffer_num, 224, 90, 2, COLOR_RED, COLOR_GRAY);

        libres_anterior = libres;
    }

    primer_dibujo = 0;
}
