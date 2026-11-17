#include "ScrewControl.h"
#include "DBusSys.h"
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

static void ControlScrewMotor(MotorCommand_t *command,
                              int channel,
                              const MotorFeedbackItem_t *feedback,
                              ScrewControlState_t *state)
{
    int direction = GetScrewDirection(channel);

    *command = (MotorCommand_t){.mode = MOTOR_COMMAND_SPEED, .enabled = 1};

    /* 摇杆回中：停转并允许下一次拧紧。 */
    if (direction == 0) {
        command->target.velocity = 0.0f;
        state->torque_reached = 0;
        state->over_torque_count = 0;
        state->last_direction = 0;
        return;
    }

    /* 达到力矩阈值后保持零速度，摇杆回中后才能重新启动。 */
    if (state->torque_reached) {
        command->target.velocity = 0.0f;
        return;
    }

    if (!feedback->valid || !isfinite(feedback->torque)) {
        command->target.velocity = 0.0f;
        state->over_torque_count = 0;
        return;
    }

    if (state->last_direction != 0 && state->last_direction != direction) {
        state->over_torque_count = 0;
    }
    state->last_direction = (int8_t)direction;

    if (fabsf(feedback->torque) >= SCREW_TORQUE_THRESHOLD_NM) {
        if (state->over_torque_count < TORQUE_CONFIRM_COUNT) {
            state->over_torque_count++;
        }
    } else {
        state->over_torque_count = 0;
    }

    if (state->over_torque_count >= TORQUE_CONFIRM_COUNT) {
        state->torque_reached = 1;
        command->target.velocity = 0.0f;
        return;
    }

    command->target.velocity = direction * SCREW_SPEED_RAD_S;
}

static ScrewControlState_t left_state;
static ScrewControlState_t right_state;

void ScrewControl_Update(const MotorFeedback_t *feedback,
                         MotorCommand_t commands[TOTAL_MOTOR_COUNT])
{
    MotorCommand_t *left_command = &commands[LEFT_SCREW_MOTOR_INDEX];
    MotorCommand_t *right_command = &commands[LEFT_ARM_MOTOR_COUNT + RIGHT_SCREW_MOTOR_INDEX];
    if (remoter.online) {
        ControlScrewMotor(left_command, remoter.rc.ch[0],
                         &feedback->motor[LEFT_SCREW_MOTOR_INDEX], &left_state);
        ControlScrewMotor(right_command, remoter.rc.ch[1],
                         &feedback->motor[LEFT_ARM_MOTOR_COUNT + RIGHT_SCREW_MOTOR_INDEX],
                         &right_state);
    } else {
        *left_command = (MotorCommand_t){.mode = MOTOR_COMMAND_SPEED, .enabled = 1};
        *right_command = (MotorCommand_t){.mode = MOTOR_COMMAND_SPEED, .enabled = 1};
        left_state = (ScrewControlState_t){0};
        right_state = (ScrewControlState_t){0};
    }
}
