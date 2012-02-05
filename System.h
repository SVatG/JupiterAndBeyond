#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "stm32f4xx.h"

typedef void InterruptHandler();

void SystemInit();

void InstallInterruptHandler(IRQn_Type interrupt,InterruptHandler handler);
void RemoveInterruptHandler(IRQn_Type interrupt,InterruptHandler handler);

#endif
