#include "ScrewControl.h"
#include "DBusSys.h"
#include "MotorFeedback.h"
#include <math.h>

#define RC_CHANNEL_MID              992
#define RC_CHANNEL_DEADBAND         20

typedef struct
{
    uint8_t stop_latched;
    uint8_t over_torque_count;
    uint8_t have_feedback;
    uint8_t window_active;
    int8_t last_direction;
    uint32_t motion_start_ms;
    uint32_t last_feedback_ms;
    uint32_t window_start_ms;
    float last_position;
    float window_position;
    float min_position;
    float max_position;
} ScrewControlState_t;

static ScrewControlState_t left_state;
static ScrewControlState_t right_state;

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

static void ResetMotionTracking(ScrewControlState_t *state)
{
    uint8_t stop_latched = state->stop_latched;
    *state = (ScrewControlState_t){.stop_latched = stop_latched};
}

void ScrewControl_ResetMonitoring(void)
{
    ResetMotionTracking(&left_state);
    ResetMotionTracking(&right_state);
}

static void StartPositionWindow(ScrewControlState_t *state,
                                const MotorFeedbackItem_t *feedback)
{
    state->window_active = 1;
    state->window_start_ms = feedback->last_receive_time_ms;
    state->last_position = feedback->position;
    state->window_position = 0.0f;
    state->min_position = 0.0f;
    state->max_position = 0.0f;
}

static void ControlScrewMotor(MotorCommand_t *command,
                              int channel,
                              const MotorFeedbackItem_t *feedback,
                              ScrewControlState_t *state)
{
    int direction = GetScrewDirection(channel);
    uint32_t now = HAL_GetTick();

    /* 纯力矩输出，位置、速度、Kp、Kd 均保持为零。 */
    *command = (MotorCommand_t){.mode = MOTOR_COMMAND_MIT, .enabled = 1};

    /* 摇杆回中后解除停止锁定，允许下一次启动。 */
    if (direction == 0) {
        *state = (ScrewControlState_t){0};
        return;
    }
    if (state->stop_latched) {
        return;
    }

    if (!feedback->valid || !isfinite(feedback->position) ||
        !isfinite(feedback->torque) ||
        now - feedback->last_receive_time_ms > MOTOR_FEEDBACK_TIMEOUT_MS) {
        ResetMotionTracking(state);
        return;
    }

    if (state->last_direction != direction) {
        ResetMotionTracking(state);
        state->last_direction = (int8_t)direction;
        state->motion_start_ms = now;
    }

    command->target.torque_ff = direction * SCREW_WORK_TORQUE_NM;

    /* 一个 CAN 反馈样本只计入一次，重复读取队列不会增加计数。 */
    if (state->have_feedback &&
        feedback->last_receive_time_ms == state->last_feedback_ms) {
        return;
    }
    if (state->have_feedback &&
        feedback->last_receive_time_ms - state->last_feedback_ms > SCREW_FEEDBACK_MAX_GAP_MS) {
        state->motion_start_ms = now;
        state->window_active = 0;
        state->over_torque_count = 0;
    }
    state->have_feedback = 1;
    state->last_feedback_ms = feedback->last_receive_time_ms;

    if (fabsf(feedback->torque) >= SCREW_TORQUE_THRESHOLD_NM) {
        state->over_torque_count++;
    } else {
        state->over_torque_count = 0;
    }
    if (state->over_torque_count >= SCREW_TORQUE_CONFIRM_COUNT) {
        state->stop_latched = 1;
        command->target.torque_ff = 0.0f;
        return;
    }

    if (now - state->motion_start_ms < SCREW_STARTUP_GRACE_MS) {
        return;
    }
    if (!state->window_active) {
        StartPositionWindow(state, feedback);
        return;
    }

    /* 消除 DM4340 角度在 -12.5/+12.5 rad 边界回绕造成的跳变。 */
    float delta = remainderf(feedback->position - state->last_position,
                             SCREW_POSITION_PERIOD_RAD);
    state->last_position = feedback->position;
    state->window_position += delta;
    state->min_position = fminf(state->min_position, state->window_position);
    state->max_position = fmaxf(state->max_position, state->window_position);

    /* 检查整个窗口的角度范围，往复运动也会计入实际转动。 */
    if (feedback->last_receive_time_ms - state->window_start_ms >= SCREW_STALL_WINDOW_MS) {
        if (state->max_position - state->min_position < SCREW_STALL_POSITION_RAD) {
            state->stop_latched = 1;
            command->target.torque_ff = 0.0f;
        } else {
            StartPositionWindow(state, feedback);
        }
    }
}

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
        *left_command = (MotorCommand_t){.mode = MOTOR_COMMAND_MIT, .enabled = 1};
        *right_command = (MotorCommand_t){.mode = MOTOR_COMMAND_MIT, .enabled = 1};
        left_state = (ScrewControlState_t){0};
        right_state = (ScrewControlState_t){0};
    }
}
