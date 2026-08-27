#include "feedback.h"
#include "CanQueue.h"

//反馈的数据内容与主控发送的命令内容保持一致。

bool SendFeedback(MODE mode)
{
    CAN_RxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[2] = {0};
    uint32_t feedback_id = 0U;

    /*
     * 根据已经完成的状态，
     * 选择反馈 ID 和两个数据字节。
     */
    switch (mode)
    {
    case DISABLED:
        feedback_id = 0x04010101U;
        tx_data[0] = 'M';
        tx_data[1] = 0U;
        break;

    case IDLE:
        feedback_id = 0x04010101U;
        tx_data[0] = 'M';
        tx_data[1] = 1U;
        break;

    case CLAW_OFF:
        feedback_id = 0x04010102U;
        tx_data[0] = 'R';
        tx_data[1] = 0U;
        break;

    case CLAW_ON:
        feedback_id = 0x04010102U;
        tx_data[0] = 'R';
        tx_data[1] = 1U;
        break;

    case GROUND_CATCH:
        feedback_id = 0x04010103U;
        tx_data[0] = 'G';
        tx_data[1] = 'E';
        break;

    case GROUND_DROP:
        feedback_id = 0x04010104U;
        tx_data[0] = 'P';
        tx_data[1] = 'E';
        break;

    case SKY_CATCH:
        feedback_id = 0x04010105U;
        tx_data[0] = 'G';
        tx_data[1] = 'S';
        break;

    case SKY_LIFT:
        feedback_id = 0x04010106U;
        tx_data[0] = 'T';
        tx_data[1] = 'S';
        break;

    case BALL_CATCH:
        feedback_id = 0x04010107U;
        tx_data[0] = 'G';
        tx_data[1] = 'B';
        break;

    case BALL_DROP:
        feedback_id = 0x04010108U;
        tx_data[0] = 'P';
        tx_data[1] = 'B';
        break;

    case BALL_LIFT:
        feedback_id = 0x04010109U;
        tx_data[0] = 'H';
        tx_data[1] = 'B';
        break;

    case GROUND_LIFT:
        feedback_id = 0x0401010AU;
        tx_data[0] = 'H';
        tx_data[1] = 'E';
        break;

    default:
        /*
         * 未知状态不发送反馈。
         */
        return false;
    }

    /*
     * 队列已满时暂时不发送。
     *
     * State_Run_Task 下一周期还会再次尝试，
     * 所以这里不能把反馈标记为已经完成。
     */
    if (CAN_Queue_IfFull(&CAN1_Txqueue))
    {
        return false;
    }

    /*
     * 准备扩展帧头：
     * ID 为对应反馈 ID，DLC 固定为 2。
     */
    HeaderPrepare(feedback_id, 2U, &tx_header);

    /*
     * CAN_Enqueue 会复制 tx_data 内容，
     * 因此函数返回后局部数组失效没有问题。
     */
    CAN_Enqueue(&CAN1_Txqueue,
                tx_header,
                tx_data);

    return true;
}