#include "head.h"

#define LED_ON LED_Show(8, 1)
#define LED_OFF LED_Show(8, 0)
#define Buff_Size 100
unsigned char UartTxBuf[Buff_Size];
uint8_t rx_buff[Buff_Size];
uint32_t Key_PressTime[4] = {0};   // 各按键按下时间
bool Key_LongPressed[4] = {false}; // 长按触发标志
uint16_t MyRTC_Time[] = {23, 1, 1, 23, 59, 55};//年-月-日-时-分-秒
uint8_t MyRTC_SetTime[3] = {0}; // 设置时间
uint8_t MyRTC_SetDate[3] = {0}; // 设置日期
uint32_t Ferq;
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
    if (htim->Instance == TIM3)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) // 直接输入捕获
        {
            cnt = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + 1;
            Ferq = 1000000 / cnt;
            __HAL_TIM_SetCounter(&htim3, 0);
            //HAL_TIM_IC_Start(htim, TIM_CHANNEL_1);
            
            HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
        }
    }
}

// 设置频率
void Set_PWM_Frequency(uint32_t frequency,float duty_percent) {
    __HAL_TIM_SetAutoreload(&htim2, 1000000 / frequency - 1);

    duty_percent = (duty_percent < 0.0f) ? 0.0f : (duty_percent > 100.0f) ? 100.0f : duty_percent;
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim2);
    uint32_t new_ccr = (uint32_t)((duty_percent / 100.0f) * (arr + 1));
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, new_ccr);
    HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);
}

/*获取ADC*/
double getADC(void)
{
    double adc;
    HAL_ADC_Start(&hadc2);
    adc = HAL_ADC_GetValue(&hadc2);
    return adc * 3.3 / 4096;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // if(huart->Instance == USART1) {
    //     RX_data_Process();
    //     HAL_UART_Receive_DMA(&huart1, Rx_buff, sizeof(Rx_buff));
    // }
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
}
void My_RTC_SetTime(void)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    sTime.Hours = MyRTC_SetTime[0];
    sTime.Minutes = MyRTC_SetTime[1];
    sTime.Seconds = MyRTC_SetTime[2];
    sDate.Year = MyRTC_SetDate[0];
    sDate.Month = MyRTC_SetDate[1];
    sDate.Date = MyRTC_SetDate[2];
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}
/*按键扫描*/
uint8_t Key_scan()
{
    static uint32_t last_time = 0;
    if (HAL_GetTick() - last_time > 50)
    {
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
    uint8_t Key_Down = Key_Val & (Key_Old ^ Key_Val);
    uint8_t Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;

    Key_PressTime[Key_Down-1] = HAL_GetTick();
    Key_LongPressed[Key_Down-1] = false;
    
    for (int i = 0; i < 4; i++)
    {
        if (Key_Val == (i + 1))
        {
            uint32_t now = HAL_GetTick();
            if (!Key_LongPressed[i] && (now - Key_PressTime[i] >= 1500))
            {
                Key_LongPressed[i] = true;
                switch (i + 1)
                {
                case 1: // 按键1长按
                    Disp_mode = 0;
                    LCD_Clear(Black);
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
    switch (Key_Up)
    {
        case 1:
            if (!Key_LongPressed[0])
            {
                Disp_mode = 1;
                Set_PWM_Frequency(1000,50.0f);
            }
            Key_PressTime[0] = 0; // 重置状态
            Key_LongPressed[0] = false;
            break;
        case 2:
            if (!Key_LongPressed[1])
            {
                Set_PWM_Frequency(1000,10.0f);
                Usart1Printf("123");

            }
            Key_PressTime[1] = 0;
            Key_LongPressed[1] = false;
            break;
        case 3:
            if (!Key_LongPressed[2])
            {

            }
            Key_PressTime[2] = 0;
            Key_LongPressed[2] = false;
            break;
        case 4:
            if (!Key_LongPressed[3])
            {

            }
            Key_PressTime[3] = 0;
            Key_LongPressed[3] = false;
            break;
        default:break;
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
        LCD_Disp(Line4, "  %d         ", Ferq);
        break;
    case 2:

        break;
    case 3:

        break;
    default:
        break;
    }
    MyRTC_ReadTime();
		//LCD_Disp(Line3, "  %d         ", Ferq);
}

/*LED显示函数*/
void LED_Proc()
{
    
}
