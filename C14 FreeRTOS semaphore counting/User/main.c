#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timer.h"
#include "exit.h"
#include "key.h"
#include "mem.h"
#include "task.h"
#include "semphr.h"

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     256        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define SemaphoreGiveTask_TASK_PRIO    2          //任务优先级
#define SemaphoreGiveTask_STK_SIZE     256        //任务堆栈大小
TaskHandle_t SemaphoreGiveTaskHanhler;        //任务句柄
void SemaphoreGiveTask(void *pArg);           //任务函数

#define SemaphoreTakeTask_TASK_PRIO    3          //任务优先级
#define SemaphoreTakeTask_STK_SIZE     256        //任务堆栈大小
TaskHandle_t SemaphoreTakeTaskHanhler;        //任务句柄
void SemaphoreTakeTask(void *pArg);           //任务函数


SemaphoreHandle_t CountSemaphore;  //计数信号量


int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	//TIM5_Int_Init(5000,7200-1);
	KEY_Init();	 
	//EXTIX_Init();
	printf("hello world\r\n");
	xTaskCreate( (TaskFunction_t) StartTask,       //任务函数
							 (const char*   ) "StartTask",     //任务名称
							 (uint16_t      ) START_STK_SIZE,  //任务堆栈大小
							 (void *        ) NULL,            //传递给任务函数的参数
							 (UBaseType_t   ) START_TASK_PRIO, //任务优先级
							 (TaskHandle_t* ) &StartTaskHanhler//任务句柄
	            );
							 
	vTaskStartScheduler();
}


void StartTask(void * pvParameter)
{
	taskENTER_CRITICAL();     //进入临界区
 
	//创建二值信号量
	
	CountSemaphore = xSemaphoreCreateCounting(255,0);
	//创建LED0任务
	
	xTaskCreate((TaskFunction_t) SemaphoreGiveTask,       //任务函数
							(const char*   ) "SemaphoreGiveTask",     //任务名称
							(uint16_t      ) SemaphoreGiveTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) SemaphoreGiveTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &SemaphoreGiveTaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) SemaphoreTakeTask,       //任务函数
							(const char*   ) "SemaphoreTakeTask",     //任务名称
							(uint16_t      ) SemaphoreTakeTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) SemaphoreTakeTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &SemaphoreTakeTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void SemaphoreGiveTask(void *pArg)
{
		u8 key,i = 0;
	  u8 semavalue;
	  BaseType_t err;
	  while(1)
		{
			key = KEY_Scan(0);
			if(CountSemaphore != NULL)
			{
				switch(key)
				{
					case KEY0_PRES:
						err = xSemaphoreGive(CountSemaphore);
					  if(err == pdFALSE)
						{
							printf("Give Semaphore failed\r\n");
						}
						printf("Key scan successful\r\n");
						//获取计数型信号量值
						semavalue = uxSemaphoreGetCount(CountSemaphore);
						break;
				}
			}
			i++;
			if(i==50)
			{
				i = 0;
				LED0 = !LED0;
			}
		  vTaskDelay(500);
		}
}

void SemaphoreTakeTask(void *pArg)
{
	u8 num;
	u8 semavalue;
	while(1)
	{
		xSemaphoreTake(CountSemaphore,portMAX_DELAY);
		num++;
		semavalue = uxSemaphoreGetCount(CountSemaphore);
		printf("Number %d\r\n",semavalue);
		vTaskDelay(1000);
	}
}



void USART1_IRQHandler(void)                	 
{
	u8 Res;
	BaseType_t xHighPriorityTaskWoken;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)   
		{
	  	Res =USART_ReceiveData(USART1); 
		
		if((USART_RX_STA&0x80)==0) 
			{
			if(USART_RX_STA&0x40) 
				{
					if(Res!=0x0a)USART_RX_STA=0; 
					else USART_RX_STA|=0x80;	 
				}
			else  
				{	
				if(Res==0x0d)USART_RX_STA|=0x40;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3F]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>63)USART_RX_STA=0; 
					}		 
				}
			}   		 
     } 
		//就向队列发送接收到的数据
		if((USART_RX_STA&0x80)&&(CountSemaphore!=NULL))
		{
			//向队列中发送数据
	    xSemaphoreGiveFromISR(CountSemaphore,&xHighPriorityTaskWoken);
			portYIELD_FROM_ISR(xHighPriorityTaskWoken);
		}
} 


