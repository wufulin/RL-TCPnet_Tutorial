/**
  ******************************************************************************
  * @file    bsp_rtt.c
  * @author  wufulin
  * @version V1.0
  * @date    2018-07-20
  * 
  * Copyright (C), 2018-2019, wufulin
  ******************************************************************************
  */

#include "bsp_rtt.h"

#ifndef NULL
  #define NULL 0
#endif

/**
   ******************************************************************************
   *  函 数 名: bsp_RTT_Init
   *  功能说明: 配置SEGGER_RTT选项
   *  形    参: 无
   *  返 回 值: 无
   ******************************************************************************
   */
void bsp_RTT_Init(void)
{
  // 默认使用Terminal(0)
  SEGGER_RTT_SetTerminal(0);

  // 配置通道0，上行配置
  SEGGER_RTT_ConfigUpBuffer(0, "RTTUP", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

  // 配置通道0，下行配置
  SEGGER_RTT_ConfigDownBuffer(0, "RTTDOWN", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
}
