#include "led.h"

void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PE端口时钟
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PE端口时钟
	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_11|GPIO_Pin_8;	    		  
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				  
 GPIO_SetBits(GPIOA,GPIO_Pin_12); 						 
 GPIO_SetBits(GPIOA,GPIO_Pin_11); 	
 GPIO_SetBits(GPIOA,GPIO_Pin_8); 	
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	    		  
 GPIO_Init(GPIOC, &GPIO_InitStructure);	  				  
 GPIO_SetBits(GPIOC,GPIO_Pin_9); 
}
 
