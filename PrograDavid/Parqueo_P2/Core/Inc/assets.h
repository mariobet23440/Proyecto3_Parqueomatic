/*
 * assets.h
 *
 *  Created on: 4 may 2026
 *      Author: Admin
 */

#ifndef INC_ASSETS_H_
#define INC_ASSETS_H_

#include <stdint.h>
#include <stdio.h>   // <-- Agrega esta para sprintf
#include <string.h>  // <-- Agrega esta para strlen

extern const uint16_t CAR_BD[880];
#define CAR_BD_WIDTH 22
#define CAR_BD_HEIGHT 40

extern const uint16_t CAR_BU[880];
#define CAR_BU_WIDTH 22
#define CAR_BU_HEIGHT 40

extern const uint16_t CAR_GD[616];
#define CAR_GD_WIDTH 22
#define CAR_GD_HEIGHT 28

extern const uint16_t CAR_GU[616];
#define CAR_GU_WIDTH 22
#define CAR_GU_HEIGHT 28

extern const uint16_t CAR_OD[880];
#define CAR_OD_WIDTH 22
#define CAR_OD_HEIGHT 40

extern const uint16_t CAR_OU[880];
#define CAR_OU_WIDTH 22
#define CAR_OU_HEIGHT 40

extern const uint16_t CAR_RD[960];
#define CAR_RD_WIDTH 24
#define CAR_RD_HEIGHT 40

extern const uint16_t CAR_RU[960];
#define CAR_RU_WIDTH 24
#define CAR_RU_HEIGHT 40

extern const uint16_t Parqueo[76800];
#define Parqueo_WIDTH 320
#define Parqueo_HEIGHT 240



#endif /* INC_ASSETS_H_ */
