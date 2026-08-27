#ifndef STATE_FUNC
#define STATE_FUNC

#include "main.h"
#include "stdbool.h"
#include "math.h"
#include "Beep.h"
#include "myostasks.h"
typedef enum
{
    DISABLED = 0, // 失能
    IDLE,         // 空闲状态，使两个电机回归原位,使能
    GROUND_CATCH, // 将机构先抬升到大地块指定高度后，再前伸一段距离，微调靠底盘
    GROUND_LIFT,  // 夹紧后机构向上抬升大地块指定距离
    GROUND_DROP,  // 机构向下放置，使大地块触地
    SKY_CATCH,    // 将机构先抬升到天空块指定高度后，再前伸一段距离，微调靠底盘
    SKY_LIFT,     // 夹紧后机构向上抬升天空块指定距离,使其能被吸盘吸住
    BALL_CATCH,   // 将机构先抬升到球指定高度后，再前伸一段距离，微调靠底盘
    BALL_LIFT,    // 夹紧后机构向上抬升球指定距离
    BALL_DROP,    // 机构向下放置，使球触地
    CLAW_ON,      // 夹紧夹爪
    CLAW_OFF,     // 松开夹爪
} MODE;

typedef struct
{
    MODE cur_mode;
    MODE set_mode;
} STATEMODE;

#define GROUND_CATCH_HEIGHT 50.0 // 夹大地块抬升高度
#define GROUND_CATCH_LENTH 1.0  // 夹大地块前伸长度
#define GROUND_LIFT_HEIGHT 60.0  // 夹紧后大地块抬升高度
#define GROUND_DROP_HEIGHT 20.0  // 大地块下放高度
#define SKY_CATCH_HEIGHT 40.0    // 夹天空块抬升高度
#define SKY_CATCH_LENTH 1.0     // 夹天空块前伸长度
#define SKY_LIFT_HEIGHT 80.0     // 夹紧后天空块抬升使与吸盘能配合的高度
#define BALL_CATCH_HEIGHT 50.0   // 夹球抬升高度
#define BALL_CATCH_LENTH 1.0    // 夹球前伸长度
#define BALL_LIFT_HEIGHT 70.0    // 夹紧后球抬升高度

void state_func(STATEMODE state_mode);
void state_receive(CAN_RxHeaderTypeDef Rxheader, uint8_t *Rx_data);
extern bool DoneSignal;
extern STATEMODE state_mode;
#endif