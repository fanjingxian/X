#include "head.h"

#define LED_ON LED_Show(8, 1)
#define LED_OFF LED_Show(8, 0)
#define Buff_Size 100
unsigned char UartTxBuf[Buff_Size];
uint8_t rx_buff[Buff_Size];
uint32_t Key_PressTime[4] = {0};   // 各按键按下时间
bool Key_LongPressed[4] = {false}; // 长按触发标志
uint32_t Ferq;
uint16_t MyRTC_Time[] = {23, 1, 1, 23, 59, 55};
uint8_t Disp_mode = 0; // 显示模式
/*计时任务*/
void alarm_clock()
{
    static uint32_t timer_5000ms;
    if (HAL_GetTick() - timer_5000ms >= 5000)
    {
        timer_5000ms = HAL_GetTick();
    }
}

/*输入捕获任务*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t cnt;
    if (htim == &htim3)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) // 直接输入捕获
        {
            cnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
            __HAL_TIM_SetCounter(htim, 0);
            Ferq = 1000000 / cnt;
            HAL_TIM_IC_Start(htim, TIM_CHANNEL_2);
        }
    }
}

/*获取ADC*/
double getADC(void)
{
    double adc;
    HAL_ADC_Start(&hadc2);
    adc = HAL_ADC_GetValue(&hadc2);
    return adc * 3.3 / 4096;
}
/*LCD打印重定向*/
void LCD_Disp(uint8_t Line, char *format, ...)
{
    char String[21];
    va_list args;
    va_start(args, format);
    vsprintf(String, format, args);
    va_end(args);
    LCD_DisplayStringLine(Line, (u8 *)String);
}
/*串口打印重定向*/
void Usart1Printf(const char *format, ...)
{
    uint16_t len;
    va_list args;
    va_start(args, format);
    len = vsnprintf((char *)UartTxBuf, sizeof(UartTxBuf), (char *)format, args);
    va_end(args);
    HAL_UART_Transmit_DMA(&huart1, UartTxBuf, len);
}
/*读取RTC时间*/
void MyRTC_ReadTime(void)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    MyRTC_Time[0] = sDate.Year;
    MyRTC_Time[1] = sDate.Month;
    MyRTC_Time[2] = sDate.Date;
    MyRTC_Time[3] = sTime.Hours;
    MyRTC_Time[4] = sTime.Minutes;
    MyRTC_Time[5] = sTime.Seconds;
    //   HAL_RTC_SetDate();
    //   HAL_RTC_SetTime();
}

/*按键扫描*/
uint8_t Key_scan()
{
    static uint32_t last_time = 0;
    if (HAL_GetTick() - last_time > 50)
    { // 消抖时间
        if (Key1 == 0)
            return 1;
        else if (Key2 == 0)
            return 2;
        else if (Key3 == 0)
            return 3;
        else if (Key4 == 0)
            return 4;
        last_time = HAL_GetTick();
    }
    return 0;
}

/*按键处理*/
void Key_Proc(void)
{
    alarm_clock();

    uint8_t Key_Val = Key_scan();
    static uint8_t Key_Old = 0;

    uint8_t Key_Down = Key_Val & (Key_Old ^ Key_Val); // 下降沿检测（按下事件）
    uint8_t Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 上升沿检测（释放事件）
    Key_Old = Key_Val;

    // 处理按键按下事件，记录时间
    switch (Key_Down)
    {
    case 1:
        Key_PressTime[0] = HAL_GetTick();
        Key_LongPressed[0] = false;
        break;
    case 2:
        Key_PressTime[1] = HAL_GetTick();
        Key_LongPressed[1] = false;
        break;
    case 3:
        Key_PressTime[2] = HAL_GetTick();
        Key_LongPressed[2] = false;
        break;
    case 4:
        Key_PressTime[3] = HAL_GetTick();
        Key_LongPressed[3] = false;
        break;
    default:
        break;
    }

    // 检查长按条件
    for (int i = 0; i < 4; i++)
    {
        if (Key_Val == (i + 1))
        { // 当前按键被按住
            uint32_t now = HAL_GetTick();
            if (!Key_LongPressed[i] && (now - Key_PressTime[i] >= 1500))
            {
                Key_LongPressed[i] = true; // 标记长按已处理
                // 执行长按操作
                switch (i + 1)
                {
                case 1: // 按键1长按
                    // 长按处理代码，如进入设置模式
                    Disp_mode = 0;
                    break;
                case 2: // 按键2长按
                    // ...
                    break;
                case 3: // 按键3长按
                    // ...
                    break;
                case 4: // 按键4长按
                    // ...
                    break;
                }
            }
        }
    }

    // 处理按键释放事件（短按）
    switch (Key_Up)
    {
    case 1:
        if (!Key_LongPressed[0])
        {                  // 未触发长按则为短按
            Disp_mode = 1; // 短按操作，如切换显示模式
        }
        Key_PressTime[0] = 0; // 重置状态
        Key_LongPressed[0] = false;
        break;
    case 2:
        if (!Key_LongPressed[1])
        {
            // 按键2短按处理
            Usart1Printf("123");
        }
        Key_PressTime[1] = 0;
        Key_LongPressed[1] = false;
        break;
    case 3:
        if (!Key_LongPressed[2])
        {
            // 按键3短按处理
        }
        Key_PressTime[2] = 0;
        Key_LongPressed[2] = false;
        break;
    case 4:
        if (!Key_LongPressed[3])
        {
            // 按键4短按处理
        }
        Key_PressTime[3] = 0;
        Key_LongPressed[3] = false;
        break;
    default:
        break;
    }
}

/*LCD显示*/
void LCD_Proc(void)
{
    switch (Disp_mode)
    {
    case 1:
        LCD_Disp(Line1, "       time          ");
        LCD_Disp(Line3, "  %02d:%02d:%02d           ", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
        break;
    case 2:

        break;
    case 3:

        break;
    default:
        break;
    }
    MyRTC_ReadTime();
}

/*LED显示函数*/
void LED_Proc()
{
    
}
