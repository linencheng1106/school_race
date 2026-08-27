#ifndef TIMEOUT_PROTECT_H
#define TIMEOUT_PROTECT_H

#include "state_func.h"

/**
 * 进入一个新状态，并启动该状态的超时计时
 */
void EnterState(MODE new_mode);

/**
 * 周期检查当前动作是否超时
 */
void CheckTimeout(void);

/**
 * 获取当前动作已经执行的时间
 */
uint32_t GetElapsedMs(void);

#endif