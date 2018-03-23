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

#define LED0_TASK_PRIO    2          //任务优先级
#define LED0_STK_SIZE     128        //任务堆栈大小
TaskHandle_t LED0TaskHanhler;        //任务句柄
void LED0Task(void *pArg);           //任务函数

#define QUERY_TASK_PRIO    3          //任务优先级
#define QUERY_STK_SIZE     256        //任务堆栈大小
TaskHandle_t QUERYTaskHanhler;        //任务句柄
void QUERYTask(void *pArg);           //任务函数

char InfoBuffer[1000];                //保存信息的数组


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
	
	xTaskCreate((TaskFunction_t) LED0Task,       //任务函数
							(const char*   ) "LED0Task",     //任务名称
							(uint16_t      ) LED0_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) LED0_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &LED0TaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) QUERYTask,       //任务函数
							(const char*   ) "QUERYTask",     //任务名称
							(uint16_t      ) QUERY_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) QUERY_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &QUERYTaskHanhler//任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void LED0Task(void *pArg)
{
	  while(1)
		{
			LED0 = ~LED0;
		  vTaskDelay(500);
		}
}

void QUERYTask(void *pArg)
{
		u32 TotalarunTime;
		UBaseType_t ArraySize,x;
		TaskStatus_t *StatusArray;
	
		//第一步，函数uxTaskGetSystemState()的使用
	  printf("/************ Step ONE : Founction uxTaskGetSystemState() ***********/\r\n");
	  ArraySize = uxTaskGetNumberOfTasks();     										//获取系统任务数量
	  StatusArray = pvPortMalloc(ArraySize*sizeof(TaskStatus_t));		//申请内存
	  if(StatusArray != NULL)                                       //申请内存成功
		{
			ArraySize = uxTaskGetSystemState((TaskStatus_t* )StatusArray,
																			 (UBaseType_t   )ArraySize,
																			 (uint32_t*     )&TotalarunTime);
			printf("TaskName\t\tPriority\t\tTaskNumber\t\t\r\n");
			for(x=0;x<ArraySize;x++)
			{
				//通过串口打印出获取到的系统任务的有关信息，比如任务名称，优先级和编号。
				printf("%s\t\t%d\t\t\t%d\t\t\t\r\n",
				        StatusArray[x].pcTaskName,
                (int)StatusArray[x].uxCurrentPriority,
								(int)StatusArray[x].xTaskNumber);
			}
		}
		vPortFree(StatusArray);                                       //释放内存
		printf("/********************END*********************/\r\n");
		printf("/**************KEY0 to Continue**************/\r\n");
		while(KEY_Scan(0)==KEY3_PRES) delay_ms(10);
		
		//第二步：函数:vTaskGetInfo()的使用
		TaskHandle_t TaskHandle;
		TaskStatus_t TaskStatus;
		
		printf("/************ Step TWO : Founction vTaskGetInfo() ********************/\r\n");
		TaskHandle = xTaskGetHandle("LED0Task");                       		//根据任务名称获取任务句柄
		//获取LED0Task的任务信息
		vTaskGetInfo((TaskHandle_t  )TaskHandle,     //任务句柄
								 (TaskStatus_t* )&TaskStatus,    //任务信息结构体
								 (BaseType_t    )pdTRUE,         //允许统计任务堆栈历史最小剩余大小
								 (eTaskState    )eInvalid        //函数自己获取任务运行状态
		            );
		//通过串口打印出指定任务的有关信息。
		printf("TaskName:                                  %s\r\n",TaskStatus.pcTaskName);
		printf("TaskNumber:                                %d\r\n",(int)TaskStatus.xTaskNumber);
		printf("TaskStatus:                                %d\r\n",TaskStatus.eCurrentState);
		printf("TaskPriority:                              %d\r\n",(int)TaskStatus.uxCurrentPriority);
		printf("TaskBasePriority                           %d\r\n",(int)TaskStatus.uxBasePriority);
		printf("TaskStackBaseAddr                          %#x\r\n",(int)TaskStatus.pxStackBase);
		printf("TaskStackMinHistry                         %d\r\n",(int)TaskStatus.usStackHighWaterMark);
	  printf("/********************END*********************/\r\n");
		printf("/**************KEY0 to Continue**************/\r\n");
		while(KEY_Scan(0)==KEY3_PRES) delay_ms(10);
		
		//第三步：函数eTaskGetState()的使用
		eTaskState TaskState;
		char TaskInfo[10];
		printf("/************ Step Three Founction: eeTaskGetState()* ************/\r\n");
		TaskHandle = xTaskGetHandle("QUERYTask");                //根据任务名称获取任务句柄
		TaskState = eTaskGetState(TaskHandle);                   //获取QUERYTask任务的任务状态
		memset(TaskInfo,0,10);
		switch((int)TaskState)
		{
			case 0:sprintf(TaskInfo,"Running");break;
			case 1:sprintf(TaskInfo,"Ready");break;
			case 2:sprintf(TaskInfo,"Suspend");break;
			case 3:sprintf(TaskInfo,"Delete");break;
			case 4:sprintf(TaskInfo,"Invalid");break;
		}
		printf("TaskStatusNumber %d   TaskStatus %s  \r\n",TaskState,TaskInfo);
		printf("/********************END*********************/\r\n");
		printf("/**************KEY0 to Continue**************/\r\n");
		while(KEY_Scan(0)==KEY3_PRES) delay_ms(10);
		
		//第四步：函数vTaskList()的使用
		printf("/************ Step Four Founction: vTaskList()* ************/\r\n");
		vTaskList(InfoBuffer);                 //获取所有任务的信息
		printf("%s\r\n",InfoBuffer);           //通过串口打印所有的任务信息
		
		while(1)
		{
			LED1 = !LED1;
			vTaskDelay(1000);
		}
}
