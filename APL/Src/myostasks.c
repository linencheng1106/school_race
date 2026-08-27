#include "myostasks.h"

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
  for (;;)
  {
    if (state_mode.cur_mode != state_mode.set_mode && DoneSignal == true)
    {
      DoneSignal = false;
      state_mode.cur_mode = state_mode.set_mode;
    }
    state_func(state_mode);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}