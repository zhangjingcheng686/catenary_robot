#include "TeachControl.h"
#include "cmsis_os2.h"
#include "zigbee.h"
#include "log.h"
#include "uint_float_convert.h"
#include <math.h>

extern osMessageQueueId_t TeachTargetQueueHandle;

_Static_assert(TEACH_TARGET_COUNT == CONTROLLED_MOTOR_COUNT,
               "Teach target count must match the controlled motors");

static TeachTarget_t target;
static uint8_t have_target;

static inline float DegToRad(float deg)
{
    return deg * (3.1415926f / 180.0f);
}

static int IsNormalJointAngleValid(int motor_index, float target_deg)
{
    if (motor_index == LEFT_JOINT_2_INDEX || motor_index == RIGHT_JOINT_3_INDEX) {
        return target_deg >= SPECIAL_JOINT_TARGET_MIN_DEG &&
               target_deg <= SPECIAL_JOINT_TARGET_MAX_DEG;
    }

    return target_deg >= NORMAL_JOINT_TARGET_MIN_DEG &&
           target_deg <= NORMAL_JOINT_TARGET_MAX_DEG;
}

static void FollowMotorAngle(MotorCommand_t *command, int motor_index, float target_deg)
{
    if (!IsNormalJointAngleValid(motor_index, target_deg)) {
        return;
    }
    Motor_TargetAngles[motor_index] = target_deg;
    *command = (MotorCommand_t){.mode = MOTOR_COMMAND_POSITION_SPEED, .enabled = 1};
    command->target.position = DegToRad(target_deg);
    command->target.velocity = DEMO_FOLLOW_VELOCITY;
}

static float MapClawInputToTorque(float input_deg)
{
    if (input_deg <= CLAW_INPUT_MIN_DEG) {
        return CLAW_TORQUE_MIN_NM;
    }
    if (input_deg < CLAW_INPUT_LOW_DEG) {
        return CLAW_TORQUE_MIN_NM +
               (input_deg - CLAW_INPUT_MIN_DEG) *
               (CLAW_TORQUE_LOW_NM - CLAW_TORQUE_MIN_NM) /
               (CLAW_INPUT_LOW_DEG - CLAW_INPUT_MIN_DEG);
    }

    if (input_deg < CLAW_INPUT_DEADBAND_MIN_DEG) {
        return CLAW_TORQUE_LOW_NM +
               (input_deg - CLAW_INPUT_LOW_DEG) *
               (0.0f - CLAW_TORQUE_LOW_NM) /
               (CLAW_INPUT_DEADBAND_MIN_DEG - CLAW_INPUT_LOW_DEG);
    }

    if (input_deg <= CLAW_INPUT_DEADBAND_MAX_DEG) {
        return 0.0f;
    }

    if (input_deg < CLAW_INPUT_HIGH_DEG) {
        return (input_deg - CLAW_INPUT_DEADBAND_MAX_DEG) *
               CLAW_TORQUE_HIGH_NM /
               (CLAW_INPUT_HIGH_DEG - CLAW_INPUT_DEADBAND_MAX_DEG);
    }

    if (input_deg < CLAW_INPUT_MAX_DEG) {
        return CLAW_TORQUE_HIGH_NM +
               (input_deg - CLAW_INPUT_HIGH_DEG) *
               (CLAW_TORQUE_MAX_NM - CLAW_TORQUE_HIGH_NM) /
               (CLAW_INPUT_MAX_DEG - CLAW_INPUT_HIGH_DEG);
    }

    return CLAW_TORQUE_MAX_NM;
}

static void FollowClawTorque(MotorCommand_t *command, float input_deg)
{
    *command = (MotorCommand_t){.mode = MOTOR_COMMAND_MIT, .enabled = 1};
    command->target.torque_ff = MapClawInputToTorque(input_deg);
}

static void StopClawTorque(MotorCommand_t commands[TOTAL_MOTOR_COUNT])
{
    commands[LEFT_CLAW_MOTOR_INDEX] =
        (MotorCommand_t){.mode = MOTOR_COMMAND_MIT, .enabled = 1};
    commands[LEFT_ARM_MOTOR_COUNT + RIGHT_CLAW_MOTOR_INDEX] =
        (MotorCommand_t){.mode = MOTOR_COMMAND_MIT, .enabled = 1};
}

void TeachControl_Reset(void)
{
    target = (TeachTarget_t){0};
    have_target = 0;
}

/* 仅生成本周期目标，不等待、不延时、不直接调用电机发送接口。 */
void TeachControl_Update(const MotorFeedback_t *feedback,
                         MotorCommand_t commands[TOTAL_MOTOR_COUNT])
{
    if (osMessageQueueGet(TeachTargetQueueHandle, &target, NULL, 0) == osOK) {
        have_target = 1;
    }
    if (!have_target || HAL_GetTick() - target.receive_time_ms > ZIGBEE_TARGET_TIMEOUT_MS) {
        StopClawTorque(commands);
        return;
    }
    for (int i = 0; i < TEACH_TARGET_COUNT; i++) {
        if (!isfinite(target.input_deg[i])) {
            StopClawTorque(commands);
            return;
        }
    }

    /* 七路输入映射至九电机数组，跳过左侧拧螺丝电机。 */
    for (int i = 0; i < LEFT_CONTROLLED_MOTOR_COUNT; i++) {
        if (i == LEFT_CLAW_ARRAY_INDEX) {
            FollowClawTorque(&commands[i], target.input_deg[i]);
        } else {
            FollowMotorAngle(&commands[i], i, target.input_deg[i]);
        }
    }
    for (int i = 0; i < RIGHT_CONTROLLED_MOTOR_COUNT; i++) {
        int input_index = LEFT_CONTROLLED_MOTOR_COUNT + i;
        int command_index = LEFT_ARM_MOTOR_COUNT + i;
        if (i == RIGHT_CLAW_ARRAY_INDEX) {
            FollowClawTorque(&commands[command_index], target.input_deg[input_index]);
        } else {
            FollowMotorAngle(&commands[command_index], input_index, target.input_deg[input_index]);
        }
    }

    if (feedback->motor[LEFT_CLAW_MOTOR_INDEX].valid &&
        feedback->motor[LEFT_ARM_MOTOR_COUNT + RIGHT_CLAW_MOTOR_INDEX].valid) {
        char l_torque_str[15] = {0};
        char r_torque_str[15] = {0};
        float2string(feedback->motor[LEFT_CLAW_MOTOR_INDEX].torque, l_torque_str);
        float2string(feedback->motor[LEFT_ARM_MOTOR_COUNT + RIGHT_CLAW_MOTOR_INDEX].torque,
                     r_torque_str);
        LOG_INFO("Claw Torque - L:%s, R:%s", l_torque_str, r_torque_str);
    }
}
