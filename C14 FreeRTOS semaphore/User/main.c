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

#define Task1_TASK_PRIO    2          //任务优先级
#define Task1_STK_SIZE     256        //任务堆栈大小
TaskHandle_t Task1TaskHanhler;        //任务句柄
void Task1Task(void *pArg);           //任务函数

#define DataProcess_TASK_PRIO    3          //任务优先级
#define DataProcess_STK_SIZE     256        //任务堆栈大小
TaskHandle_t DataProcessTaskHanhler;        //任务句柄
void DataProcessTask(void *pArg);           //任务函数


SemaphoreHandle_t BinarySemaphore;  //二值信号量

#define LED1ON    1
#define LED1OFF   2
#define BEEFON    3
#define BEEFOFF   4
#define COMMANDERR 0XFF

u8 CommandProcess(u8* buf)
{
	u8 cValue = COMMANDERR;
	if(NULL != strstr(buf,"LED1ON"))
		cValue = LED1ON;
	else 	if(NULL != strstr(buf,"LED1OFF"))
		cValue = LED1OFF;
	else 	if(NULL != strstr(buf,"BEEFON"))
		cValue = BEEFON;
	else 	if(NULL != strstr(buf,"BEEFOFF"))
		cValue = BEEFOFF;
	return cValue;
}

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
	
	BinarySemaphore = xSemaphoreCreateBinary();
	//创建LED0任务
	
	xTaskCreate((TaskFunction_t) Task1Task,       //任务函数
							(const char*   ) "Task1Task",     //任务名称
							(uint16_t      ) Task1_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) Task1_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &Task1TaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) DataProcessTask,       //任务函数
							(const char*   ) "DataProcessTask",     //任务名称
							(uint16_t      ) DataProcess_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) DataProcess_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &DataProcessTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void Task1Task(void *pArg)
{
	  while(1)
		{
			LED0 = !LED0;
		  vTaskDelay(500);
		}
}

void DataProcessTask(void *pArg)
{
		u8 len = 0;
	  u8 CommandValue = COMMANDERR;
	  BaseType_t err = pdFALSE;
	  u8 * CommandStr;
		while(1)
		{
			if(BinarySemaphore!=NULL)
			{
			err = xSemaphoreTake(BinarySemaphore,portMAX_DELAY);
				if(err == pdTRUE)
				{
					len = USART_RX_STA&0x3f;  //接收数据长度
					CommandStr = pvPortMalloc(len+1);
					CommandStr[len] = '\0';
					CommandValue = CommandProcess(USART_RX_BUF);
					if(CommandValue != COMMANDERR)
					{
						switch(CommandValue)
						{
							case LED1ON :
								printf("LED1ON command\r\n");
							  break;
							case LED1OFF :
								printf("LED1OFF command\r\n");
							  break;
							case BEEFON :
								printf("BEEFON command\r\n");
							  break;
							case BEEFOFF :
								printf("BEEFOFF command\r\n");
							  break;
						}
					}
					else
					{
						printf("Invalid Command\r\n");
					}
					USART_RX_STA = 0;
					memset(USART_RX_BUF,0,64);
					vPortFree(CommandStr);
					
				}
			}
			else if(err == pdFALSE)
			{
							vTaskDelay(10);
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
		if((USART_RX_STA&0x80)&&(BinarySemaphore!=NULL))
		{
			//向队列中发送数据
	    xSemaphoreGiveFromISR(BinarySemaphore,&xHighPriorityTaskWoken);
			portYIELD_FROM_ISR(xHighPriorityTaskWoken);
		}
} 


