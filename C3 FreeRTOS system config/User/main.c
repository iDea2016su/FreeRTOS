#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     128        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define LED0_TASK_PRIO    2          //任务优先级
#define LED0_STK_SIZE     50        //任务堆栈大小
TaskHandle_t LED0TaskHanhler;        //任务句柄
void LED0Task(void *pArg);           //任务函数

#define LED1_TASK_PRIO    3          //任务优先级
#define LED1_STK_SIZE     50        //任务堆栈大小
TaskHandle_t LED1TaskHanhler;        //任务句柄
void LED1Task(void *pArg);           //任务函数

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	printf("hello\r\n");
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
		xTaskCreate( (TaskFunction_t) LED0Task,       //任务函数
							 (const char*   ) "LED0Task",     //任务名称
							 (uint16_t      ) LED0_STK_SIZE,  //任务堆栈大小
							 (void *        ) NULL,            //传递给任务函数的参数
							 (UBaseType_t   ) LED0_TASK_PRIO, //任务优先级
							 (TaskHandle_t* ) &LED0TaskHanhler//任务句柄
	            );
	
		//创建LED1任务
		xTaskCreate( (TaskFunction_t) LED1Task,       //任务函数
							 (const char*   ) "LED1Task",     //任务名称
							 (uint16_t      ) LED1_STK_SIZE,  //任务堆栈大小
							 (void *        ) NULL,            //传递给任务函数的参数
							 (UBaseType_t   ) LED1_TASK_PRIO, //任务优先级
							 (TaskHandle_t* ) &LED1TaskHanhler//任务句柄
	            );
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void LED0Task(void *pArg)
{
	while(1)
	{
		printf("hell1\r\n");
		LED0 = ~LED0;
		vTaskDelay(500);
		
	}
}
void LED1Task(void *pArg)
{
	while(1)
	{
		
		printf("hello\r\n");
		LED0 = 1;
		vTaskDelay(200);
		LED0 = 0;
		vTaskDelay(800);
		
	}
}