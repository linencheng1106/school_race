#include "timeout_protect.h"
#include "DJmotor.h"
//-----------------------------------------------------------超时保护-------------------------------------------------------------//
static uint32_t GetTimeoutMs(MODE mode)
{
    switch (mode)
    {
    case DISABLED:
        return 0U;

    case IDLE:
        /*
         * 两个电机回到初始位置。
         */
        return 4000U;

    case GROUND_CATCH:
        /*
         * 取大地块需要两个电机同时运动，
         * 暂时给 3 秒。
         */
        return 3000U;

    case GROUND_LIFT:
        return 2500U;

    case GROUND_DROP:
        return 2500U;

    case SKY_CATCH:
        return 3000U;

    case SKY_LIFT:
        return 2500U;

    case BALL_CATCH:
        return 3000U;

    case BALL_LIFT:
        return 2500U;

    case BALL_DROP:
        return 2500U;

    case CLAW_ON:
    case CLAW_OFF:
        return 0U;

    default:
        /*
         * 出现未处理状态时，给一个较短的保护时间。
         */
        return 1000U;
    }
}

static void StopMotors(void)
{
    /* 停止升降电机 */
    DJmotor[0].Begin = false;
    DJmotor[0].MODE_Set = DJ_Disable;
    DJmotor[0].valSet.current_raw = 0;
    DJmotor[0].valSet.speed_rpm = 0;

    /* 停止伸缩电机 */
    DJmotor[1].Begin = false;
    DJmotor[1].MODE_Set = DJ_Disable;
    DJmotor[1].valSet.current_raw = 0;
    DJmotor[1].valSet.speed_rpm = 0;
}

void EnterState(MODE new_mode)
{
    /* 更新当前执行状态 */
    state_mode.cur_mode = new_mode;

    /* 记录状态开始执行的时间 */
    state_mode.enter_time_ms = HAL_GetTick();

    /* 根据状态获得对应超时时间 */
    state_mode.timeout_ms =GetTimeoutMs(new_mode);

    /*
     * 新动作开始时清除活动故障。
     * last_error 和 last_error_mode 不清零，
     * 便于调试器继续观察上一次发生的错误。
     */
    state_mode.fault_active = false;

    /*
     * 新动作尚未完成。
     * 后续由 state_func() 根据位置反馈将其置为 true。
     */
    DoneSignal = false;
}

//获取当前已执行时间
uint32_t GetElapsedMs(void)
{
    return (uint32_t)(HAL_GetTick() - state_mode.enter_time_ms);
}

//检查是否超时主函数
/* 超时条件：
 * 1. 当前动作还没有完成；
 * 2. 当前状态启用了超时；
 * 3. 当前没有处理过超时；
 * 4. 已运行时间大于等于允许时间。
 
 超时后的状态是：
cur_mode       = DISABLED
set_mode       = DISABLED
DoneSignal     = true
fault_active   = true
last_error     = MECHANISM_ERROR_ACTION_TIMEOUT
last_error_mode= 发生超时的动作
 */
void CheckTimeout(void)
{
    /*
     * 动作已经完成，不需要检查超时。
     */
    if (DoneSignal == true)
    {
        return;
    }

    /*
     * timeout_ms == 0 表示该状态不启用超时保护。
     * 例如 DISABLED 状态。
     */
    if (state_mode.timeout_ms == 0U)
    {
        return;
    }

    /*
     * 本次故障已经处理过，防止每 5 ms
     * 重复执行停止动作。
     */
    if (state_mode.fault_active == true)
    {
        return;
    }

    /*
     * 当前动作还没有达到超时时间。
     */
    if (GetElapsedMs() <
        state_mode.timeout_ms)
    {
        return;
    }

    /*************** 执行到这里说明动作已经超时 ***************/

    /*
     * 首先保存故障信息。
     * 必须在把 cur_mode 改成 DISABLED 之前保存，
     * 否则无法知道究竟是哪个动作超时。
     */
    state_mode.last_error = TIMEOUT_ACTION;

    state_mode.last_error_mode =state_mode.cur_mode;

    state_mode.fault_active = true;

    /*
     * 立即停止两个大疆电机，防止：
     * 1. 编码器反馈断线后 PID 持续输出；
     * 2. 机构卡死后电机长时间堵转发热；
     * 3. isDone() 永远为 false 导致动作永久执行。
     */
    StopMotors();

    /*
     * 进入失能状态。
     *
     * 同时修改 cur_mode 和 set_mode，
     * 防止下一轮任务又重新进入刚才超时的状态。
     */
    state_mode.cur_mode = DISABLED;
    state_mode.set_mode = DISABLED;

    /*
     * 标记本次动作流程已经终止。
     * 这样收到新的 CAN 命令后，状态机可以再次切换。
     *
     * 这里的 true 表示“本次流程结束”，
     * 不表示动作成功到达目标位置。
     * 是否成功应结合 last_error 判断。
     */
    DoneSignal = true;
}
