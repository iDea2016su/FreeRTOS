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

#define LowTask_TASK_PRIO    2        //任务优先级
#define LowTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t LowTaskHanhler;          //任务句柄
void LowTask(void *pArg);             //任务函数


#define MidTask_TASK_PRIO    3        //任务优先级
#define MidTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t MidTaskHanhler;          //任务句柄
void MidTask(void *pArg);             //任务函数

#define HigTask_TASK_PRIO    4        //任务优先级
#define HigTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t HigTaskHanhler;          //任务句柄
void HigTask(void *pArg);             //任务函数

SemaphoreHandle_t MutexSemaphore;



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
	
	MutexSemaphore = xSemaphoreCreateMutex();
	
	//创建LED0任务
	
	xTaskCreate((TaskFunction_t) HigTask,       //任务函数
							(const char*   ) "HigTask",     //任务名称
							(uint16_t      ) HigTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) HigTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &HigTaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) MidTask,       //任务函数
							(const char*   ) "MidTask",     //任务名称
							(uint16_t      ) MidTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) MidTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &MidTaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) LowTask,       //任务函数
							(const char*   ) "LowTask",     //任务名称
							(uint16_t      ) LowTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) LowTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &LowTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void HigTask(void *pArg)
{
	u8 num;
	
	while(1)
	{
		vTaskDelay(500);    //延时500ms
		num++;
		printf("high task Pend Sem\r\n");
		xSemaphoreTake(MutexSemaphore,portMAX_DELAY); //获取互斥信号量
		printf("High Task Running\r\n");
		LED1 = !LED1;
		xSemaphoreGive(MutexSemaphore);                //释放互斥信号量
    vTaskDelay(500);
	}
}

void MidTask(void *pArg)
{
	u8 num=0;
	while(1)
	{
		num++;
		printf("middle task Running\r\n");
		LED0 = !LED0;
		vTaskDelay(1000);
	}
}

void LowTask(void *pArg)
{
	static u32 times = 0;
	while(1)
	{
		xSemaphoreTake(MutexSemaphore,portMAX_DELAY);
		printf("Low Task Runnning");
		for(times=0;times<20000000;times++)  //模拟低优先级任务占用二值信号量
		{
			taskYIELD();  //发起任务调度
		}
		xSemaphoreGive(MutexSemaphore);
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
		if((USART_RX_STA&0x80)&&(MutexSemaphore!=NULL))
		{
			//向队列中发送数据
	    xSemaphoreGiveFromISR(MutexSemaphore,&xHighPriorityTaskWoken);
			portYIELD_FROM_ISR(xHighPriorityTaskWoken);
		}
} 


