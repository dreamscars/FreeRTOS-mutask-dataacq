#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "stm32f1xx_hal.h"
 
static uint8_t OLED_GRAM[128][8];
 
/**
 * @brief  设置 OLED 正常显示或反色显示
 * @param  i : 0 -> 正常显示, 1 -> 反色显示
 */
void OLED_ColorTurn(uint8_t i)
{
    if(i == 0)
        OLED_WR_Byte(0xA6, OLED_CMD);   // 正常显示
    if(i == 1)
        OLED_WR_Byte(0xA7, OLED_CMD);   // 反色显示
}

/**
 * @brief  设置 OLED 屏幕旋转 180 度
 * @param  i : 0 -> 正常方向, 1 -> 旋转 180 度
 */
void OLED_DisplayTurn(uint8_t i)
{
    if(i == 0) {
        OLED_WR_Byte(0xC8, OLED_CMD);   // 正常方向（COM 扫描）
        OLED_WR_Byte(0xA1, OLED_CMD);   // 正常方向（SEG 映射）
    }
    if(i == 1) {
        OLED_WR_Byte(0xC0, OLED_CMD);   // 反转方向（COM 扫描）
        OLED_WR_Byte(0xA0, OLED_CMD);   // 反转方向（SEG 映射）
    }
}

/**
 * @brief  简单的 I2C 延时，用于产生时序
 */
void IIC_delay(void)
{
    volatile uint8_t t = 20;
    while(t--);
}

/**
 * @brief  I2C 起始信号：SCL 高电平时，SDA 由高变低
 */
void OLED_I2C_Start(void)
{
    OLED_SDA_Set();     // SDA 置高
    OLED_SCL_Set();     // SCL 置高
    IIC_delay();
    OLED_SDA_Clr();     // SDA 拉低
    IIC_delay();
    OLED_SCL_Clr();     // SCL 拉低，准备发送数据
    IIC_delay();
}

/**
 * @brief  I2C 停止信号：SCL 高电平时，SDA 由低变高
 */
void OLED_I2C_Stop(void)
{
    OLED_SDA_Clr();     // SDA 拉低
    OLED_SCL_Set();     // SCL 置高
    IIC_delay();
    OLED_SDA_Set();     // SDA 拉高，产生停止条件
}

/**
 * @brief  等待从机应答信号（检测 SDA 电平）
 */
void OLED_I2C_WaitAck(void)
{
    OLED_SDA_Set();     // 释放 SDA 总线，由从机控制
    IIC_delay();
    OLED_SCL_Set();     // SCL 高电平期间读取 SDA
    IIC_delay();
    OLED_SCL_Clr();     // 拉低 SCL 完成应答位
    IIC_delay();
}

/**
 * @brief  通过 I2C 发送一个字节（高位在前）
 * @param  dat : 要发送的字节数据
 */
void OLED_Send_Byte(uint8_t dat)
{
    uint8_t i;
    for(i = 0; i < 8; i++) {
        if(dat & 0x80)          // 从最高位开始发送
            OLED_SDA_Set();
        else
            OLED_SDA_Clr();
        IIC_delay();
        OLED_SCL_Set();         // SCL 上升沿，从机采样数据
        IIC_delay();
        OLED_SCL_Clr();         // 拉低 SCL，准备下一位
        dat <<= 1;
    }
}

/**
 * @brief  向 OLED 写入一个字节，可指定为命令或数据
 * @param  dat  : 要写入的字节
 * @param  mode : 0 -> 命令, 1 -> 数据
 */
void OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
    OLED_I2C_Start();
    OLED_Send_Byte(0x78);       // OLED 设备地址（写操作）
    OLED_I2C_WaitAck();
    if(mode)
        OLED_Send_Byte(0x40);   // 控制字节：后续是数据
    else
        OLED_Send_Byte(0x00);   // 控制字节：后续是命令
    OLED_I2C_WaitAck();
    OLED_Send_Byte(dat);        // 发送实际数据/命令
    OLED_I2C_WaitAck();
    OLED_I2C_Stop();
}

/**
 * @brief  开启 OLED 显示（点亮屏幕）
 */
void OLED_DisPlay_On(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);   // 电荷泵使能命令
    OLED_WR_Byte(0x14, OLED_CMD);   // 开启电荷泵
    OLED_WR_Byte(0xAF, OLED_CMD);   // 点亮屏幕（退出睡眠模式）
}

/**
 * @brief  关闭 OLED 显示（熄灭屏幕）
 */
void OLED_DisPlay_Off(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);   // 电荷泵使能命令
    OLED_WR_Byte(0x10, OLED_CMD);   // 关闭电荷泵
    OLED_WR_Byte(0xAE, OLED_CMD);   // 关闭屏幕（进入睡眠模式）
}

/**
 * @brief  将显存数组 OLED_GRAM 的内容全部刷新到 OLED 屏幕
 * @note   OLED_GRAM[128][8] 对应 128 列 × 8 页（每页 8 行，共 64 行）
 */
void OLED_Refresh(void)
{
    uint8_t i, n;
    for(i = 0; i < 8; i++) {
        OLED_WR_Byte(0xB0 + i, OLED_CMD);   // 设置页地址（行起始）
        OLED_WR_Byte(0x00, OLED_CMD);       // 设置列地址低四位（起始列低4位）
        OLED_WR_Byte(0x10, OLED_CMD);       // 设置列地址高四位（起始列高4位）
        OLED_I2C_Start();
        OLED_Send_Byte(0x78);               // 设备地址
        OLED_I2C_WaitAck();
        OLED_Send_Byte(0x40);               // 数据模式
        OLED_I2C_WaitAck();
        for(n = 0; n < 128; n++) {
            OLED_Send_Byte(OLED_GRAM[n][i]); // 发送该页整行数据
            OLED_I2C_WaitAck();
        }
        OLED_I2C_Stop();
    }
}

void OLED_Refresh_Designated_area(uint8_t x, uint8_t y, uint8_t size, uint8_t char_len)
{
	uint8_t i, n, y_sta, x_sta, base_ynum, base_xnum;
	if(size % 8 == 0) base_ynum = size / 8;
	else base_ynum = size / 8 + 1;
	base_xnum = (size / 2) * char_len;
	y_sta = y / 8;
	x_sta = x;
	for(i = y_sta; i < y_sta+base_ynum; i++) {
			OLED_WR_Byte(0xB0 + i, OLED_CMD);   // 设置页地址（行起始）
			OLED_WR_Byte(0x00|(x & 0x0f), OLED_CMD);       // 设置列地址低四位（起始列低4位）
			OLED_WR_Byte(0x10|((x >> 4) & 0x0F), OLED_CMD);       // 设置列地址高四位（起始列高4位）
			OLED_I2C_Start();
			OLED_Send_Byte(0x78);               // 设备地址
			OLED_I2C_WaitAck();
			OLED_Send_Byte(0x40);               // 数据模式
			OLED_I2C_WaitAck();
			for(n = x_sta; n < x_sta+base_xnum; n++) {
					OLED_Send_Byte(OLED_GRAM[n][i]); // 发送该页整行数据
					OLED_I2C_WaitAck();
			}
			OLED_I2C_Stop();
	}
}

/**
 * @brief  清屏：将显存全部清零，并刷新屏幕
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for(i = 0; i < 8; i++) {
        for(n = 0; n < 128; n++) {
            OLED_GRAM[n][i] = 0;            // 清除显存数据
        }
    }
    OLED_Refresh();                         // 更新屏幕
}

/**
 * @brief  在指定坐标画点或清除点
 * @param  x : 列坐标 (0~127)
 * @param  y : 行坐标 (0~63)
 * @param  t : 1 -> 画点（点亮）, 0 -> 清除点（熄灭）
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t i, m, n;
    i = y / 8;          // 计算所在页（0~7）
    m = y % 8;          // 计算在页内的位偏移
    n = 1 << m;         // 对应位的掩码
    if(t) {
        OLED_GRAM[x][i] |= n;               // 置1
    } else {
        OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
        OLED_GRAM[x][i] |= n;
        OLED_GRAM[x][i] = ~OLED_GRAM[x][i]; // 等价于清0对应位
    }
    // 注意：此处未调用 OLED_Refresh，需外部调用或批量刷新
}

/**
 * @brief  画线（Bresenham 算法）
 * @param  x1,y1 : 起点坐标
 * @param  x2,y2 : 终点坐标
 * @param  mode  : 1 -> 画线（使用画点函数点亮）, 0 -> 擦除线（但模式实际传给画点）
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if(delta_x > 0) incx = 1;
    else if(delta_x == 0) incx = 0;
    else { incx = -1; delta_x = -delta_x; }
    if(delta_y > 0) incy = 1;
    else if(delta_y == 0) incy = 0;
    else { incy = -1; delta_y = -delta_y; }
    if(delta_x > delta_y) distance = delta_x;
    else distance = delta_y;
    for(t = 0; t <= distance; t++) {
        OLED_DrawPoint(uRow, uCol, mode);
        xerr += delta_x;
        yerr += delta_y;
        if(xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if(yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/**
 * @brief  画圆（中点圆算法）
 * @param  x : 圆心列坐标
 * @param  y : 圆心行坐标
 * @param  r : 半径
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
    int a = 0, b = r, num;
    while(2 * b * b >= r * r) {
        OLED_DrawPoint(x + a, y - b, 1);
        OLED_DrawPoint(x - a, y - b, 1);
        OLED_DrawPoint(x - a, y + b, 1);
        OLED_DrawPoint(x + a, y + b, 1);
        OLED_DrawPoint(x + b, y + a, 1);
        OLED_DrawPoint(x + b, y - a, 1);
        OLED_DrawPoint(x - b, y - a, 1);
        OLED_DrawPoint(x - b, y + a, 1);
        a++;
        num = (a * a + b * b) - r * r;
        if(num > 0) {
            b--;
            a--;
        }
    }
}


//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//size1:选择字体 6x8/6x12/8x16/12x24
//mode:0,反色显示;1,正常显示
void OLED_ShowChar(uint8_t x,uint8_t y, char chr,uint8_t size1,uint8_t mode)
{
	uint8_t i,m,temp,size2;
	uint8_t chr1;
	uint8_t x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
	chr1=(uint8_t)(chr-' ');  //计算偏移后的值
	for(i=0;i<size2;i++)
	{
		if(size1==8)
			  {temp=asc2_0806[chr1][i];} //调用0806字体
		else if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字体
		else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字体
		else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字体
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
  }
	//OLED_Refresh();
}


/**
 * @brief  显示字符串
 * @param  x,y    : 起始坐标
 * @param  chr    : 字符串指针
 * @param  size1  : 字体大小
 * @param  mode   : 显示模式（0反色，1正常）
 */
void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t size1, uint8_t mode)
{
    while((*chr >= ' ') && (*chr <= '~')) {
        OLED_ShowChar(x, y, *chr, size1, mode);
        if(size1 == 8) x += 6;
        else x += size1/2;
        chr++;
    }
    OLED_Refresh();
}

/**
 * @brief  计算 m 的 n 次方（用于数字显示）
 * @param  m : 底数
 * @param  n : 指数
 * @return 结果
 */
uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while(n--) result *= m;
    return result;
}

/**
 * @brief  显示数字（十进制）
 * @param  x,y   : 起始坐标
 * @param  num   : 要显示的数字
 * @param  len   : 数字的位数（不足补前导零）
 * @param  size1 : 字体大小
 * @param  mode  : 显示模式
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode)
{
    uint8_t t, temp, m = 0;
    if(size1 == 8) m = 2;   // 特殊偏移？实际代码中未详细解释，保留原样
    for(t = 0; t < len; t++) {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        if(temp == 0)
            OLED_ShowChar(x + (size1/2 + m) * t, y, '0', size1, mode);
        else
            OLED_ShowChar(x + (size1/2 + m) * t, y, temp + '0', size1, mode);
    }
    //OLED_Refresh();
	OLED_Refresh_Designated_area(x, y, size1, len);
}

void OLED_ShowfloatNum(uint8_t x, uint8_t y, float num, uint8_t size1, uint8_t mode)
{
    uint8_t m;
    uint32_t int_part = (uint32_t)num; // 整数部分
    float decimal_part = num - int_part; // 小数部分
    uint32_t decimal_int = (uint32_t)(decimal_part * OLED_Pow(10, 2)); // 小数部分转换为整数
    if(size1 == 8) m = 2; // 调整字体大小偏移
    OLED_ShowNum(x, y, int_part, 4, size1, mode); // 显示整数部分，假设最多4位
    OLED_ShowChar(x + (size1/2 + m) * 4, y, '.', size1, mode); // 显示小数点
    OLED_ShowNum(x + (size1/2 + m) * 5, y, decimal_int, 2, size1, mode); // 显示小数部分
}

//#if 0
/**
 * @brief  显示汉字
 * @param  x,y   : 起始坐标
 * @param  num   : 汉字在字库数组中的索引
 * @param  size1 : 汉字字体大小（16,24,32,64）
 * @param  mode  : 显示模式
 */
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode)
{
    uint8_t m, temp;
    uint8_t x0 = x, y0 = y;
    uint16_t i, size3 = (size1/8 + ((size1%8)?1:0)) * size1; // 汉字点阵总字节数
    for(i = 0; i < size3; i++) {
        if(size1 == 16)     temp = Hzk1[num][i];
        else if(size1 == 24)temp = Hzk2[num][i];
        else if(size1 == 32)temp = Hzk3[num][i];
        else if(size1 == 64)temp = Hzk4[num][i];
        else return;
        for(m = 0; m < 8; m++) {
            if(temp & 0x01) OLED_DrawPoint(x, y, mode);
            else            OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if((x - x0) == size1) {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
    OLED_Refresh();
}
//#endif

/**
 * @brief  滚动显示多个汉字（左移循环）
 * @param  num   : 要显示的汉字个数
 * @param  space : 每屏显示之间的间隔帧数
 * @param  mode  : 显示模式
 * @note   该函数为无限循环，使用时需注意
 */
void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode)
{
    uint8_t i, n, t = 0, m = 0, r;
    while(1) {
        if(m == 0) {
            OLED_ShowChinese(128, 24, t, 16, mode); // 在屏幕外右侧写入汉字
            t++;
        }
        if(t == num) {
            for(r = 0; r < 16 * space; r++) {       // 间隔等待
                for(i = 1; i < 144; i++) {
                    for(n = 0; n < 8; n++) {
                        OLED_GRAM[i-1][n] = OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh();
            }
            t = 0;
        }
        m++;
        if(m == 16) m = 0;
        // 整体左移一列
        for(i = 1; i < 144; i++) {
            for(n = 0; n < 8; n++) {
                OLED_GRAM[i-1][n] = OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

/**
 * @brief  显示图片（位图）
 * @param  x,y    : 起始坐标
 * @param  sizex  : 图片宽度（像素）
 * @param  sizey  : 图片高度（像素）
 * @param  BMP[]  : 图片数据数组（按页存储，高位在上）
 * @param  mode   : 显示模式
 */
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode)
{
    uint16_t j = 0;
    uint8_t i, n, temp, m;
    uint8_t x0 = x, y0 = y;
    sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);  // 高度转换为页数
    for(n = 0; n < sizey; n++) {
        for(i = 0; i < sizex; i++) {
            temp = BMP[j++];
            for(m = 0; m < 8; m++) {
                if(temp & 0x01) OLED_DrawPoint(x, y, mode);
                else            OLED_DrawPoint(x, y, !mode);
                temp >>= 1;
                y++;
            }
            x++;
            if((x - x0) == sizex) {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
    OLED_Refresh();
}

/**
 * @brief  OLED 初始化函数
 * @note   配置 I2C 引脚，发送一系列初始化命令，清屏并点亮屏幕
 */
void OLED_Init(void)
{
    // HAL库等价实现
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();  // 使能GPIOB时钟

    GPIO_InitStruct.Pin   = GPIO_PIN_11 | GPIO_PIN_10;  // PB11(SCL) + PB10(SDA)
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;        // 推挽输出
    GPIO_InitStruct.Pull  = GPIO_NOPULL;                // 无上下拉电阻
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;    // 中等速度

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);		

    HAL_Delay(200);      // 等待 OLED 上电稳定

    // 以下为 SSD1306 的初始化命令序列
    OLED_WR_Byte(0xAE, OLED_CMD);   // 关闭显示
    OLED_WR_Byte(0x00, OLED_CMD);   // 低列地址
    OLED_WR_Byte(0x10, OLED_CMD);   // 高列地址
    OLED_WR_Byte(0x40, OLED_CMD);   // 起始行地址
    OLED_WR_Byte(0x81, OLED_CMD);   // 对比度设置
    OLED_WR_Byte(0xCF, OLED_CMD);   // 对比度值
    OLED_WR_Byte(0xA1, OLED_CMD);   // SEG 映射（正常）
    OLED_WR_Byte(0xC8, OLED_CMD);   // COM 扫描方向（正常）
    OLED_WR_Byte(0xA6, OLED_CMD);   // 正常显示（非反色）
    OLED_WR_Byte(0xA8, OLED_CMD);   // 复用率设置
    OLED_WR_Byte(0x3F, OLED_CMD);   // 1/64 驱动
    OLED_WR_Byte(0xD3, OLED_CMD);   // 显示偏移
    OLED_WR_Byte(0x00, OLED_CMD);   // 无偏移
    OLED_WR_Byte(0xD5, OLED_CMD);   // 时钟分频
    OLED_WR_Byte(0x80, OLED_CMD);   // 建议值
    OLED_WR_Byte(0xD9, OLED_CMD);   // 预充电周期
    OLED_WR_Byte(0xF1, OLED_CMD);   // 预充电值
    OLED_WR_Byte(0xDA, OLED_CMD);   // COM 引脚配置
    OLED_WR_Byte(0x12, OLED_CMD);   // 默认值
    OLED_WR_Byte(0xDB, OLED_CMD);   // VCOMH 电压
    OLED_WR_Byte(0x30, OLED_CMD);   // 电压值
    OLED_WR_Byte(0x20, OLED_CMD);   // 寻址模式
    OLED_WR_Byte(0x02, OLED_CMD);   // 页寻址模式
    OLED_WR_Byte(0x8D, OLED_CMD);   // 电荷泵
    OLED_WR_Byte(0x14, OLED_CMD);   // 开启电荷泵
    OLED_Clear();                   // 清屏
    OLED_WR_Byte(0xAF, OLED_CMD);   // 开启显示
}

static void OLED_DrawStr(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode)
{
    while (*str) {
        OLED_ShowChar(x, y, *str, size, mode);
        if (size == 8) x += 6;
        else x += size / 2;
        str++;
    }
}

void OLED_Show(SensorData_t *sensorData)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0;

    OLED_DrawStr(0, 0, "AdcDist:", 8, 1);
    OLED_ShowNum(60, 0, sensorData->avg_adc_distance, 4, 8, 1);
    OLED_DrawStr(0, 8, "Dist:", 8, 1);
    OLED_ShowfloatNum(60, 8, sensorData->distance, 8, 1);

    OLED_DrawStr(0, 16, "AdcTemp:", 8, 1);
    OLED_ShowNum(60, 16, sensorData->avg_adc_temperature, 4, 8, 1);
    OLED_DrawStr(0, 24, "Temp:", 8, 1);
    OLED_ShowfloatNum(60, 24, sensorData->temperature, 8, 1);

    OLED_DrawStr(0, 32, "AdcIllu:", 8, 1);
    OLED_ShowNum(60, 32, sensorData->avg_adc_illuminance, 4, 8, 1);
    OLED_DrawStr(0, 40, "Illu:", 8, 1);
    OLED_ShowfloatNum(60, 40, sensorData->illuminance, 8, 1);

    OLED_Refresh();
}


