#ifndef __OLED_H
#define __OLED_H 
#include "sys.h"
#include "data_process.h"
#include "stdlib.h"	
 
typedef struct SensorData SensorData_t; // 前向声明，避免循环依赖
 
//----------------OLED端口定义----------------- 
/***************根据自己需求更改****************/
#define OLED_SCL_PROT  			        GPIOB
#define OLED_SCL_PIN				    GPIO_PIN_11
#define OLED_SCL_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 3; }while(0)
#define OLED_SDA_PROT  			        GPIOB
#define OLED_SDA_PIN				    GPIO_PIN_10
#define OLED_SDA_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 3; }while(0)
/*********************END**********************/
 
#define OLED_SCL_Clr() HAL_GPIO_WritePin(OLED_SCL_PROT,OLED_SCL_PIN,0)//SCL
#define OLED_SCL_Set() HAL_GPIO_WritePin(OLED_SCL_PROT,OLED_SCL_PIN,1)
 
#define OLED_SDA_Clr() HAL_GPIO_WritePin(OLED_SDA_PROT,OLED_SDA_PIN,0)//DIN
#define OLED_SDA_Set() HAL_GPIO_WritePin(OLED_SDA_PROT,OLED_SDA_PIN,1)
 
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
 
void OLED_ClearPoint(uint8_t x,uint8_t y);
void OLED_ColorTurn(uint8_t i);
void OLED_DisplayTurn(uint8_t i);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_WaitAck(void);
void OLED_Send_Byte(uint8_t dat);
void OLED_WR_Byte(uint8_t dat,uint8_t mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t mode);
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r);
void OLED_ShowChar(uint8_t x,uint8_t y,char chr,uint8_t size1,uint8_t mode);
void OLED_ShowChar6x8(uint8_t x,uint8_t y,uint8_t chr,uint8_t mode);
void OLED_ShowString(uint8_t x,uint8_t y,char *chr,uint8_t size1,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t mode);
void OLED_ShowfloatNum(uint8_t x, uint8_t y, float num, uint8_t size1, uint8_t mode);
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t mode);
void OLED_ScrollDisplay(uint8_t num,uint8_t space,uint8_t mode);
void OLED_ShowPicture(uint8_t x,uint8_t y,uint8_t sizex,uint8_t sizey,uint8_t BMP[],uint8_t mode);
void OLED_Init(void);
void OLED_Show(SensorData_t *sensorData);
 
#endif
 
