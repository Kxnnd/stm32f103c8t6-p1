/**
 * @file    oled.c
 * @brief   SSD1306 OLED 驱动（I2C 0.96寸）
 * @note    移植自野火 EBF24030001 小智 F103C8 HAL 例程
 *          硬件 I2C 使用 CubeMX 生成的 I2C1(hi2c1, PB6/PB7)
 */

#include "oled.h"
#include "oled_font.h"
#include "i2c.h"

/* ===================== 底层写命令/写数据 ===================== */

/* 写数据到 OLED */
static void Oled_Write_Data(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_ID, OLED_WR_DATA, I2C_MEMADD_SIZE_8BIT, &data, 1, 0x100);
}

/* 写命令到 OLED */
static void Oled_Write_Cmd(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_ID, OLED_WR_CMD, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 0x100);
}

/* ===================== OLED 初始化 ===================== */

void OLED_Init(void)
{
    /* 关闭显示 */
    Oled_Write_Cmd(0xAE);

    /* 设置显示对比度：0~255，值越大越亮 */
    Oled_Write_Cmd(0x81);
    Oled_Write_Cmd(0xFF);

    /* 恢复 RAM 内容显示 */
    Oled_Write_Cmd(0xA4);

    /* 正常显示模式（非反显） */
    Oled_Write_Cmd(0xA6);

    /* 关闭滚动 */
    Oled_Write_Cmd(0x2E);

    /* 水平滚动设置 */
    Oled_Write_Cmd(0x26);
    Oled_Write_Cmd(0x00);
    Oled_Write_Cmd(0x00);
    Oled_Write_Cmd(0x03);
    Oled_Write_Cmd(0x07);
    Oled_Write_Cmd(0x00);
    Oled_Write_Cmd(0xFF);

    /* 设置寻址模式 */
    Oled_Write_Cmd(0x20);
    Oled_Write_Cmd(0x10);   /* 页寻址模式 */

    /* 页起始地址 */
    Oled_Write_Cmd(0xB0);
    /* 列起始地址低位 */
    Oled_Write_Cmd(0x00);
    /* 列起始地址高位 */
    Oled_Write_Cmd(0x10);

    /* 设置显示起始行 0 */
    Oled_Write_Cmd(0x40);

    /* 段重映射（列0映射到段127） */
    Oled_Write_Cmd(0xA1);

    /* 设置多路复用比（1/64） */
    Oled_Write_Cmd(0xA8);
    Oled_Write_Cmd(0x3F);

    /* COM 扫描方向（从 COM63 到 COM0） */
    Oled_Write_Cmd(0xC8);

    /* 设置显示偏移为 0 */
    Oled_Write_Cmd(0xD3);
    Oled_Write_Cmd(0x00);

    /* COM 引脚硬件配置 */
    Oled_Write_Cmd(0xDA);
    Oled_Write_Cmd(0x12);

    /* 设置时钟分频/振荡频率 */
    Oled_Write_Cmd(0xD9);
    Oled_Write_Cmd(0x22);

    /* 设置 VCOMH 电压 */
    Oled_Write_Cmd(0xDB);
    Oled_Write_Cmd(0x20);

    /* 开启电荷泵 */
    Oled_Write_Cmd(0x8D);
    Oled_Write_Cmd(0x14);

    /* 开启显示 */
    Oled_Write_Cmd(0xAF);
}

/* ===================== 光标定位 ===================== */

void OLED_SetPos(unsigned char x, unsigned char y) /* 设置起始坐标 */
{
    Oled_Write_Cmd(0xb0 + y);                       /* 页地址 0~7 */
    Oled_Write_Cmd(((x & 0xf0) >> 4) | 0x10);       /* 列高 4 位 */
    Oled_Write_Cmd((x & 0x0f) | 0x01);              /* 列低 4 位（+1 偏移，兼容本屏） */
}

/* ===================== 填充/清屏 ===================== */

void OLED_Fill(unsigned char fill_data) /* 全屏填充 */
{
    unsigned char m, n;
    for (m = 0; m < 8; m++)            /* 8 页 */
    {
        Oled_Write_Cmd(0xb0 + m);      /* 页地址 page0~page7 */
        Oled_Write_Cmd(0x00);          /* 列起始地址低位 */
        Oled_Write_Cmd(0x10);          /* 列起始地址高位 */
        for (n = 0; n < 128; n++)      /* 每页 128 列 */
        {
            Oled_Write_Data(fill_data);
        }
    }
}

void OLED_CLS(void) /* 清屏 */
{
    OLED_Fill(0x00);
}

/* ===================== 开关显示 ===================== */

void OLED_ON(void)
{
    Oled_Write_Cmd(0X8D);   /* 设置电荷泵 */
    Oled_Write_Cmd(0X14);   /* 开启电荷泵 */
    Oled_Write_Cmd(0XAF);   /* OLED 开启 */
}

void OLED_OFF(void)
{
    Oled_Write_Cmd(0X8D);   /* 设置电荷泵 */
    Oled_Write_Cmd(0X10);   /* 关闭电荷泵 */
    Oled_Write_Cmd(0XAE);   /* OLED 关闭 */
}

/* ===================== 显示字符串 ===================== */

/**
 * @note  textsize = 1 : 6x8 小字，每字符宽 6 列
 *        textsize = 2 : 8x16 大字，每字符宽 8 列，占上下两页
 */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char textsize)
{
    unsigned char c = 0, i = 0, j = 0;
    switch (textsize)
    {
        case 1:     /* 6x8 小字 */
        {
            while (ch[j] != '\0')
            {
                c = ch[j] - 32;         /* 字库从空格(ASCII 32)开始 */
                if (x > 126)            /* 超出行宽则换页 */
                {
                    x = 0;
                    y++;
                }
                OLED_SetPos(x, y);
                for (i = 0; i < 6; i++)
                    Oled_Write_Data(F6x8[c][i]);
                x += 6;
                j++;
            }
        }
        break;
        case 2:     /* 8x16 大字 */
        {
            while (ch[j] != '\0')
            {
                c = ch[j] - 32;
                if (x > 120)            /* 超出行宽则换页 */
                {
                    x = 0;
                    y++;
                }
                OLED_SetPos(x, y);              /* 上半页 */
                for (i = 0; i < 8; i++)
                    Oled_Write_Data(F8X16[c * 16 + i]);
                OLED_SetPos(x, y + 1);          /* 下半页 */
                for (i = 0; i < 8; i++)
                    Oled_Write_Data(F8X16[c * 16 + i + 8]);
                x += 8;
                j++;
            }
        }
        break;
    }
}

