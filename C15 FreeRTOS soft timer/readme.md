
# 软件定时器使用说明
## 注意事项
* 在软件定时器的中断服务函数中不能调用使任务阻塞的函数，比如vTaskDekay();
* 在获取信号量时也不能设置大于0的阻塞时间