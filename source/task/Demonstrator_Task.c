#include "cmsis_os2.h"
#include "Demonstrator_Task.h"
#include "Switch_Task.h"
#include "motor_DM.h"
#include "zigbee.h" 
#include "log.h"
#include "uint_float_convert.h"

/**
 * @brief 角度转弧度辅助函数
 */
static inline float DegToRad(float deg)
{
    return deg * (3.1415926f / 180.0f);
}

/**
 * @brief 验证普通关节的目标角度是否合法（加入了独立关节判断）
 */
static int IsNormalJointAngleValid(int motor_index, float target_deg)
{
    if (motor_index == LEFT_JOINT_2_INDEX || motor_index == RIGHT_JOINT_3_INDEX) {
        return target_deg >= SPECIAL_JOINT_TARGET_MIN_DEG &&
               target_deg <= SPECIAL_JOINT_TARGET_MAX_DEG;
    }

    return target_deg >= NORMAL_JOINT_TARGET_MIN_DEG &&
           target_deg <= NORMAL_JOINT_TARGET_MAX_DEG;
}

/**
 * @brief 普通关节电机的跟随控制
 */
static void FollowMotorAngle(DM_motor_t *motor, int motor_index, float target_deg)
{
    if (motor == NULL) {
        return;
    }

    // 空间边界拦截：传入当前正在处理的电机索引，实现差异化限位
    if (!IsNormalJointAngleValid(motor_index, target_deg)) {
        return;
    }

    Motor_TargetAngles[motor_index] = target_deg;
    PosSpeed_CtrlMotorDM(motor, DegToRad(target_deg), DEMO_FOLLOW_VELOCITY);
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

static void FollowClawTorque(DM_motor_t *motor, float input_deg)
{
    if (motor == NULL) {
        return;
    }

    float target_torque = MapClawInputToTorque(input_deg);
    MIT_CtrlMotorDM(motor, 0.0f, 0.0f, 0.0f, 0.0f, target_torque);
}

static void StopClawTorque(void)
{
    DM_motor_t *left_claw = Switch_Motors.left_arm_motors[LEFT_CLAW_ARRAY_INDEX];
    DM_motor_t *right_claw = Switch_Motors.right_arm_motors[RIGHT_CLAW_ARRAY_INDEX];

    if (left_claw != NULL) {
        MIT_CtrlMotorDM(left_claw, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    if (right_claw != NULL) {
        MIT_CtrlMotorDM(right_claw, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

/**
 * @brief 示教器跟随主任务
 */
void Demonstrator_Task(void *argument)
{
    (void)argument;
    osDelay(1500); 

    char l_torque_str[15] = {0};
    char r_torque_str[15] = {0};

    for (;;)
    {
        if (!Remote_Data_Valid) {
            StopClawTorque();
            osDelay(10);
            continue; 
        }

        // --- 处理左臂 ---
        for (int i = 0; i < 3; i++) {
            if (i == LEFT_CLAW_ARRAY_INDEX) {
                FollowClawTorque(Switch_Motors.left_arm_motors[i],
                                 Remote_TargetAngles[i]);
            } else {
                FollowMotorAngle(Switch_Motors.left_arm_motors[i], i, Remote_TargetAngles[i]);
            }
        }

        // --- 处理右臂 ---
        for (int i = 0; i < 4; i++) {
            int motor_index = i + 3;

            if (i == RIGHT_CLAW_ARRAY_INDEX) {
                FollowClawTorque(Switch_Motors.right_arm_motors[i],
                                 Remote_TargetAngles[motor_index]);
            } else {
                FollowMotorAngle(Switch_Motors.right_arm_motors[i],
                                 motor_index,
                                 Remote_TargetAngles[motor_index]);
            }
        }

        // 严密的空指针保护：防止上电初期任务调度抢占时电机指针未初始化完成，导致硬件死机
        if (Switch_Motors.left_arm_motors[LEFT_CLAW_ARRAY_INDEX] != NULL && 
            Switch_Motors.right_arm_motors[RIGHT_CLAW_ARRAY_INDEX] != NULL) 
        {
            float l_torque = Switch_Motors.left_arm_motors[LEFT_CLAW_ARRAY_INDEX]->motor_msg.torque_current;
            float r_torque = Switch_Motors.right_arm_motors[RIGHT_CLAW_ARRAY_INDEX]->motor_msg.torque_current;

            float2string(l_torque, l_torque_str);
            float2string(r_torque, r_torque_str);

            LOG_INFO("Claw Torque - L:%s, R:%s", l_torque_str, r_torque_str);
        }

        osDelay(15); 
    }
}
