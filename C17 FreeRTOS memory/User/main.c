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

#define MallocTask_TASK_PRIO    2        //任务优先级
#define MallocTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t MallocTaskHanhler;          //任务句柄
void MallocTask(void *pArg);             //任务函数


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
	
	xTaskCreate((TaskFunction_t) MallocTask,            //任务函数
							(const char*   ) "MallocTask",          //任务名称
							(uint16_t      ) MallocTask_STK_SIZE,   //任务堆栈大小
							(void *        ) NULL,                       //传递给任务函数的参数
							(UBaseType_t   ) MallocTask_TASK_PRIO,  //任务优先级
							(TaskHandle_t* ) &MallocTaskHanhler     //任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void MallocTask(void * pAgr)
{
	u8 *buffer = NULL;
	u8 times,i,key = 0;
	u32 freemem;
	while(1)
	{
		key = KEY_Scan(0);
		switch(key)
		{
			case KEY0_PRES:
				if(buffer == NULL)
				{
				   buffer = pvPortMalloc(30);
				}
			  printf("Addr of mem is %#x\r\n",(int)buffer);
			  break;
			case KEY1_PRES:
				if(buffer != NULL)
				{
					vPortFree(buffer);
				}
				buffer = NULL;
				break;
			case KEY2_PRES:
				if(buffer != NULL)
				{
					times ++;
					sprintf((char*)buffer,"use %d times",times);
					printf("%s\r\n",buffer);
				}
				break;
		}
		while(KEY_Scan(0)!=0);
		freemem = xPortGetFreeHeapSize();
	
		i++;
		if(i>=100)
		{
			i=0;
			LED0 = !LED0;
				printf("Remain %d size\r\n",freemem);
		}
		vTaskDelay(10);
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