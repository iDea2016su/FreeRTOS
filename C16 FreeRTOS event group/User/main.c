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
#include "timers.h"
#include "event_groups.h"

#define START_TASK_PRIO    1                  //任务优先级
#define START_STK_SIZE     256                //任务堆栈大小
TaskHandle_t StartTaskHanhler;                //任务句柄
void StartTask(void *pvParameters);           //任务函数

#define EventSetBitTask_TASK_PRIO    2        //任务优先级
#define EventSetBitTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t EventSetBitTaskHanhler;          //任务句柄
void EventSetBitTask(void *pArg);             //任务函数

#define EventGroupTask_TASK_PRIO    3        //任务优先级
#define EventGroupTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t EventGroupTaskHanhler;          //任务句柄
void EventGroupTask(void *pArg);             //任务函数

#define EventQueryTask_TASK_PRIO    4        //任务优先级
#define EventQueryTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t EventQueryTaskHanhler;          //任务句柄
void EventQueryTask(void *pArg);             //任务函数

EventGroupHandle_t EventGroupHandler;        //事件标志组句柄

#define EVENTBIT_0   (1<<0)                  //事件位，1左移0位
#define EVENTBIT_1   (1<<1)
#define EVENTBIT_2   (1<<2)

#define EVENTBIT_ALL (EVENTBIT_0|EVENTBIT_1|EVENTBIT_2)

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	KEY_Init();	
  EXTIX_Init();	
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
 
	EventGroupHandler = xEventGroupCreate();
	
	xTaskCreate((TaskFunction_t) EventSetBitTask,            //任务函数
							(const char*   ) "EventSetBitTask",          //任务名称
							(uint16_t      ) EventSetBitTask_STK_SIZE,   //任务堆栈大小
							(void *        ) NULL,                       //传递给任务函数的参数
							(UBaseType_t   ) EventSetBitTask_TASK_PRIO,  //任务优先级
							(TaskHandle_t* ) &EventSetBitTaskHanhler     //任务句柄
							);
	xTaskCreate((TaskFunction_t) EventGroupTask,            //任务函数
							(const char*   ) "EventGroupTask",          //任务名称
							(uint16_t      ) EventGroupTask_STK_SIZE,   //任务堆栈大小
							(void *        ) NULL,                      //传递给任务函数的参数
							(UBaseType_t   ) EventGroupTask_TASK_PRIO,  //任务优先级
							(TaskHandle_t* ) &EventGroupTaskHanhler     //任务句柄
							);
	xTaskCreate((TaskFunction_t) EventQueryTask,            //任务函数
							(const char*   ) "EventQueryTask",          //任务名称
							(uint16_t      ) EventQueryTask_STK_SIZE,   //任务堆栈大小
							(void *        ) NULL,                      //传递给任务函数的参数
							(UBaseType_t   ) EventQueryTask_TASK_PRIO,  //任务优先级
							(TaskHandle_t* ) &EventQueryTaskHanhler     //任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void EventSetBitTask(void * pAgr)
{
	u8 key;
	while(1)
	{
		if(EventGroupHandler != NULL)
		{
			key = KEY_Scan(0);
			switch(key)
			{
				case KEY1_PRES:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_1);
				  break;
			  case KEY2_PRES:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_2);
				  break;
			}
		}
		vTaskDelay(10);  
	}
}

void EventGroupTask(void * pAgr)
{
	u8 num;
	EventBits_t EventValue;
	while(1)
	{
		if(EventGroupHandler != NULL)
		{
			//等待事件组中的相应事件位
			EventValue = xEventGroupWaitBits((EventGroupHandle_t )EventGroupHandler,
																			 (EventBits_t        )EVENTBIT_ALL,
																			 (BaseType_t         )pdTRUE,
																			 (BaseType_t         )pdTRUE,
																			 (TickType_t         )portMAX_DELAY
			                                );
			printf("the event value %d\r\n",EventValue);
			printf("Get All bits\r\n");
			num++;
			LED1 = !LED1;
		}
		else
		{
			vTaskDelay(10);
		}
	}
}

void EventQueryTask(void * pAgr)
{
	u8 num = 0;
	EventBits_t NewValue,LastValue;
	while(1)
	{
		if(EventGroupTaskHanhler != NULL)
		{
			NewValue = xEventGroupGetBits(EventGroupHandler);   //获取事件组的值
			if(NewValue != LastValue)
			{
				LastValue = NewValue;
				printf("Update the event value %d\r\n",NewValue);
			}
		}
		num ++;
		if(num>=50)
		{
			num = 0;
			LED0 =! LED0;
		}
		vTaskDelay(50);
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
//		//就向队列发送接收到的数据
//		if((USART_RX_STA&0x80)&&(BinarySemaphore!=NULL))
//		{
//			//向队列中发送数据
//	    xSemaphoreGiveFromISR(BinarySemaphore,&xHighPriorityTaskWoken);
//			portYIELD_FROM_ISR(xHighPriorityTaskWoken);
//		}
} 

void EXTI4_IRQHandler(void)
{
	BaseType_t Result,xHigherPriorityTaskWoken;
	delay_xms(50);
	if(KEY3 == 0)
	{
		Result = xEventGroupSetBitsFromISR(EventGroupHandler,EVENTBIT_0,&xHigherPriorityTaskWoken);
		if(Result != pdFAIL)
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line4);
}