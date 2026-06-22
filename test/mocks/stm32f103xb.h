#ifndef __STM32F103XB_MOCK_H
#define __STM32F103XB_MOCK_H

/* Minimal mock for CMSIS STM32F103xB header — provides register layout
 * typedefs needed by sys.h and other BSP headers.  No actual hardware
 * registers are accessed during host-based unit testing. */

#include <stdint.h>

/* Peripheral base addresses (dummy values) */
#define PERIPH_BASE         0x40000000UL
#define APB1PERIPH_BASE     PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x10000UL)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x20000UL)

/* Core register types */
typedef struct {
    volatile uint32_t ISER[8];
    uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];
    uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];
    uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];
    uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];
    uint32_t RESERVED4[56];
    volatile uint8_t  IP[240];
    uint32_t RESERVED5[644];
    volatile uint32_t STIR;
} NVIC_Type;

#define NVIC_BASE           (0xE000E100UL)
#define NVIC                ((NVIC_Type *)NVIC_BASE)

/* SCB system control block */
typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint8_t  SHP[12];
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t DFSR;
    volatile uint32_t MMFAR;
    volatile uint32_t BFAR;
    volatile uint32_t AFSR;
} SCB_Type;

#define SCB_BASE            (0xE000ED00UL)
#define SCB                 ((SCB_Type *)SCB_BASE)

/* DWT data watchpoint */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t CYCCNT;
    volatile uint32_t CPICNT;
    volatile uint32_t EXCCNT;
    volatile uint32_t SLEEPCNT;
    volatile uint32_t LSUCNT;
    volatile uint32_t FOLDCNT;
    volatile uint32_t PCSR;
} DWT_Type;

#define DWT_BASE            (0xE0001000UL)
#define DWT                 ((DWT_Type *)DWT_BASE)

/* GPIO register structure */
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

/* RCC register structure */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

/* USART register structure */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

/* DMA register structure */
typedef struct {
    volatile uint32_t CCR;
    volatile uint32_t CNDTR;
    volatile uint32_t CPAR;
    volatile uint32_t CMAR;
} DMA_Channel_TypeDef;

/* ADC register structure */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_TypeDef;

/* TIM register structure */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

/* AFIO register structure */
typedef struct {
    volatile uint32_t EVCR;
    volatile uint32_t MAPR;
    volatile uint32_t EXTICR[4];
    uint32_t RESERVED0;
    volatile uint32_t MAPR2;
} AFIO_TypeDef;

/* EXTI register structure */
typedef struct {
    volatile uint32_t IMR;
    volatile uint32_t EMR;
    volatile uint32_t RTSR;
    volatile uint32_t FTSR;
    volatile uint32_t SWIER;
    volatile uint32_t PR;
} EXTI_TypeDef;

/* Flash interface */
typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

/* I2C register structure */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_TypeDef;

/* SPI register structure */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t CRCPR;
    volatile uint32_t RXCRCR;
    volatile uint32_t TXCRCR;
    volatile uint32_t I2SCFGR;
    volatile uint32_t I2SPR;
} SPI_TypeDef;

/* GPIO peripheral definitions */
#define GPIOA               ((GPIO_TypeDef *)0x40010800UL)
#define GPIOB               ((GPIO_TypeDef *)0x40010C00UL)
#define GPIOC               ((GPIO_TypeDef *)0x40011000UL)
#define GPIOD               ((GPIO_TypeDef *)0x40011400UL)

/* RCC peripheral */
#define RCC                 ((RCC_TypeDef *)0x40021000UL)

/* USART peripheral */
#define USART1              ((USART_TypeDef *)0x40013800UL)

/* DMA */
#define DMA1                ((DMA_Channel_TypeDef *)0x40020000UL)
#define DMA1_Channel1       ((DMA_Channel_TypeDef *)(DMA1_BASE + 0x08))
#define DMA1_BASE           0x40020000UL

/* ADC */
#define ADC1                ((ADC_TypeDef *)0x40012400UL)
#define ADC2                ((ADC_TypeDef *)0x40012800UL)

/* TIM */
#define TIM2                ((TIM_TypeDef *)0x40000000UL)
#define TIM3                ((TIM_TypeDef *)0x40000400UL)
#define TIM4                ((TIM_TypeDef *)0x40000800UL)

/* AFIO */
#define AFIO                ((AFIO_TypeDef *)0x40010000UL)

/* EXTI */
#define EXTI                ((EXTI_TypeDef *)0x40010400UL)

/* FLASH */
#define FLASH               ((FLASH_TypeDef *)0x40022000UL)

/* I2C */
#define I2C1                ((I2C_TypeDef *)0x40005400UL)
#define I2C2                ((I2C_TypeDef *)0x40005800UL)

/* SPI */
#define SPI1                ((SPI_TypeDef *)0x40013000UL)
#define SPI2                ((SPI_TypeDef *)0x40003800UL)

/* Interrupt numbers */
typedef enum {
    WWDG_IRQn            = 0,
    PVD_IRQn             = 1,
    TAMPER_IRQn          = 2,
    RTC_IRQn             = 3,
    FLASH_IRQn           = 4,
    RCC_IRQn             = 5,
    EXTI0_IRQn           = 6,
    EXTI1_IRQn           = 7,
    EXTI2_IRQn           = 8,
    EXTI3_IRQn           = 9,
    EXTI4_IRQn           = 10,
    DMA1_Channel1_IRQn   = 11,
    DMA1_Channel2_IRQn   = 12,
    DMA1_Channel3_IRQn   = 13,
    DMA1_Channel4_IRQn   = 14,
    DMA1_Channel5_IRQn   = 15,
    DMA1_Channel6_IRQn   = 16,
    DMA1_Channel7_IRQn   = 17,
    ADC1_2_IRQn          = 18,
    USB_HP_CAN1_TX_IRQn  = 19,
    USB_LP_CAN1_RX0_IRQn = 20,
    CAN1_RX1_IRQn        = 21,
    CAN1_SCE_IRQn        = 22,
    EXTI9_5_IRQn         = 23,
    TIM1_BRK_IRQn        = 24,
    TIM1_UP_IRQn         = 25,
    TIM1_TRG_COM_IRQn    = 26,
    TIM1_CC_IRQn         = 27,
    TIM2_IRQn            = 28,
    TIM3_IRQn            = 29,
    TIM4_IRQn            = 30,
    I2C1_EV_IRQn         = 31,
    I2C1_ER_IRQn         = 32,
    I2C2_EV_IRQn         = 33,
    I2C2_ER_IRQn         = 34,
    SPI1_IRQn            = 35,
    SPI2_IRQn            = 36,
    USART1_IRQn          = 37,
    USART2_IRQn          = 38,
    USART3_IRQn          = 39,
    EXTI15_10_IRQn       = 40,
    RTCAlarm_IRQn        = 41,
    USBWakeUp_IRQn       = 42,
    TIM8_BRK_IRQn        = 43,
    TIM8_UP_IRQn         = 44,
    TIM8_TRG_COM_IRQn    = 45,
    TIM8_CC_IRQn         = 46,
    ADC3_IRQn            = 47,
    FMCS_IRQn            = 48,
    SDIO_IRQn            = 49,
    TIM5_IRQn            = 50,
    SPI3_IRQn            = 51,
    UART4_IRQn           = 52,
    UART5_IRQn           = 53,
    TIM6_IRQn            = 54,
    TIM7_IRQn            = 55,
    DMA2_Channel1_IRQn   = 56,
    DMA2_Channel2_IRQn   = 57,
    DMA2_Channel3_IRQn   = 58,
    DMA2_Channel4_IRQn   = 59,
    DMA2_Channel5_IRQn   = 60,
    ETH_IRQn             = 61,
    ETH_WKUP_IRQn        = 62,
    CAN2_TX_IRQn         = 63,
    CAN2_RX0_IRQn        = 64,
    CAN2_RX1_IRQn        = 65,
    CAN2_SCE_IRQn        = 66,
    OTG_FS_IRQn          = 67,
} IRQn_Type;

/* Core register access macros */
#define __I                 volatile const
#define __O                 volatile
#define __IO                volatile

/* Memory barrier */
#define __DSB()             do { } while(0)
#define __ISB()             do { } while(0)

/* System init function */
void SystemInit(void);

#endif /* __STM32F103XB_MOCK_H */
