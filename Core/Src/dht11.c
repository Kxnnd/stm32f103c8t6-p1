#include "dht11.h"

static uint32_t DWT_START = 0;

void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DWT_Delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;   /* 局部变量，不再污染全局 DWT_START */
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

static void DQ_Mode_PP(void) { GPIOA->CRL &= ~(0x0F << 0); GPIOA->CRL |=  (0x03 << 0); }
static void DQ_Mode_In(void) { GPIOA->CRL &= ~(0x0F << 0); GPIOA->CRL |=  (0x04 << 0); }

#define DQ_H()    (DHT11_DQ_PORT->BSRR = DHT11_DQ_PIN)
#define DQ_L()    (DHT11_DQ_PORT->BRR  = DHT11_DQ_PIN)
#define DQ_RD()   ((DHT11_DQ_PORT->IDR & DHT11_DQ_PIN) ? 1 : 0)

void DHT11_Init(void) { DQ_Mode_PP(); DQ_H(); DWT_Delay_us(1000000); }

static uint8_t DHT11_Start(void)
{
    uint32_t to;
    DQ_Mode_PP(); DQ_L(); DWT_Delay_us(18000);
    DQ_H(); DWT_Delay_us(30);
    DQ_Mode_In();
    to=0; while(DQ_RD()==1){if(++to>1000)return 1;DWT_Delay_us(1);}
    to=0; while(DQ_RD()==0){if(++to>1000)return 1;DWT_Delay_us(1);}
    to=0; while(DQ_RD()==1){if(++to>1000)return 1;DWT_Delay_us(1);}
    return 0;
}

static uint8_t DHT11_Read_Bit(void)
{
    uint32_t to=0;
    while(DQ_RD()==0){if(++to>500)return 0;DWT_Delay_us(1);}
    DWT_START=DWT->CYCCNT; to=0;
    while(DQ_RD()==1){if(++to>500)return 0;DWT_Delay_us(1);}
    return ((DWT->CYCCNT-DWT_START)/(SystemCoreClock/1000000)>45)?1:0;
}

static uint8_t DHT11_Read_Byte(void)
{
    uint8_t b=0;
    for(uint8_t i=0;i<8;i++){b<<=1;b|=DHT11_Read_Bit();}
    return b;
}

uint8_t DHT11_Read(DHT11_Data *data)
{
    uint8_t buf[5];
    __disable_irq();
    if(DHT11_Start()){__enable_irq();return 1;}
    for(uint8_t i=0;i<5;i++)buf[i]=DHT11_Read_Byte();
    __enable_irq();
    if(buf[0]+buf[1]+buf[2]+buf[3]!=buf[4])return 1;
    data->humidity_int=buf[0];data->humidity_dec=buf[1];
    data->temp_int=buf[2];data->temp_dec=buf[3];
    data->checksum=buf[4];
    DQ_Mode_PP();DQ_H();
    return 0;
}
