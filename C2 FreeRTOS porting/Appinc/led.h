#pragma once
#include "sys.h"

#define LED0 PAout(12) 
#define LED1 PAout(11) 
#define LED2 PAout(8) 
#define LED3 PCout(9) 

void LED_Init(void);