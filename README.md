# 0.96oled
# I2C 通信协议基础与 OLED 驱动代码解析

## 一、I2C 通信协议基础

### 1. 起始信号 (Start Condition)

- **步骤**：
    1. 首先，起始SCL时，要先确保SDA是高电平。
    2. 然后设置SCL为高电平。
    3. 接着，SDA要比SCL先动作，所以SDA先拉低。
    4. 最后再拉低SCL。
- *笔记风格总结：SDA高、SCL高 -> SDA低 -> SCL低*

### 2. 终止信号 (Stop Condition)

- **步骤**：
    1. 首先，先确保SDA是拉低的状态。
    2. 确保SDA是从低电平变化到高电平的。
    3. 所以，先拉高SCL。
    4. 然后再拉高SDA。
- *笔记风格总结：SDA低、SCL低 -> SCL高 -> SDA高*

### 3. I2C 写字节

- **步骤**：
    1. 定义一个 `uint8_t` 类型的 `byte` 变量，用来存放要发送的字节。
    2. 用一个 `for` 循环，循环8次，对应字节的8个位。
    3. 在循环中，通过 `(byte & (0x80 >> i))` 来判断当前位是1还是0，并据此设置SDA线的电平（1则SDA高，0则SDA低）。
    4. 然后拉高SCL，让从机读取SDA上的数据。
    5. 再拉低SCL，准备发送下一位或者结束。

### 4. I2C 读字节

- **步骤**：
    1. 定义一个 `uint8_t` 类型的 `byte` 变量，并初始化为0，用来存放读取到的字节。
    2. 首先要释放SDA线（即设置为输入模式或SDA_Write(1)），确保SDA为高电平，由从机控制。
    3. 然后用一个 `for` 循环，循环8次。
    4. 在循环中，先拉高SCL，准备读取SDA上的数据。
    5. 判断SDA引脚当前的电平状态。
    6. 如果SDA为高电平（读到1），就通过 `byte |= (0x80 >> i)` 将 `byte` 对应位设为1。
    7. 然后拉低SCL。
    8. 循环结束后，`byte` 中就存放了从机发送过来的一个字节数据。

### 5. I2C 发送 ACK/NACK 信号

- **步骤**：
    1. 定义一个 `uint8_t` 类型的 `ack` 变量。
    2. 直接给SDA引脚赋值 `ack` 的值 (0代表ACK，1代表NACK)。
    3. 然后拉高SCL。
    4. 再拉低SCL即可。

### 6. I2C 接收 ACK/NACK 信号

- **步骤**：
    1. 定义一个 `uint8_t` 类型的 `reack` 变量。
    2. 先释放SDA线（即设置为输入模式或SDA_Write(1)），确保SDA为高电平，等待从机应答。
    3. 然后拉高SCL。
    4. 读取SDA引脚的值，赋给 `reack` (0代表接收到ACK，1代表接收到NACK或无应答)。
    5. 然后拉低SCL。
    6. 返回 `reack` 的值。

### 7. I2C 指定地址写

- **步骤**：
    1. 发送起始信号。
    2. 发送从机设备地址（包含写操作位）。
    3. 等待并检查ACK。
    4. 发送要写入的内部寄存器地址或命令控制字。
    5. 等待并检查ACK。
    6. 发送要写入的数据。
    7. 等待并检查ACK。
    8. 发送终止信号。

### 8. I2C 指定地址读

- **步骤**：
    1. 发送起始信号。
    2. 发送从机设备地址（包含写操作位）。
    3. 等待并检查ACK。
    4. 发送要读取的内部寄存器地址。
    5. 等待并检查ACK。
    6. 发送重新开始信号（Repeated Start）。
    7. 发送从机设备地址（包含读操作位）。
    8. 等待并检查ACK。
    9. 然后就可以调用I2C读字节的函数来读取数据了，每读一个字节后，主机可以发送ACK（如果还想继续读）或NACK（如果这是最后一个要读的字节）。
    10. 可以用 `for` 循环多次读取多个字节。
    11. 最后发送终止信号。

---

## 二、OLED 驱动代码解析 (基于 I2C)

### 1. 引脚定义宏

- **宏定义**：
    
    ```c
    #define OLED_SCL_Write(state) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, ((state) ? GPIO_PIN_SET : GPIO_PIN_RESET))
    #define OLED_SDA_Write(state) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, ((state) ? GPIO_PIN_SET : GPIO_PIN_RESET))
    
    ```
    
- **解析**：
    - `OLED_SCL_Write(state)`：控制SCL引脚（PB6）的电平，`state` 为1则高电平，为0则低电平。
    - `OLED_SDA_Write(state)`：控制SDA引脚（PB7）的电平，`state` 为1则高电平，为0则低电平。

### 2. OLED I2C 引脚初始化 (`OLED_I2C_Init`)

- **代码**：
    
    ```c
    void OLED_I2C_Init(void)  // 初始化引脚
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStructure.Pull = GPIO_NOPULL;
        GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
        OLED_SCL_Write(1);
        OLED_SDA_Write(1);
    }
    
    ```
    
- **步骤**：
    1. `__HAL_RCC_GPIOB_CLK_ENABLE();`：先使能GPIOB端口的时钟，不然GPIO用不了。
    2. 定义一个 `GPIO_InitTypeDef` 结构体变量 `GPIO_InitStructure`。
    3. `GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;`：设置引脚模式为开漏输出，I2C协议要求。
    4. `GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;`：选择PB6 (SCL) 和 PB7 (SDA)。
    5. `GPIO_InitStructure.Pull = GPIO_NOPULL;`：不使用内部上下拉电阻，需外部上拉。
    6. `GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;`：设置引脚输出速度为高速。
    7. `HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);`：调用HAL库函数初始化引脚。
    8. `OLED_SCL_Write(1);` 和 `OLED_SDA_Write(1);`：将SCL和SDA拉高，进入I2C空闲状态。

### 3. 读取 SDA 状态 (`oled_sda_read`)

- **代码**：
    
    ```c
    uint8_t oled_sda_read(void)  // 读取 SDA 状态
    {
        return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);  // 应为 GPIO_PIN_7
    }
    
    ```
    
- **解析**：
    - 这个函数用来读取SDA引脚（PB7）的电平状态。
    - **注意**：原代码中可能是 `GPIO_PIN_13`，但根据宏定义，应为 `GPIO_PIN_7`，这里已更正。

### 4. I2C 起始信号 (`oled_start`)

- **代码**：
    
    ```c
    void oled_start(void)  // I2C 起始
    {
        OLED_SDA_Write(1);  // 先确保 SDA 拉高
        OLED_SCL_Write(1);  // 拉高 SCL
        OLED_SDA_Write(0);
        OLED_SCL_Write(0);
    }
    
    ```
    
- **步骤**：
    1. `OLED_SDA_Write(1);`：先确保SDA是高电平。
    2. `OLED_SCL_Write(1);`：然后拉高SCL。
    3. `OLED_SDA_Write(0);`：SDA先拉低。
    4. `OLED_SCL_Write(0);`：SCL再拉低，完成起始信号。

### 5. I2C 终止信号 (`oled_stop`)

- **代码**：
    
    ```c
    void oled_stop(void)  // I2C 终止
    {
        OLED_SDA_Write(0);  // 确保终止信号的 SDA 是从低到高
        OLED_SCL_Write(1);
        OLED_SDA_Write(1);
    }
    
    ```
    
- **步骤**：
    1. `OLED_SDA_Write(0);`：先确保SDA是低电平。
    2. `OLED_SCL_Write(1);`：SCL先拉高。
    3. `OLED_SDA_Write(1);`：SDA再拉高，完成终止信号。

### 6. 发送一个字节 (`oled_send_byte`)

- **代码**：
    
    ```c
    void oled_send_byte(uint8_t byte)  // 用来发送一个字节
    {
        uint8_t i;
        for(i = 0; i < 8; i++)
        {
            OLED_SDA_Write((byte & (0x80 >> i)) ? 1 : 0);
            OLED_SCL_Write(1);
            OLED_SCL_Write(0);
        }
        OLED_SCL_Write(1);  // 额外时钟，不处理应答信号
        OLED_SCL_Write(0);
    }
    
    ```
    
- **步骤**：
    1. 用 `for` 循环，`i` 从0到7，发送8位。
    2. `OLED_SDA_Write((byte & (0x80 >> i)) ? 1 : 0);`：判断当前位并设置SDA。
    3. `OLED_SCL_Write(1);`：拉高SCL，从机读取数据。
    4. `OLED_SCL_Write(0);`：拉低SCL，完成一位。
    5. 额外时钟脉冲为从机应答准备，但未处理ACK。

### 7. OLED 写命令 (`OLED_WriteCommand`)

- **代码**：
    
    ```c
    void OLED_WriteCommand(uint8_t Command)  // 指定地址写
    {
        oled_start();
        oled_send_byte(0x78);  // 从机地址
        oled_send_byte(0x00);  // 用来写命令
        oled_send_byte(Command);
        oled_stop();
    }
    
    ```
    
- **步骤**：
    1. `oled_start();`：发送起始信号。
    2. `oled_send_byte(0x78);`：发送从机地址 `0x78`。
    3. `oled_send_byte(0x00);`：发送控制字节，表示命令。
    4. `oled_send_byte(Command);`：发送具体命令。
    5. `oled_stop();`：发送终止信号。

### 8. OLED 写数据 (`OLED_WriteData`)

- **代码**：
    
    ```c
    void OLED_WriteData(uint8_t Data)
    {
        oled_start();
        oled_send_byte(0x78);
        oled_send_byte(0x40);  // 写数据
        oled_send_byte(Data);
        oled_stop();
    }
    
    ```
    
- **步骤**：
    1. `oled_start();`：发送起始信号。
    2. `oled_send_byte(0x78);`：发送从机地址。
    3. `oled_send_byte(0x40);`：发送控制字节，表示数据。
    4. `oled_send_byte(Data);`：发送具体数据。
    5. `oled_stop();`：发送终止信号。

### 9. OLED 设置光标位置 (`OLED_SetCursor`)

- **代码**：
    
    ```c
    void OLED_SetCursor(uint8_t Y, uint8_t X)  // 用来确认页数和列
    {
        OLED_WriteCommand(0xB0 | Y); // 确认页数
        OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));  // 设置列的高四位
        OLED_WriteCommand(0x00 | (X & 0x0F));  // 低四位
    }
    
    ```
    
- **步骤**：
    1. `OLED_WriteCommand(0xB0 | Y);`：设置页地址（0-7）。
    2. `OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));`：设置列地址高4位。
    3. `OLED_WriteCommand(0x00 | (X & 0x0F));`：设置列地址低4位。

### 10. OLED 清屏 (`OLED_Clear`)

- **代码**：
    
    ```c
    void OLED_Clear(void)
    {
        uint8_t i, j;
        for(j = 0; j < 8; j++)
        {
            OLED_SetCursor(j, 0);  // 循环8页
            for(i = 0; i < 128; i++)
            {
                OLED_WriteData(0x00);  // 每页的每一列写0x00确保清空
            }
        }
    }
    
    ```
    
- **步骤**：
    1. 用两层循环遍历8页，每页128列。
    2. `OLED_SetCursor(j, 0);`：设置每页起始位置。
    3. `OLED_WriteData(0x00);`：写入0x00清空显示。

### 11. OLED 显示字符 (`OLED_ShowChar`)

- **代码**：
    
    ```c
    void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)  // 显示字符
    {
        uint8_t i;
        OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);  // 确认写的地方
        for(i = 0; i < 8; i++)
        {
            OLED_WriteData(OLED_F8x16[Char - ' '][i]);  // 写上半页
        }
        OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
        for(i = 0; i < 8; i++)
        {
            OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);  // 写下半页
        }
    }
    
    ```
    
- **步骤**：
    1. 设置光标到上半部分 `(Line - 1) * 2`。
    2. 写入字模上半页8列数据。
    3. 设置光标到下半部分 `+ 1`。
    4. 写入字模下半页8列数据。

### 12. OLED 显示字符串 (`OLED_ShowString`)

- **代码**：
    
    ```c
    void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)  // 显示字符串
    {
        uint8_t i;
        for(i = 0; String[i] != '\\0'; i++)
        {
            OLED_ShowChar(Line, Column + i, String[i]);  // 加i打印
        }
    }
    
    ```
    
- **步骤**：
    1. 循环遍历字符串，直至 `\\0`。
    2. 逐个调用 `OLED_ShowChar` 显示字符。

### 13. OLED 显示正数 (`OLED_ShowNum`)

- **代码**：
    
    ```c
    void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number)  // 显示正数
    {
        char buff[12];
        sprintf(buff, "%u", Number);  // 转换
        OLED_ShowString(Line, Column, buff);
    }
    
    ```
    
- **步骤**：
    1. 用 `sprintf` 将正数转为字符串。
    2. 调用 `OLED_ShowString` 显示。

### 14. OLED 显示带符号整数 (`OLED_ShowSignedNum`)

- **代码**：
    
    ```c
    void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number)  // 显示带符号整数
    {
        char buff[12];
        sprintf(buff, "%d", Number);  // 处理正负号
        OLED_ShowString(Line, Column, buff);
    }
    
    ```
    
- **步骤**：
    1. 用 `sprintf` 将带符号整数转为字符串。
    2. 调用 `OLED_ShowString` 显示。

### 15. OLED 初始化 (`OLED_Init`)

- **代码**：
    
    ```c
    void OLED_Init(void)  // 初始化
    {
        HAL_Delay(100);
        OLED_I2C_Init();  // 端口初始化
        OLED_WriteCommand(0xAE);  // 关闭显示
        OLED_WriteCommand(0xD5);  // 设置时钟分频比/振荡器频率
        OLED_WriteCommand(0x80);
        OLED_WriteCommand(0xA8);  // 设置多路复用率
        OLED_WriteCommand(0x3F);
        OLED_WriteCommand(0xD3);  // 设置显示偏移
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x40);  // 设置显示开始行
        OLED_WriteCommand(0xA1);  // 设置左右方向
        OLED_WriteCommand(0xC8);  // 设置上下方向
        OLED_WriteCommand(0xDA);  // 设置COM引脚硬件配置
        OLED_WriteCommand(0x12);
        OLED_WriteCommand(0x81);  // 设置对比度控制
        OLED_WriteCommand(0xCF);
        OLED_WriteCommand(0xD9);  // 设置预充电周期
        OLED_WriteCommand(0xF1);
        OLED_WriteCommand(0xDB);  // 设置VCOMH取消选择级别
        OLED_WriteCommand(0x30);
        OLED_WriteCommand(0xA4);  // 设置整个显示打开/关闭
        OLED_WriteCommand(0xA6);  // 设置正常/倒转显示
        OLED_WriteCommand(0x8D);  // 设置充电泵
        OLED_WriteCommand(0x14);
        OLED_WriteCommand(0xAF);  // 开启显示
        OLED_Clear();  // 清屏
    }
    
    ```
    
- **步骤**：
    1. `HAL_Delay(100);`：延时确保上电稳定。
    2. `OLED_I2C_Init();`：初始化I2C引脚。
    3. 发送一系列命令配置OLED芯片。
    4. `OLED_Clear();`：清屏。
