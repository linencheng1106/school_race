#ifndef STATE_FUNC
#define STATE_FUNC

#include "main.h"
#include "stdbool.h"
#include "math.h"
#include "Beep.h"

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


/**
 * 状态机错误类型
 */
typedef enum
{
    TIMEOUT_NONE = 0, // 没有超时
    TIMEOUT_ACTION    // 动作执行超时
} TimeoutError_t;


typedef struct
{
    MODE cur_mode;             // 当前状态
    volatile MODE set_mode;    // CAN 请求状态

    uint32_t enter_time_ms;    // 当前状态进入时间
    uint32_t timeout_ms;       // 当前状态超时时间

    bool fault_active;         // 是否存在活动超时
    TimeoutError_t last_error; // 最近一次超时错误
    MODE last_error_mode;      // 最近一次超时的状态
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
/**
 * 软件复位请求标志
 *
 * 在 CAN 接收中断中置 true，
 * 在 State_Run_Task 中读取并执行复位。
 *
 */
extern volatile bool ResetFlag;

extern bool DoneSignal;
extern STATEMODE state_mode;


//正式进入一个新状态
//负责更新时间、超时时间和 DoneSignal。
void EnterState(MODE new_mode);

//检查当前动作是否执行超时
//由 State_Run_Task 周期调用
void CheckTimeout(void);

//获取当前动作已经运行的时间
//当前状态已运行时间，单位：ms
uint32_t GetElapsedMs(void);

#endif