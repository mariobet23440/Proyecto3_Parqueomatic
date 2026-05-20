/*
 * sevseg.h
 *
 *  Created on: 14 may 2026
 *      Author: Admin
 */

#ifndef INC_SEVSEG_H_
#define INC_SEVSEG_H_

#include "main.h"

void SevSeg_UpdateValue(uint16_t val);
void SevSeg_Multiplex_ISR(void); // Llamar en el callback del Timer 4

#endif /* INC_SEVSEG_H_ */
