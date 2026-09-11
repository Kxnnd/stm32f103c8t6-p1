/**
 * @file    dht11.h
 * @brief   DHT11 温湿度传感器驱动
 */

#ifndef __DHT11_H__
#define __DHT11_H__

#include "main.h"

/* ---------- 引脚 ---------- */
#define DHT11_DQ_PORT       GPIOA
#define DHT11_DQ_PIN        GPIO_PIN_0

/* ---------- 返回值 ---------- */
#define DHT11_OK            0
#define DHT11_ERROR         1

/* ---------- 数据结构 ---------- */
typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checksum;
} DHT11_Data;

/* ---------- 函数 ---------- */
void DWT_Delay_Init(void);
void DWT_Delay_us(uint32_t us);
void DHT11_Init(void);
uint8_t DHT11_Read(DHT11_Data *data);

#endif
