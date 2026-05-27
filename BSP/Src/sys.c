#include "sys.h"

/**
 * @brief  NVIC优先级分组设置（寄存器版本）
 * @note   配置NVIC优先级分组，STM32F103使用4位优先级
 * @param  priority_group: 优先级分组，可选值0-7
 *         0: 0位抢占优先级，4位子优先级
 *         1: 1位抢占优先级，3位子优先级
 *         2: 2位抢占优先级，2位子优先级
 *         3: 3位抢占优先级，1位子优先级
 *         4: 4位抢占优先级，0位子优先级
 *         5-7: 保留
 * @retval 无
 */
void Set_NVIC_PriorityGrouping(uint32_t priority_group)
{
    uint32_t reg_value;
    
    /* 读取SCB->AIRCR寄存器当前值 */
    reg_value = SCB->AIRCR;
    
    /* 清除VECTKEY和PRIGROUP字段，保留其他位 */
    reg_value &= ~(SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_PRIGROUP_Msk);
    
    /* 写入VECTKEY（0x5FA）和新的优先级分组 */
    reg_value |= (0x5FAUL << SCB_AIRCR_VECTKEY_Pos) | 
                 ((priority_group & 0x07UL) << SCB_AIRCR_PRIGROUP_Pos);
    
    /* 写回SCB->AIRCR寄存器 */
    SCB->AIRCR = reg_value;
}

/**
 * @brief  设置指定中断的优先级（寄存器版本）
 * @note   根据优先级分组计算抢占优先级和子优先级
 * @param  IRQn: 中断号（IRQn_Type枚举）
 * @param  preempt_priority: 抢占优先级
 * @param  sub_priority: 子优先级
 * @retval 无
 */
void Set_NVIC_Priority(IRQn_Type IRQn, uint32_t preempt_priority, uint32_t sub_priority)
{
    uint32_t priority_group;
    uint32_t priority;
    uint32_t preempt_bits;
    uint32_t sub_bits;
    
    /* 获取当前优先级分组 */
    priority_group = (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) >> SCB_AIRCR_PRIGROUP_Pos;
    
    /* 根据分组计算抢占优先级和子优先级的位数 */
    /* STM32F103使用4位优先级，PRIGROUP值与位数关系：
     * PRIGROUP=0: 抢占1位(0-1), 子3位(0-7)  -> 实际：抢占0位, 子4位
     * PRIGROUP=1: 抢占2位(0-3), 子2位(0-3)  -> 实际：抢占1位, 子3位
     * PRIGROUP=2: 抢占3位(0-7), 子1位(0-1)  -> 实际：抢占2位, 子2位
     * PRIGROUP=3: 抢占4位(0-15), 子0位      -> 实际：抢占3位, 子1位
     * PRIGROUP=4: 抢占5位(0-31), 子-1位     -> 实际：抢占4位, 子0位
     * 
     * 注意：实际可编程优先级位数是4位，所以：
     * PRIGROUP=0: 0位抢占, 4位子
     * PRIGROUP=1: 1位抢占, 3位子
     * PRIGROUP=2: 2位抢占, 2位子
     * PRIGROUP=3: 3位抢占, 1位子
     * PRIGROUP=4: 4位抢占, 0位子
     */
    preempt_bits = 7 - priority_group;
    if (preempt_bits > 4) preempt_bits = 4;
    sub_bits = 4 - preempt_bits;
    
    /* 计算优先级值（左对齐到8位寄存器的高4位） */
    priority = ((preempt_priority & ((1UL << preempt_bits) - 1)) << sub_bits) | 
               (sub_priority & ((1UL << sub_bits) - 1));
    priority = (priority << (8 - 4)) & 0xFFUL;
    
    /* 设置中断优先级 */
    if (IRQn >= 0) {
        /* 外部中断（IRQn >= 0） */
        NVIC->IP[IRQn] = (uint8_t)priority;
    } else {
        /* 内核中断（IRQn < 0） */
        SCB->SHP[((uint32_t)IRQn & 0x0FUL) - 4UL] = (uint8_t)priority;
    }
}

/**
 * @brief  使能指定中断（寄存器版本）
 * @note   通过设置NVIC->ISER寄存器使能中断
 * @param  IRQn: 中断号（IRQn_Type枚举）
 * @retval 无
 */
void Enable_NVIC_IRQ(IRQn_Type IRQn)
{
    if (IRQn >= 0) {
        /* 计算寄存器索引和位位置 */
        /* ISER[0]: IRQ 0-31, ISER[1]: IRQ 32-63, ISER[2]: IRQ 64-95 */
        NVIC->ISER[((uint32_t)IRQn >> 5UL)] = (1UL << ((uint32_t)IRQn & 0x1FUL));
    }
}

/**
 * @brief  禁用指定中断（寄存器版本）
 * @note   通过设置NVIC->ICER寄存器禁用中断
 * @param  IRQn: 中断号（IRQn_Type枚举）
 * @retval 无
 */
void Disable_NVIC_IRQ(IRQn_Type IRQn)
{
    if (IRQn >= 0) {
        /* 计算寄存器索引和位位置 */
        NVIC->ICER[((uint32_t)IRQn >> 5UL)] = (1UL << ((uint32_t)IRQn & 0x1FUL));
    }
}

/**
 * @brief  系统NVIC初始化函数
 * @note   配置NVIC优先级分组和常用中断优先级
 * @param  无
 * @retval 无
 */
void NVIC_Init(uint8_t preempt_priority, uint8_t sub_priority, IRQn_Type IRQn, uint8_t group)
{
    /* 设置优先级分组为组2：2位抢占优先级，2位子优先级 */
    /* 抢占优先级范围：0-3，子优先级范围：0-3 */
    Set_NVIC_PriorityGrouping(group);
    
    /* 配置外设中断优先级示例 */
    /* EXTI0_IRQn: 外部中断线0，抢占优先级1，子优先级0 */
    Set_NVIC_Priority(IRQn, preempt_priority, sub_priority);
    Enable_NVIC_IRQ(IRQn);
}
