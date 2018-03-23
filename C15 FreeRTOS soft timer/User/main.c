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

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     256        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define TimerControlTask_TASK_PRIO    2        //任务优先级
#define TimerControlTask_STK_SIZE     256      //任务堆栈大小
TaskHandle_t TimerControlTaskHanhler;          //任务句柄
void TimerControlTask(void *pArg);             //任务函数


TimerHandle_t AutoReloadTimerHandle;      //周期定时器句柄
TimerHandle_t OneShotTimerHandle;         //单次定时器句柄

void AutoReloadCallback(TimerHandle_t xTimer);
void OneShotCallback(TimerHandle_t xtimer);


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
 
  //创建软件周期定时器，周期定时器，周期1s(1000个时钟节拍)，周期模式
	AutoReloadTimerHandle = xTimerCreate((const char *)"AutoReloadTimer",
																			 (TickType_t  )1000,
																			 (UBaseType_t )pdTRUE,
																			 (void *      )1,
																			 (TimerCallbackFunction_t)AutoReloadCallback
	                                     );
	OneShotTimerHandle = xTimerCreate((const char *  )"OneShotTimer",
																		(TickType_t    )2000,
																		(UBaseType_t   )pdFALSE,
																	  (void *        )2,
																		(TimerCallbackFunction_t)OneShotCallback
																		);
	xTaskCreate((TaskFunction_t) TimerControlTask,       //任务函数
							(const char*   ) "HigTask",     //任务名称
							(uint16_t      ) TimerControlTask_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) TimerControlTask_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &TimerControlTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void TimerControlTask(void *pArg)
{
	  u8 key, num;
	  while(1)
		{
			//只有两个定时器都创建成功了才能对其进行操作
			if((AutoReloadTimerHandle!=NULL)&&(OneShotTimerHandle!=NULL))
			{
				key = KEY_Scan(0);
				switch(key)
				{
					case KEY0_PRES:
						xTimerStart(AutoReloadTimerHandle,0);
					  printf("Start peri Timer\r\n");
					  break;
					case KEY1_PRES:
						xTimerStart(OneShotTimerHandle,0);
					  printf("Start one Timer\r\n");
					  break;
					case KEY2_PRES:
						xTimerStop(AutoReloadTimerHandle,0);
					  xTimerStop(OneShotTimerHandle,0);
					  printf("Stop All timers\r\n");
					  break;
				}
			}
			num++;
			if(num==50) 
			{
				num = 0;
				LED0 = !LED0;
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

void AutoReloadCallback(TimerHandle_t xTimer)
{
	static u8 timer1Num = 0;
	timer1Num ++;
	printf("Timer peri run %d times\r\n",timer1Num);
}

void OneShotCallback(TimerHandle_t xTimer)
{
	static u8 timer2Num = 0;
	timer2Num ++;
	LED1 = !LED1;
	printf("Timer once stops in %d times\r\n",timer2Num);
}
