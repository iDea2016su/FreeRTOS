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
StackType_t StartTaskStack[START_STK_SIZE]; //任务堆栈
StaticTask_t StartTaskTCB;            //任务控制块
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数


#define KEY_TASK_PRIO     2          //任务优先级
#define KEY_STK_SIZE     128        //任务堆栈大小
StackType_t KEYTaskStack[START_STK_SIZE]; //任务堆栈
StaticTask_t KEYTaskTCB;            //任务控制块
TaskHandle_t KEYTaskHanhler;        //任务句柄
void KEYTask(void *pArg);           //任务函数


#define TASK1_TASK_PRIO    3          //任务优先级
#define TASK1_STK_SIZE     128        //任务堆栈大小
StackType_t TASK1TaskStack[START_STK_SIZE]; //任务堆栈
StaticTask_t TASK1TaskTCB;            //任务控制块
TaskHandle_t TASK1TaskHanhler;        //任务句柄
void TASK1Task(void *pArg);           //任务函数

#define TASK2_TASK_PRIO    4          //任务优先级
#define TASK2_STK_SIZE     128        //任务堆栈大小
StackType_t TASK2TaskStack[START_STK_SIZE]; //任务堆栈
StaticTask_t TASK2TaskTCB;            //任务控制块
TaskHandle_t TASK2TaskHanhler;        //任务句柄
void TASK2Task(void *pArg);           //任务函数



//空闲任务任务堆栈
static StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
//空闲任务控制块
static StaticTask_t IdleTaskTCB;

//定时器服务任务堆栈
static StackType_t TimerTaskStack[configTIMER_TASK_STACK_DEPTH];
//定时器服务任务控制块
static StaticTask_t TimerTaskTCB;

//获取空闲任务地任务堆栈和任务控制块内存，因为本历程使用的
//静态内存，因此空闲任务的任务堆栈和任务控制块的内存就应该
//由用户来提供，FreeRTOS提供了接口函数vApplicationGetIdleTaskMemory()
//实现此函数即可。

//ppxIdleTaskTCBBuffer:任务控制块内存
//ppxIdleTaskStackBuffer:任务堆栈内存
//pulIdleTaskStackSize:任务堆栈大小

void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer,
																	 StackType_t ** ppxIdleTaskStackBuffer,
																	 uint32_t * pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &IdleTaskTCB;
	*ppxIdleTaskStackBuffer = IdleTaskStack;
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

//获取定时器服务任务的任务堆栈和任务控制块内存
//ppxTimerTaskTCBBuffer:任务控制块内存
//ppxTimerTsakStackBuffer:任务堆栈内存
//pulTimerTaskStackSize:任务堆栈大小

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
																		StackType_t  **ppxTimerTaskStackBuffer,
																		uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &TimerTaskTCB;
	*ppxTimerTaskStackBuffer = TimerTaskStack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	KEY_Init();	 
	EXTIX_Init();
	printf("system started\r\n");
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
	xTaskCreate((TaskFunction_t) KEYTask,       //任务函数
							(const char*   ) "KEYTask",     //任务名称
							(uint16_t      ) KEY_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) KEY_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &KEYTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void TASK1Task(void *pArg)
{
	  while(1)
		{
			printf("task 1 running \r\n");
			LED1 = ~LED1;
		  vTaskDelay(1000);
		}
}

void TASK2Task(void *pArg)
{
	  while(1)
		{
		  printf("task 2 running\r\n");
	    vTaskDelay(1000);
			LED0 = ~LED0;
		}

}
void KEYTask(void *pArg)
{
	u8 key=0;
	while(1)
	{
		key = KEY_Scan(0);
		switch(key)
		{
			case KEY0_PRES:
				   vTaskSuspend(TASK1TaskHanhler);
			     printf("suspend task 1\r\n");
			     break;
			case KEY1_PRES:
				   vTaskResume(TASK1TaskHanhler);
			     printf("resume task  1\r\n");
			     break;
			case KEY2_PRES:
				   vTaskSuspend(TASK2TaskHanhler);
			     printf("suspend task 2\r\n");
			     break;
		}
		vTaskDelay(10);
	}

}

//外部中断0服务程序 
void EXTI4_IRQHandler(void)
{
	BaseType_t YieldRequired;
	delay_xms(20);//消抖
	 		 
	YieldRequired = 	xTaskResumeFromISR(TASK2TaskHanhler);
	printf("resume task 2\r\n");
	if(YieldRequired == pdTRUE)  // 恢复的任务大于被打断的任务，要进行上下文切换
	{
	   portYIELD_FROM_ISR(YieldRequired);
	}
	
	EXTI_ClearITPendingBit(EXTI_Line4); //清除LINE0上的中断标志位  
}
