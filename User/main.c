/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : RL-RTX + RL-TCPnet的工程模板的实现。
*              实验目的：
*                1. 学习RL-RTX + RL-TCPnet的工程模板的制作。
*              实验内容：
*                1. 强烈推荐将网线接到路由器或者交换机上面测试，因为已经使能了DHCP，可以自动获取IP地址。
*                2. 创建了一个TCP Server，而且使能了局域网域名NetBIOS，用户只需在电脑端ping armfly
*                   就可以获得板子的IP地址，端口号1001。
*                3. 用户可以在电脑端用网络调试软件创建TCP Client连接此服务器端。
*                4. 按键K1按下，发送8字节的数据给TCP Client。
*                5. 按键K2按下，发送1024字节的数据给TCP Client。
*                6. 按键K3按下，发送5MB字节的数据给TCP Client。
*                7. 各个任务实现的功能如下：
*                   AppTaskUserIF任务 ：按键消息处理。
*                   AppTaskLED任务    ：LED闪烁。
*                   AppTaskMsgPro任务 ：按键检测。
*                   AppTaskTCPMain任务：RL-TCPnet测试任务。
*                   AppTaskStart任务  ：启动任务，也是最高优先级任务，这里实现RL-TCPnet的时间基准更新。
*              注意事项：
*                1. 每个MDK的安装目录里面都会有一个RTX源码，对于MDK4.XX来说，大家使用的
*                   那个MDK版本，务必使用那个MDK版本下面的RTX，这样使用MDK自带的RTX调试组件
*                   时，才能显示正确的调试信息。
*                2. 当前使用的RTX源码是MDK4.74里面的，KEIL官方已经放弃MDK4系列的更新了，
*                   这个版本号是MDK4系列里面最新版本了。如果需要使用MDK自带的RTX调试组件显示
*                   信息，请务必使用MDK4.74.
*                3. 对于MDK5.XX，RTX也在其安装目录里面，但是RTX已经不作为单独版本发布了，
*                   它有一个全新的名字叫CMSIS-RTOS RTX。ARM官方在RTX的基础上给RTX又做了一层封装。
*                4. 本实验推荐使用串口软件SecureCRT，要不串口打印效果不整齐。此软件在
*                   V6开发板光盘里面有。
*                5. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者            说明
*       V1.0    2017-04-26   Eric2013    1. ST固件库1.6.1版本
*                                        2. BSP驱动包V1.2
*                                        3. RL-RTX版本4.74
*                                        4. RL-TCPnet版本V4.74
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "includes.h"		



/*
**********************************************************************************************************
											函数声明
**********************************************************************************************************
*/
static void AppTaskCreate (void);
__task void AppTaskUserIF(void);
__task void AppTaskLED(void);
__task void AppTaskMsgPro(void);
__task void AppTaskTCPMain(void);
__task void AppTaskStart(void);


/*
**********************************************************************************************************
											 变量
**********************************************************************************************************
*/
static uint64_t AppTaskUserIFStk[1024/8];  /* 任务栈 */
static uint64_t AppTaskLEDStk[1024/8];     /* 任务栈 */
static uint64_t AppTaskMsgProStk[1024/8];  /* 任务栈 */
static uint64_t AppTaskTCPMainStk[2048/8]; /* 任务栈 */
static uint64_t AppTaskStartStk[1024/8];   /* 任务栈 */

/* 任务句柄 */
OS_TID HandleTaskUserIF = NULL;
OS_TID HandleTaskMsgPro = NULL;
OS_TID HandleTaskLED = NULL;
OS_TID HandleTaskTCPMain = NULL;


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main (void) 
{	
	/* 初始化外设 */
	bsp_Init();
	
	/* 创建启动任务 */
 	os_sys_init_user (AppTaskStart,              /* 任务函数 */
	                  5,                         /* 任务优先级 */
	                  &AppTaskStartStk,          /* 任务栈 */
	                  sizeof(AppTaskStartStk));  /* 任务栈大小，单位字节数 */
	while(1);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: 按键消息处理		
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 1  (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
__task void AppTaskUserIF(void)
{
	uint8_t ucKeyCode;

    while(1)
    {
		ucKeyCode = bsp_GetKey();
		
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				/* K1键按下，直接发送事件标志给任务AppTaskTCPMain，设置bit0 */
				case KEY_DOWN_K1:
					SEGGER_RTT_printf(0,"K1键按下，直接发送事件标志给任务AppTaskTCPMain，bit0被设置\r\n");
					os_evt_set (KEY1_BIT0, HandleTaskTCPMain);					
					break;	

				/* K2键按下，直接发送事件标志给任务AppTaskTCPMain，设置bit1 */
				case KEY_DOWN_K2:
					SEGGER_RTT_printf(0,"K2键按下，直接发送事件标志给任务AppTaskTCPMain，bit1被设置\r\n");
					os_evt_set (KEY2_BIT1, HandleTaskTCPMain);
					break;
				
				/* K3键按下，直接发送事件标志给任务AppTaskTCPMain，设置bit2 */
				case KEY_DOWN_K3:
					SEGGER_RTT_printf(0,"K3键按下，直接发送事件标志给任务AppTaskTCPMain，bit2被设置\r\n");
					os_evt_set (KEY3_BIT2, HandleTaskTCPMain);
					break;

				/* 其他的键值不处理 */
				default:                     
					break;
			}
		}
		
		os_dly_wait(20);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskLED
*	功能说明: LED闪烁。
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 2  
*********************************************************************************************************
*/
__task void AppTaskLED(void)
{
	const uint16_t usFrequency = 500; /* 延迟周期 */
	
	/* 设置延迟周期 */
	os_itv_set(usFrequency);
	
    while(1)
    {
		bsp_LedToggle(2);

		/* os_itv_wait是绝对延迟，os_dly_wait是相对延迟。*/
		os_itv_wait();
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskMsgPro
*	功能说明: 按键检测
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 3  
*********************************************************************************************************
*/
__task void AppTaskMsgPro(void)
{
    while(1)
    {
		bsp_KeyScan();
		os_dly_wait(10);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskTCPMain
*	功能说明: RL-TCPnet测试任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
__task void AppTaskTCPMain(void)
{
	while (1) 
	{
		TCPnetTest();
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 启动任务，也是最高优先级任务，这里实现RL-TCPnet的时间基准更新。
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 5  
*********************************************************************************************************
*/
__task void AppTaskStart(void)
{
	/* 初始化RL-TCPnet */
	init_TcpNet ();
	
	/* 创建任务 */
	AppTaskCreate();
	
	os_itv_set (100);
	
    while(1)
    {
		os_itv_wait ();
		
		/* RL-TCPnet时间基准更新函数 */
		timer_tick ();
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	HandleTaskUserIF = os_tsk_create_user(AppTaskUserIF,             /* 任务函数 */ 
	                                      1,                         /* 任务优先级 */ 
	                                      &AppTaskUserIFStk,         /* 任务栈 */
	                                      sizeof(AppTaskUserIFStk)); /* 任务栈大小，单位字节数 */
	
	HandleTaskLED = os_tsk_create_user(AppTaskLED,              /* 任务函数 */ 
	                                   2,                       /* 任务优先级 */ 
	                                   &AppTaskLEDStk,          /* 任务栈 */
	                                   sizeof(AppTaskLEDStk));  /* 任务栈大小，单位字节数 */
	
	HandleTaskMsgPro = os_tsk_create_user(AppTaskMsgPro,             /* 任务函数 */ 
	                                      3,                         /* 任务优先级 */ 
	                                      &AppTaskMsgProStk,         /* 任务栈 */
	                                      sizeof(AppTaskMsgProStk)); /* 任务栈大小，单位字节数 */
	
    HandleTaskTCPMain = os_tsk_create_user(AppTaskTCPMain,             /* 任务函数 */ 
	                                      4,                         /* 任务优先级 */ 
	                                      &AppTaskTCPMainStk,         /* 任务栈 */
	                                      sizeof(AppTaskTCPMainStk)); /* 任务栈大小，单位字节数 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
