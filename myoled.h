#ifndef my_oled_H
#define my_oled_H

#include "main.h"

void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number);
void OLED_ShowChinese_16x16(uint8_t Line, uint8_t Column, const uint8_t *char_font_ptr);
void OLED_ShowChineseString_16x16(uint8_t Line, uint8_t Column, const uint8_t *chinese_str_indices, uint8_t length);
void OLED_ShowHzk_16x16(uint8_t Line, uint8_t Column, uint8_t index);
void OLED_TestChinese(void);
void OLED_Init(void);




#endif
