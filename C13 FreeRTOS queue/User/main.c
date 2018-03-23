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

#define START_TASK_PRIO    1          //任务优先级
#define START_STK_SIZE     256        //任务堆栈大小
TaskHandle_t StartTaskHanhler;        //任务句柄
void StartTask(void *pvParameters);   //任务函数

#define Task1_TASK_PRIO    2          //任务优先级
#define Task1_STK_SIZE     256        //任务堆栈大小
TaskHandle_t Task1TaskHanhler;        //任务句柄
void Task1Task(void *pArg);           //任务函数

#define KeyProcess_TASK_PRIO    3          //任务优先级
#define KeyProcess_STK_SIZE     256        //任务堆栈大小
TaskHandle_t KeyProcessTaskHanhler;        //任务句柄
void KeyProcessTask(void *pArg);           //任务函数

#define KEYMSG_Q_NUM   1  //按键消息队列的数量
#define MESSAGE_Q_NUM  4  //发送数据的消息队列的数量

QueueHandle_t KeyQueue;   //按键值消息队列句柄
QueueHandle_t MsgQueue;   //信息队列句柄





int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	uart_init(115200);
	LED_Init();
	TIM5_Int_Init(5000,7200-1);
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
	
	//创建KeyQueue队列
	KeyQueue = xQueueCreate(KEYMSG_Q_NUM,sizeof(u8));
	MsgQueue = xQueueCreate(MESSAGE_Q_NUM,64);
	//创建LED0任务
	
	xTaskCreate((TaskFunction_t) Task1Task,       //任务函数
							(const char*   ) "Task1Task",     //任务名称
							(uint16_t      ) Task1_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) Task1_TASK_PRIO, //任务优先级
							(TaskHandle_t* ) &Task1TaskHanhler//任务句柄
							);
	xTaskCreate((TaskFunction_t) KeyProcessTask,       //任务函数
							(const char*   ) "KeyProcessTask",     //任务名称
							(uint16_t      ) KeyProcess_STK_SIZE,  //任务堆栈大小
							(void *        ) NULL,            //传递给任务函数的参数
							(UBaseType_t   ) KeyProcess_TASK_PRIO, //任务优先级
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
			if((KeyQueue!=0)&&(key))  //消息队列KeyQueue创建成功，并且按键按下
			{
				err = xQueueSend(KeyQueue,&key,10);
				if(err==errQUEUE_FULL)
				{
					printf("The KeyQueue is full, Send data falied!\r\n");
				}
			}
			i++;
			if(i%10==0)
			{
				//检测MsgQueue的容量
			}
			if(i==50)
			{
				i = 0;
				LED0 = !LED0;
			}
		  vTaskDelay(10);
		}
}

void KeyProcessTask(void *pArg)
{
		u8 num,key;
		while(1)
		{
			if(KeyQueue!=0)
			{
				//请求消息KeyQueue
				if(xQueueReceive(KeyQueue,&key,portMAX_DELAY))
				{
					switch(key)
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
			vTaskDelay(10);
		}
}

void CheckMsgQueue(void)
{
	u8   p[20];
	u8 msgQRemainSize;   //消息队列剩余大小
	u8 msgQTotalSize;    //消息队列总大小
	
	taskENTER_CRITICAL();
	msgQRemainSize = uxQueueSpacesAvailable(MsgQueue);   //得到队列剩余大小
	msgQTotalSize = uxQueueMessagesWaiting(MsgQueue)+uxQueueSpacesAvailable(MsgQueue);
	sprintf((char*)p,"Remain Size:%d\r\n",msgQRemainSize);
	printf("%s",p);
	sprintf((char*)p,"Total Size:%d\r\n",msgQTotalSize);
	printf("%s",p);
	taskEXIT_CRITICAL();
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
		if((USART_RX_STA&0x80)&&(MsgQueue!=NULL))
		{
			//向队列中发送数据
			xQueueSendFromISR(MsgQueue,USART_RX_BUF,&xHighPriorityTaskWoken);
			USART_RX_STA = 0;
			memset(USART_RX_BUF,0,64);
			portYIELD_FROM_ISR(xHighPriorityTaskWoken);
		//	printf("Msg Insert\r\n");
		}
} 


