#ifndef __SYS_H
#define __SYS_H

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

/**
 * @brief  NVIC优先级分组定义
 * @note   对应SCB->AIRCR寄存器的PRIGROUP字段
 */
#define NVIC_PRIORITYGROUP_0    0x00000007U /*!< 0位抢占优先级，4位子优先级 */
#define NVIC_PRIORITYGROUP_1    0x00000006U /*!< 1位抢占优先级，3位子优先级 */
#define NVIC_PRIORITYGROUP_2    0x00000005U /*!< 2位抢占优先级，2位子优先级 */
#define NVIC_PRIORITYGROUP_3    0x00000004U /*!< 3位抢占优先级，1位子优先级 */
#define NVIC_PRIORITYGROUP_4    0x00000003U /*!< 4位抢占优先级，0位子优先级 */

/**
 * @brief  NVIC优先级分组设置（寄存器版本）
 * @param  priority_group: 优先级分组，可选值0-4
 * @retval 无
 */
void Set_NVIC_PriorityGrouping(uint32_t priority_group);

/**
 * @brief  设置指定中断的优先级（寄存器版本）
 * @param  IRQn: 中断号
 * @param  preempt_priority: 抢占优先级
 * @param  sub_priority: 子优先级
 * @retval 无
 */
void Set_NVIC_Priority(IRQn_Type IRQn, uint32_t preempt_priority, uint32_t sub_priority);

/**
 * @brief  使能指定中断（寄存器版本）
 * @param  IRQn: 中断号
 * @retval 无
 */
void Enable_NVIC_IRQ(IRQn_Type IRQn);

/**
 * @brief  禁用指定中断（寄存器版本）
 * @param  IRQn: 中断号
 * @retval 无
 */
void Disable_NVIC_IRQ(IRQn_Type IRQn);

/**
 * @brief  系统NVIC初始化函数
 * @param  无
 * @retval 无
 */
void NVIC_Init(uint8_t preempt_priority, uint8_t sub_priority, IRQn_Type IRQn, uint8_t group);

#endif /* __SYS_H */
