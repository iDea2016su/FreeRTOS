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
#include "limits.h"

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     256        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define Task1Task_TASK_PRIO    2          //任务优先级
#define Task1Task_STK_SIZE     256        //任务堆栈大小
TaskHandle_t Task1TaskHanhler;            //任务句柄
void Task1Task(void *pArg);               //任务函数

#define KeyProcessTask_TASK_PRIO    3          //任务优先级
#define KeyProcessTask_STK_SIZE     256        //任务堆栈大小
TaskHandle_t KeyProcessTaskHanhler;            //任务句柄
void KeyProcessTask(void *pArg);               //任务函数


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
 
	//创建LED0任务
	
	xTaskCreate((TaskFunction_t) Task1Task,       //任务函数
							(const char*   ) "Task1Task",     //任务名称
							(uint16_t      ) Task1Task_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) Task1Task_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &Task1TaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) KeyProcessTask,       //任务函数
							(const char*   ) "KeyProcessTask",     //任务名称
							(uint16_t      ) KeyProcessTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) KeyProcessTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &KeyProcessTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void Task1Task(void *pArg)
{
		u8 key,i = 0;
	  BaseType_t err;
	  while(1)
		{
			key = KEY_Scan(0);
			if((KeyProcessTaskHanhler!= NULL)&&(key))
			{
				err = xTaskNotify((TaskHandle_t  )KeyProcessTaskHanhler,
													(uint32_t          )key,
													(eNotifyAction     )eSetValueWithOverwrite
				);
				if(err == pdFAIL)
				{
					printf("mail failed\r\n");
				}
			}
			i++;
			if(i>=50)
			{
				i=0;
				LED0 = !LED0;
			}
			vTaskDelay(10);
		}
}

void KeyProcessTask(void * pAgr)
{
	u8 num,beepsta = 1;
	uint32_t NotifyValue;
	BaseType_t err;
	while(1)
	{
		err = xTaskNotifyWait((uint32_t    )0X00,     //进入函数的时候不清除任务bit
		                      (uint32_t    )ULONG_MAX, //退出的时候清除所有的bit
		                      (uint32_t *  )&NotifyValue,
		                      (TickType_t  )portMAX_DELAY
		);
		if(err == pdTRUE)
		{
			switch((u8)NotifyValue)
			{
				case KEY0_PRES:
					LED1 = !LED1;
				  break;
				case KEY1_PRES:
					LED2 = !LED2;
				  break;
				case KEY2_PRES:
					LED3 = !LED3;
				  break;
			}
		}
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


