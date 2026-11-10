#include "Switch_Task.h"
#include "motor_DM.h"
#include "cmsis_os2.h"
#include "DBusSys.h"
#include "ArmControl_Task.h"
#include <math.h>

#define SCREW_SPEED_RAD_S           2.0f
#define SCREW_TORQUE_THRESHOLD_NM   2.0f
#define TORQUE_CONFIRM_COUNT        5

#define RC_CHANNEL_MID              992
#define RC_CHANNEL_DEADBAND         20

typedef struct
{
    uint8_t torque_reached;
    uint8_t over_torque_count;
    int8_t last_direction;
} ScrewControlState_t;

static int GetScrewDirection(int channel)
{
    if (channel > RC_CHANNEL_MID + RC_CHANNEL_DEADBAND) {
        return 1;
    }
    if (channel < RC_CHANNEL_MID - RC_CHANNEL_DEADBAND) {
        return -1;
    }
    return 0;
}

static void ControlScrewMotor(DM_motor_t *motor,
                              int channel,
                              ScrewControlState_t *state)
{
    int direction = GetScrewDirection(channel);

    if (motor == NULL) {
        return;
    }

    /* 摇杆回中：停转并允许下一次拧紧。 */
    if (direction == 0) {
        Speed_CtrlMotorDM(motor, 0.0f);
        state->torque_reached = 0;
        state->over_torque_count = 0;
        state->last_direction = 0;
        return;
    }

    /* 达到力矩阈值后保持零速度，摇杆回中后才能重新启动。 */
    if (state->torque_reached) {
        Speed_CtrlMotorDM(motor, 0.0f);
        return;
    }

    if (state->last_direction != 0 && state->last_direction != direction) {
        state->over_torque_count = 0;
    }
    state->last_direction = (int8_t)direction;

    if (fabsf(motor->motor_msg.torque_current) >= SCREW_TORQUE_THRESHOLD_NM) {
        if (state->over_torque_count < TORQUE_CONFIRM_COUNT) {
            state->over_torque_count++;
        }
    } else {
        state->over_torque_count = 0;
    }

    if (state->over_torque_count >= TORQUE_CONFIRM_COUNT) {
        state->torque_reached = 1;
        Speed_CtrlMotorDM(motor, 0.0f);
        return;
    }

    Speed_CtrlMotorDM(motor, direction * SCREW_SPEED_RAD_S);
}

void ClawControl_Task(void const *argument)
{
    UNUSED(argument);
    osDelay(1600);

    DM_motor_t *left_new_motor =
        Switch_Motors.left_arm_motors[LEFT_SCREW_MOTOR_INDEX];
    DM_motor_t *right_new_motor =
        Switch_Motors.right_arm_motors[RIGHT_SCREW_MOTOR_INDEX];

    ScrewControlState_t left_state = {0};
    ScrewControlState_t right_state = {0};

    for (;;)
    {
        if (remoter.online) {
            ControlScrewMotor(left_new_motor, remoter.rc.ch[0], &left_state);
            ControlScrewMotor(right_new_motor, remoter.rc.ch[1], &right_state);
        } else {
            if (left_new_motor != NULL) {
                Speed_CtrlMotorDM(left_new_motor, 0.0f);
            }
            if (right_new_motor != NULL) {
                Speed_CtrlMotorDM(right_new_motor, 0.0f);
            }
            left_state = (ScrewControlState_t){0};
            right_state = (ScrewControlState_t){0};
        }

        osDelay(20);
    }
}
