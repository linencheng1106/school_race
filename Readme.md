# 注意事项

超时后电机已经失能，因此恢复时应该按照：
超时
→ DISABLED
→ 发送使能命令
→ 进入 IDLE
→ 再发送具体动作
不要在超时后直接发送 GROUND_CATCH 等动作，因为这些状态只设置目标位置，没有重新设置：
DJmotor[x].Begin = true;
DJmotor[x].MODE_Set = DJ_Position;

状态切换条件是：

```
if ((state_mode.cur_mode != state_mode.set_mode) &&
    (DoneSignal == true))
{
    EnterState(state_mode.set_mode);
    feedback_sent = false;
}
```

也就是说必须同时满足：
1. 请求状态和当前状态不同；
2. 当前动作已经结束。

# 超时保护完整逻辑
主控发送动作命令
        ↓
CAN中断修改set_mode
        ↓
State_Run_Task发现cur_mode != set_mode
        ↓
EnterState()
  ├─更新cur_mode
  ├─记录enter_time_ms
  ├─设置timeout_ms
  ├─清除fault_active
  └─DoneSignal=false
        ↓
state_func()
  ├─设置电机目标
  └─判断是否到位
        ↓
CheckTimeout()
  ├─已经完成 → 不处理
  ├─未启用超时 → 不处理
  ├─还没到时间 → 不处理
  └─达到超时
       ├─记录错误
       ├─停止两个电机
       ├─cur_mode=DISABLED
       ├─set_mode=DISABLED
       └─DoneSignal=true