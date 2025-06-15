#include "myoled.h"
#include "OLED_Font.h"
#include "stdio.h"

#define OLED_SCL_Write(state) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, ((state) ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define OLED_SDA_Write(state) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, ((state) ? GPIO_PIN_SET : GPIO_PIN_RESET))

void OLED_I2C_Init(void)  //初始化引脚
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pin = GPIO_PIN_12 | GPIO_PIN_13;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_SCL_Write(1);
	OLED_SDA_Write(1);
}

uint8_t oled_sda_read(void)  //读取sda状态
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
}

void oled_start(void)  //i2c起始(sda早到，先拉低sda，然后拉低scl)
{
	OLED_SDA_Write(1);  //先确保sda拉高(在上一次通信可能sda被拉低)
	OLED_SCL_Write(1);  //拉高SCL
	OLED_SDA_Write(0);
	OLED_SCL_Write(0);
}

void oled_stop(void)  //i2c终止(sda迟退，先拉高scl，再拉高sda)
{
	OLED_SDA_Write(0);  //确保终止信号的sda是从低到高
	OLED_SCL_Write(1);
	OLED_SDA_Write(1);
}

void oled_send_byte(uint8_t byte)  //用来发送一个字节
{
	uint8_t i;
	for(i = 0; i < 8; i++)
	{
		OLED_SDA_Write((byte & (0x80 >> i)) ? 1 : 0);
		OLED_SCL_Write(1);
		OLED_SCL_Write(0);
	}
	OLED_SCL_Write(1);  //额外时钟，不处理应答信号
	OLED_SCL_Write(0);
}

//uint8_t oled_re_byte(void)
//{
//	uint8_t i;
//	uint8_t byte = 0x00;
//	OLED_SDA_Write(1);
//	for(i = 0; i < 8; i++)
//	{
//		OLED_SCL_Write(1);
//		if(oled_sda_read())
//		{
//			byte |= (0x80 >> i);
//		}
//		OLED_SCL_Write(0);
//	}
//	return byte;
//}

//uint8_t oled_ack(void)0
//{
//	uint8_t reack;
//	OLED_SDA_Write(1);
//	OLED_SCL_Write(1);
//	reack = oled_sda_read();
//	OLED_SCL_Write(0);
//	return reack;
//}

//void oled_send_ack(uint8_t ack)
//{
//	OLED_SDA_Write(ack);
//	OLED_SCL_Write(1);
//	OLED_SCL_Write(0);
//}

void OLED_WriteCommand(uint8_t Command)  //指定地址写
{
	oled_start();
	oled_send_byte(0x78);  //从机地址
//	oled_ack();
	oled_send_byte(0x00);  //用来写命令
//	oled_ack();
	oled_send_byte(Command);
//	oled_ack();
	oled_stop();
}

void OLED_WriteData(uint8_t Data)
{
	oled_start();
	oled_send_byte(0x78);
//	oled_ack();
	oled_send_byte(0x40);  //写数据
//	oled_ack();
	oled_send_byte(Data);
//	oled_ack();
	oled_stop();
}

void OLED_SetCursor(uint8_t Y, uint8_t X)  //用来确认页数和列
{
	OLED_WriteCommand(0xB0 | Y); //确认页数
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));  //设置列的高四位
	OLED_WriteCommand(0x00 | (X & 0x0F));  //低四位
}

void OLED_Clear(void)
{
	uint8_t i,j;
	for(j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);  //循环8页
		
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);  //每页的每一列写0x00确保清空
		}
	}
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)  //显示字符(行：1~4，列：1~16)
{
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);  //用来确认写的地方
	for(i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);  //循环8次写上半页的字符(' '在字模库中第一个是空格，ASCII相减就能得到字模库里对应的字符位置)
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
	for(i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);  //写下半页字符(16字模高)
	}
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)  //显示字符串
{
	uint8_t i;
	for(i = 0; String[i] != '\0'; i++)  //确保不是最后一个字符
	{
		OLED_ShowChar(Line, Column + i, String[i]);  //加i打印
	}
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number)  //用来显示正数
{
	char buff[12];
	sprintf(buff, "%u", Number);  //用sprintf来转换显示
	OLED_ShowString(Line, Column, buff);
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number)  //显示带符号整数
{
	char buff[12];
	sprintf(buff, "%d", Number);  //sprintf可以处理正负号
	OLED_ShowString(Line, Column, buff);
}

void OLED_ShowChinese_16x16(uint8_t Line, uint8_t Column, const uint8_t *char_font_ptr)
{
	uint8_t i;
	// 16x16中文字符显示
	// Line: 1~4, Column: 1~8 (每个中文字符占用2个8像素宽的位置)
	uint8_t start_col = (Column - 1) * 8;
	
	// 显示上半部分（第一页）
	OLED_SetCursor((Line - 1) * 2, start_col);
	for(i = 0; i < 16; i++)
	{
		OLED_WriteData(char_font_ptr[i]);
	}
	
	// 显示下半部分（第二页）
	OLED_SetCursor((Line - 1) * 2 + 1, start_col);
	for(i = 0; i < 16; i++)
	{
		OLED_WriteData(char_font_ptr[i + 16]);
	}
}

void OLED_ShowChineseString_16x16(uint8_t Line, uint8_t Column, const uint8_t *chinese_str_indices, uint8_t length)
{
	uint8_t i;
	for(i = 0; i < length; i++)
	{
		// 检查边界：每个中文字符占用2个字符位置（16像素），所以最多显示4个中文字符
		if(Column + i * 2 > 15)	break;  // 最大列位置是16，减去当前字符的2个位置
		OLED_ShowHzk_16x16(Line, Column + i * 2, chinese_str_indices[i]);
	}
}

void OLED_Init(void)  //初始化
{
	HAL_Delay(100);
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}

// 显示Hzk字库中的中文字符
void OLED_ShowHzk_16x16(uint8_t Line, uint8_t Column, uint8_t index)
{
	uint8_t i;
	// 16x16中文字符显示，使用Hzk字库
	uint8_t start_col = (Column - 1) * 8;
	
	// 显示上半部分（第一页）
	OLED_SetCursor((Line - 1) * 2, start_col);
	for(i = 0; i < 16; i++)
	{
		OLED_WriteData(Hzk[index][i]);
	}
	
	// 显示下半部分（第二页）
	OLED_SetCursor((Line - 1) * 2 + 1, start_col);
	for(i = 0; i < 16; i++)
	{
		OLED_WriteData(Hzk[index][i + 16]);
	}
}





















