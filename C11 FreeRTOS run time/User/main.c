#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timer.h"
#include "exit.h"
#include "key.h"

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     128        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define TASK1_TASK_PRIO    2          //任务优先级
#define TASK1_STK_SIZE     128        //任务堆栈大小
TaskHandle_t TASK1TaskHanhler;        //任务句柄
void TASK1Task(void *pArg);           //任务函数

#define TASK2_TASK_PRIO    3          //任务优先级
#define TASK2_STK_SIZE     128        //任务堆栈大小
TaskHandle_t TASK2TaskHanhler;        //任务句柄
void TASK2Task(void *pArg);           //任务函数


#define RUNTIMESTATS_TASK_PRIO    4          //任务优先级
#define RUNTIMESTATS_STK_SIZE     128        //任务堆栈大小
TaskHandle_t RUNTIMESTATSTaskHanhler;        //任务句柄
void RUNTIMESTATSTask(void *pArg);           //任务函数


char RunTimeInof[400];                //保存信息的数组


int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	KEY_Init();	 
	//EXTIX_Init();
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
	
	xTaskCreate((TaskFunction_t) TASK1Task,       //任务函数
							(const char*   ) "TASK1Task",     //任务名称
							(uint16_t      ) TASK1_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) TASK1_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &TASK1TaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) TASK2Task,       //任务函数
							(const char*   ) "TASK2Task",     //任务名称
							(uint16_t      ) TASK2_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) TASK2_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &TASK2TaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) RUNTIMESTATSTask,        //任务函数
							(const char*   ) "RUNTIMESTATSYTask",     //任务名称
							(uint16_t      ) RUNTIMESTATS_STK_SIZE,   //任务堆栈大小
							(void *        ) NULL,                    //传递给任务函数的参数
							(UBaseType_t   ) RUNTIMESTATS_TASK_PRIO,  //任务优先级
							(TaskHandle_t* ) &RUNTIMESTATSTaskHanhler //任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void TASK1Task(void *pArg)
{
	  while(1)
		{
			LED0 = ~LED0;
		  vTaskDelay(1000);
		}
}
void TASK2Task(void *pArg)
{
	  while(1)
		{
			LED1 = ~LED1;
		  vTaskDelay(1000);
		}
}
void RUNTIMESTATSTask(void *pArg)
{
	  u8 key = 0;
	  while(1)
		{
			key = KEY_Scan(0);
			if(key == KEY1_PRES)
			{
				memset(RunTimeInof,0,400);
				vTaskGetRunTimeStats(RunTimeInof);
				printf("TaskName      RunTime       %%\r\n");
				printf("%s\r\n",RunTimeInof);
			}
		}
}
