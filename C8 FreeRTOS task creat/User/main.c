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

#define LIST_TASK_PRIO    4          //任务优先级
#define LIST_STK_SIZE     128        //任务堆栈大小
TaskHandle_t LISTTaskHanhler;        //任务句柄
void LISTTask(void *pArg);           //任务函数

List_t TestList;                     //测试用列表
ListItem_t ListItem1;                //测试用列表项1
ListItem_t ListItem2;                //测试用列表项2
ListItem_t ListItem3;                //测试用列表项3

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	KEY_Init();	 
	EXTIX_Init();
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
	xTaskCreate((TaskFunction_t) LISTTask,        //任务函数
							(const char*   ) "LISTTask",      //任务名称
							(uint16_t      ) LIST_STK_SIZE,   //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) LIST_TASK_PRIO,  //任务优先级
							(TaskHandle_t* ) &LISTTaskHanhler //任务句柄
							);
		//删除开始任务
		vTaskDelete(StartTaskHanhler);
	  
	  taskEXIT_CRITICAL();      //退出临界区
}

void TASK1Task(void *pArg)
{
	  while(1)
		{
		//	printf("task 1 running \r\n");
			LED0 = ~LED0;
		  vTaskDelay(500);
		}
}

void LISTTask(void * pvParameters)
{
	//第一步：初始化列表和列表项
	vListInitialise(&TestList);
	vListInitialiseItem(&ListItem1);
	vListInitialiseItem(&ListItem2);
	vListInitialiseItem(&ListItem3);
	
	ListItem1.xItemValue = 40;
	ListItem2.xItemValue = 60;
	ListItem3.xItemValue = 50;
	
	//第二步：打印列表和其他列表项的地址
	printf("/****************List and ListItem address****************/\r\n");
	printf("Project                                        address     \r\n");
	printf("TestList                                       %#x         \r\n",(int)&TestList);
	printf("TestList->pxIndex                              %#x         \r\n",(int)TestList.pxIndex);
	printf("TestList->ListEnd                              %#x         \r\n",(int)&TestList.xListEnd);
	printf("ListItem1                                      %#x         \r\n",(int)&ListItem1);
	printf("ListItem2                                      %#x         \r\n",(int)&ListItem2);
	printf("ListItem3                                      %#x         \r\n",(int)&ListItem3);
	printf("/**********************END********************************/\r\n");
	printf("Please put KEY0 to continue\r\n");
	while(KEY_Scan(0)!=KEY0_PRES) delay_ms(10);
	//第三步：向列表中添加列表项ListItem1，并通过串口打印所有
	//列表项中成员变量pxNext和pxPrevious的值，通过这两个值观察列表
	//项在列表中的连接情况
	vListInsert(&TestList,&ListItem1);                     //插入列表项1
	printf("/***************Insert ListItem1**************/\r\n");
	printf("Projet                           address       \r\n");
	printf("TestList->xListEnd->pNext        %#x           \r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem1->pxNext                %#x           \r\n",(int)(ListItem1.pxNext));
	printf("/**************before and next connection line**************/\r\n");
	printf("TestList->xListEnd->pxPrevious   %#x           \r\n",(int)(TestList.xListEnd.pxPrevious));
	printf("ListItem1->pxPrevious            %#x           \r\n",(int)(ListItem1.pxPrevious));
	printf("/*********************end*********************/\r\n");
	printf("Please put KEY0 to continue\r\n\r\n\r\n");
	while(KEY_Scan(0)!=KEY0_PRES) delay_ms(10);
	
	
	//第四步:向列表中添加列表项ListItem2，并通过串口打印所有
	//列表项中成员变量pxNext和pxPrevious的值，通过这两个值观察列表
	//项在列表中的连接情况
	vListInsert(&TestList,&ListItem2);                     //插入列表项1
	printf("/***************Insert ListItem2**************/\r\n");
	printf("Projet                           address       \r\n");
	printf("TestList->xListEnd->pNext        %#x           \r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem1->pxNext                %#x           \r\n",(int)(ListItem1.pxNext));
	printf("ListItem2->pxNext                %#x           \r\n",(int)(ListItem2.pxNext));
	printf("/**************before and next connection line**************/\r\n");
	printf("TestList->xListEnd->pxPrevious   %#x           \r\n",(int)(TestList.xListEnd.pxPrevious));
	printf("ListItem1->pxPrevious            %#x           \r\n",(int)(ListItem1.pxPrevious));
  printf("ListItem2->pxPrevious            %#x           \r\n",(int)(ListItem2.pxPrevious));
	printf("/*********************end*********************/\r\n");
	printf("Please put KEY0 to continue\r\n\r\n\r\n");
	while(KEY_Scan(0)!=KEY0_PRES) delay_ms(10);
	
	
	//第五步:向列表中添加列表项ListItem2，并通过串口打印所有
	//列表项中成员变量pxNext和pxPrevious的值，通过这两个值观察列表
	//项在列表中的连接情况
	vListInsert(&TestList,&ListItem2);                     //插入列表项1
	printf("/***************Insert ListItem2**************/\r\n");
	printf("Projet                           address       \r\n");
	printf("TestList->xListEnd->pNext        %#x           \r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem1->pxNext                %#x           \r\n",(int)(ListItem1.pxNext));
	printf("ListItem3->pxNext                %#x           \r\n",(int)(ListItem3.pxNext));
	printf("ListItem2->pxNext                %#x           \r\n",(int)(ListItem2.pxNext));
	printf("/**************before and next connection line**************/\r\n");
	printf("TestList->xListEnd->pxPrevious   %#x           \r\n",(int)(TestList.xListEnd.pxPrevious));
	printf("ListItem1->pxPrevious            %#x           \r\n",(int)(ListItem1.pxPrevious));
  printf("ListItem3->pxPrevious            %#x           \r\n",(int)(ListItem3.pxPrevious));
	printf("ListItem2->pxPrevious            %#x           \r\n",(int)(ListItem2.pxPrevious));
	printf("/*********************end*********************/\r\n");
	printf("Please put KEY0 to continue\r\n\r\n\r\n");
	while(KEY_Scan(0)!=KEY0_PRES) delay_ms(10);
	
	//第六步:向列表中删除列表项ListItem2，并通过串口打印所有
	//列表项中成员变量pxNext和pxPrevious的值，通过这两个值观察列表
	//项在列表中的连接情况
	uxListRemove(&ListItem2);                     //插入列表项1
	printf("/***************Insert ListItem2**************/\r\n");
	printf("Projet                           address       \r\n");
	printf("TestList->xListEnd->pNext        %#x           \r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem1->pxNext                %#x           \r\n",(int)(ListItem1.pxNext));
	printf("ListItem3->pxNext                %#x           \r\n",(int)(ListItem3.pxNext));
	printf("/**************before and next connection line**************/\r\n");
	printf("TestList->xListEnd->pxPrevious   %#x           \r\n",(int)(TestList.xListEnd.pxPrevious));
	printf("ListItem1->pxPrevious            %#x           \r\n",(int)(ListItem1.pxPrevious));
  printf("ListItem3->pxPrevious            %#x           \r\n",(int)(ListItem3.pxPrevious));
	printf("/*********************end*********************/\r\n");
	printf("Please put KEY0 to continue\r\n\r\n\r\n");
	while(KEY_Scan(0)!=KEY0_PRES) delay_ms(10);
	
	//第七步：删除ListItem2,并通过串口打印所有列表项中成员变量pxNext和
	//pxPrevious的值，通过这两个值观察列表项在列表中的连接情况。
	TestList.pxIndex = TestList.pxIndex->pxNext;   //pxIndex向后移一项，这样，pxIndex就会指向ListItem1
  vListInsertEnd(&TestList,&ListItem2);          //列表末尾添加列表项ListItem2
	printf("/*******************add list item in the end******************/\r\n");
	printf("Projet                           address       \r\n");
	printf("TestList->pxIndex                %#x           \r\n",(int)(TestList.pxIndex));
	printf("TestList->xListEnd->pNext        %#x           \r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem2->pxNext                %#x           \r\n",(int)(ListItem2.pxNext));
	printf("ListItem1->pxNext                %#x           \r\n",(int)(ListItem1.pxNext));
	printf("ListItem3->pxNext                %#x           \r\n",(int)(ListItem3.pxNext));
	printf("/**************before and next connection line**************/\r\n");
	printf("TestList->xListEnd->pxPrevious   %#x           \r\n",(int)(TestList.xListEnd.pxPrevious));
	printf("ListItem2->pxPrevious            %#x           \r\n",(int)(ListItem2.pxPrevious));	
	printf("ListItem1->pxPrevious            %#x           \r\n",(int)(ListItem1.pxPrevious));
  printf("ListItem3->pxPrevious            %#x           \r\n",(int)(ListItem3.pxPrevious));
	printf("/*********************end*********************/\r\n");

	while(1)
	{
		LED1 = ! LED1;
		vTaskDelay(1000);
	}
	
}

