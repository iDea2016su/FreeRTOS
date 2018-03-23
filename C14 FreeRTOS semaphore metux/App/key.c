#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
								    
 
void KEY_Init(void)  
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //上拉
 	GPIO_Init(GPIOA, &GPIO_InitStructure); 


}

u8 KEY_Scan(u8 mode)
{	 	  
	if(KEY0==0||KEY1==0||KEY2==0||KEY3==0)
	{
		delay_ms(10);//去抖动 
		if(KEY0==0)return KEY0_PRES;
		else if(KEY1==0)return KEY1_PRES;
		else if(KEY2==0)return KEY2_PRES;
	}    
 	return 0;// 无按键按下
}
