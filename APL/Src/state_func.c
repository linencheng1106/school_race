#include "state_func.h"
#include "DJmotor.h"
#include "Solenoid.h"
STATEMODE state_mode =
    {
        .cur_mode = DISABLED,
        .set_mode = DISABLED,
};
float L2D(float lenth) // 将运动的距离转化成要转角度
{
    return (lenth);
}

void claw_on() // 打开夹爪
{
    solenoid_on(3, 0x03);//第3组的阀1和2
}

void claw_off() // 闭合夹爪
{
    solenoid_on(3, 0x00);
}
bool DoneSignal = false;                  // 判断是否完成指定模式的标志
bool isDone(float feedback, float target) // 判断电机是否运动到指定位置附近（防抖）
{
    if (fabsf(feedback - target) <= 0.05)
        return true;
    return false;
}

void state_func(STATEMODE statemode) // 负责设定PID的目标值
{
    switch (statemode.cur_mode)
    {

    case DISABLED:
    {
        DJmotor[0].Begin = 0;
        DJmotor[1].Begin = 0;
        DoneSignal = true;
    }
    break;
    case IDLE:
    {
        DJmotor[0].Begin = 1;
        DJmotor[0].MODE_Set = DJ_Position;
        DJmotor[1].Begin = 1;
        DJmotor[1].MODE_Set = DJ_Position;
        DJmotor[0].valSet.angle_deg = 0;

        DJmotor[1].valSet.angle_deg = 0;
        if (isDone(DJmotor[0].valNow.angle_deg, 0) && isDone(DJmotor[1].valNow.angle_deg, 0))
            DoneSignal = true;
    }
    break;

    case GROUND_CATCH:
    {
        DJmotor[0].valSet.angle_deg = L2D(GROUND_CATCH_HEIGHT);

        DJmotor[1].valSet.angle_deg = L2D(GROUND_CATCH_LENTH);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(GROUND_CATCH_HEIGHT)) && isDone(DJmotor[1].valNow.angle_deg, L2D(GROUND_CATCH_LENTH)))
            DoneSignal = true;
    }
    break;

    case GROUND_LIFT:
    {
        DJmotor[0].valSet.angle_deg = L2D(GROUND_LIFT_HEIGHT);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(GROUND_LIFT_HEIGHT)))
            DoneSignal = true;
    }
    break;

    case GROUND_DROP:
    {
        DJmotor[0].valSet.angle_deg = L2D(GROUND_DROP_HEIGHT);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(GROUND_DROP_HEIGHT)))
            DoneSignal = true;
    }
    break;

    case SKY_CATCH:
    {
        DJmotor[0].valSet.angle_deg = L2D(SKY_CATCH_HEIGHT);

        DJmotor[1].valSet.angle_deg = L2D(SKY_CATCH_LENTH);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(SKY_CATCH_HEIGHT)) && isDone(DJmotor[1].valNow.angle_deg, L2D(SKY_CATCH_LENTH)))
            DoneSignal = true;
    }
    break;

    case SKY_LIFT:
    {
        DJmotor[0].valSet.angle_deg = L2D(SKY_LIFT_HEIGHT);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(SKY_LIFT_HEIGHT)))
            DoneSignal = true;
    }
    break;

    case BALL_CATCH:
    {
        DJmotor[0].valSet.angle_deg = L2D(BALL_CATCH_HEIGHT);

        DJmotor[1].valSet.angle_deg = L2D(BALL_CATCH_LENTH);

        if (isDone(DJmotor[0].valNow.angle_deg, L2D(BALL_CATCH_HEIGHT)) && isDone(DJmotor[1].valNow.angle_deg, L2D(BALL_CATCH_LENTH)))
            DoneSignal = true;
    }
    break;

    case BALL_LIFT:
    {
        DJmotor[0].valSet.angle_deg = L2D(BALL_LIFT_HEIGHT);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(BALL_LIFT_HEIGHT)))
            DoneSignal = true;
    }
    break;

    case BALL_DROP:
    {
        DJmotor[0].valSet.angle_deg = L2D(GROUND_DROP_HEIGHT);
        if (isDone(DJmotor[0].valNow.angle_deg, L2D(GROUND_DROP_HEIGHT)))
            DoneSignal = true;
    }
    break;

    case CLAW_ON:
    {
        claw_on();
        DoneSignal = true;
    }
    break;

    case CLAW_OFF:
    {
        claw_off();
        DoneSignal = true;
    }
    break;
    }
}

void state_receive(CAN_RxHeaderTypeDef Rxheader, uint8_t *Rx_data)
{
    if ((Rxheader.IDE != CAN_ID_EXT) ||
    (Rxheader.RTR != CAN_RTR_DATA) ||
    (Rxheader.ExtId < 0x01010401U) ||
    (Rxheader.ExtId > 0x01010408U) ||
    (Rxheader.DLC != 2U))
    {
        return;
    }

    uint8_t msg = (uint8_t)(Rxheader.ExtId - 0x01010400U); /* 1..8 */
    switch (msg)
    {
    case 1:
    {
        if (Rx_data[0] == 'M' && Rx_data[1] == 1)
            state_mode.set_mode = IDLE;
        if (Rx_data[0] == 'M' && Rx_data[1] == 0)
            state_mode.set_mode = DISABLED;
    }
    break;
    case 2:
    {
        if (Rx_data[0] == 'R' && Rx_data[1] == 1)
            state_mode.set_mode = CLAW_ON;
        if (Rx_data[0] == 'R' && Rx_data[1] == 0)
            state_mode.set_mode = CLAW_OFF;
    }
    break;
    case 3:
    {
        if (Rx_data[0] == 'G' && Rx_data[1] == 'E')
            state_mode.set_mode = GROUND_CATCH;
    }
    break;
    case 4:
    {
        if (Rx_data[0] == 'P' && Rx_data[1] == 'E')
            state_mode.set_mode = GROUND_DROP;
    }
    break;
    case 5:
    {
        if (Rx_data[0] == 'G' && Rx_data[1] == 'S')
            state_mode.set_mode = SKY_CATCH;
    }
    break;
    case 6:
    {
        if (Rx_data[0] == 'T' && Rx_data[1] == 'S')
            state_mode.set_mode = SKY_LIFT;
    }
    break;
    case 7:
    {
        if (Rx_data[0] == 'G' && Rx_data[1] == 'B')
            state_mode.set_mode = BALL_CATCH;
    }
    break;
    case 8:
    {
        if (Rx_data[0] == 'P' && Rx_data[1] == 'B')
            state_mode.set_mode = BALL_DROP;
    }
    break;
    }
}
