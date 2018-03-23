#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timer.h"

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     256        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define INTERRUPT_TASK_PRIO    2          //任务优先级
#define INTERRUPT_STK_SIZE     256         //任务堆栈大小
TaskHandle_t INTERRUPTTaskHanhler;        //任务句柄
void INTERRUPTTask(void *pArg);           //任务函数



int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	TIM3_Int_Init(10000-1,7200-1);
	TIM5_Int_Init(10000-1,7200-1);
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
	
		xTaskCreate( (TaskFunction_t) INTERRUPTTask,       //任务函数
							 (const char*   ) "INTERRUPTTask",     //任务名称
							 (uint16_t      ) INTERRUPT_STK_SIZE,  //任务堆栈大小
							 (void *        ) NULL,            //传递给任务函数的参数
							 (UBaseType_t   ) INTERRUPT_TASK_PRIO, //任务优先级
							 (TaskHandle_t* ) &INTERRUPTTaskHanhler//任务句柄
	            );
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void INTERRUPTTask(void *pArg)
{
		static u32 total_num = 0;
	    while(1)
		{
			total_num += 1;
			if(total_num == 5)
			{
				printf("close interrupt........\r\n");
				portDISABLE_INTERRUPTS();   //关闭中断
				delay_xms(5000);    		//延时5s
				printf("open interrupt.........\r\n");
				portENABLE_INTERRUPTS();
			//	total_num = 0;
			}
			LED0 = ~LED0;
		}
		vTaskDelay(1000);
}