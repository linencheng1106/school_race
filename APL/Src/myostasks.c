#include "myostasks.h"
#include "timeout_protect.h"
#include "DJmotor.h"
#include "feedback.h"
uint8_t BeepAlarmTimes = 0;

void LedWaterTask(void *argument)
{
  for (;;)
  {
    Led_Water();
  }
}

void BeepAlarmTask(void *argument)
{
  for (;;)
  {
    uint8_t i;
    for (i = 0; i < BeepAlarmTimes; i++)
    {
      BEEP_ON();
      osDelay(250);
      BEEP_OFF();
      osDelay(250);
      if(i==(BeepAlarmTimes-1))
      BeepAlarmTimes=0;
    }
    osDelay(1);
  }
}

void State_Run_Task(void *argument)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(5);

  bool feedback_sent = true;

  for (;;)
  {
    //--------------------------软件复位处理-----------------------//

    if (ResetFlag == true)
    {

        DJmotor[0].Begin = false;
        DJmotor[0].MODE_Set = DJ_Disable;
        DJmotor[0].valSet.current_raw = 0;
        DJmotor[0].valSet.speed_rpm = 0;

        DJmotor[1].Begin = false;
        DJmotor[1].MODE_Set = DJ_Disable;
        DJmotor[1].valSet.current_raw = 0;
        DJmotor[1].valSet.speed_rpm = 0;

        BEEP_ON();
        vTaskDelay(pdMS_TO_TICKS(100U));

        BEEP_OFF();

        /*
         * 给 GPIO 和电机发送流程少量处理时间。
         */
        vTaskDelay(pdMS_TO_TICKS(20U));
        
        //关闭中断
        __set_FAULTMASK(1);

        /*
         * 执行 Cortex-M 系统软件复位。
         *
         * CPU、外设和程序运行状态会重新初始化，
         * 效果接近重新上电，但不等同于真正断电。
         */
        NVIC_SystemReset();
    }

    /*
     * 只有满足以下两个条件时才切换状态：
     *
     * 1. CAN 请求状态和当前状态不同；
     * 2. 当前动作已经完成或者已经因超时终止。
     */
    if ((state_mode.cur_mode != state_mode.set_mode) &&
            (DoneSignal == true))
    {
    /*
     * 统一调用进入状态函数。
     * 不能再直接修改 cur_mode，
     * 否则不会记录动作进入时间。
     */
      EnterState(state_mode.set_mode);
      feedback_sent = false;
    }

        /*
         * 执行当前动作：
         * 设置电机模式、目标位置并判断是否到位。
         */
        state_func(state_mode);

        /*
         * 动作执行后检查超时。
         *
         * 把它放在 state_func() 后面有一个好处：
         * 如果本周期电机刚好到位，state_func()
         * 会先把 DoneSignal 设置为 true，
         * 随后的超时检查就不会误报。
         */
        CheckTimeout();
        
        /*
          * 只在模式正常完成时反馈。
          *
          * fault_active == true 表示动作因超时而终止，
          * 此时不能发送“动作完成”反馈。
          */
          if ((DoneSignal == true) &&
              (state_mode.fault_active == false) &&
              (feedback_sent == false))
          {
              /*
              * 队列未满时，SendFeedback 返回 true。
              * 发送成功后设置 feedback_sent，
              * 防止每 5 ms 重复发送。
              *
              * 队列已满时保持 false，
              * 下一周期自动重试。
              */  
              if (SendFeedback(state_mode.cur_mode) == true)
              {
                  feedback_sent = true;
              }
          }
        /*
         * 等待到下一个 5 ms 周期。
         */
        vTaskDelayUntil(&xLastWakeTime,xFrequency);
  }
}