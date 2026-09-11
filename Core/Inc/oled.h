/**
 * @file    oled.h
 * @brief   SSD1306 OLED 驱动头文件（I2C 0.96寸）
 * @note    移植自野火 EBF24030001(小智 F103C8) 驱动，I2C 改用 CubeMX 的 hi2c1
 */

#ifndef __OLED_H__
#define __OLED_H__

#include "main.h"

/* ===================== 宏定义 ===================== */

/* OLED I2C 地址 */
#define OLED_ID          0x78

/* SSD1306 控制字节 */
#define OLED_WR_CMD      0x00    /* 后续为命令 */
#define OLED_WR_DATA     0x40    /* 后续为数据 */

/* ===================== 函数声明 ===================== */

/**
 * @brief  OLED 初始化（在 MX_I2C1_Init 之后调用）
 */
void OLED_Init(void);

/**
 * @brief  设置光标位置
 * @param  x : 列 0~127
 * @param  y : 页 0~7
 */
void OLED_SetPos(unsigned char x, unsigned char y);

/**
 * @brief  全屏填充
 * @param  fill_data : 0x00 全灭，0xFF 全亮
 */
void OLED_Fill(unsigned char fill_data);

/**
 * @brief  清屏
 */
void OLED_CLS(void);

/**
 * @brief  开启显示
 */
void OLED_ON(void);

/**
 * @brief  关闭显示
 */
void OLED_OFF(void);

/**
 * @brief  显示 ASCII 字符串
 * @param  x        : 起始列 0~127
 * @param  y        : 起始页 0~7
 * @param  ch       : 字符串（以 '\0' 结尾）
 * @param  textsize : 1 = 6x8 小字; 2 = 8x16 大字
 */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char textsize);

#endif /* __OLED_H__ */
