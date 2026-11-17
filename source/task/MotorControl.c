#include "MotorControl.h"
#include "MotorFeedback.h"
#include "TeachControl.h"
#include "ScrewControl.h"
#include "RLControl.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <string.h>

_Static_assert(TOTAL_MOTOR_COUNT == ROBOT_MOTOR_COUNT, "Motor count mismatch");

static volatile RobotMode_t requested_mode = ROBOT_MODE_IDLE;
static RobotMode_t active_mode = ROBOT_MODE_IDLE;
static MotorCommand_t motor_commands[TOTAL_MOTOR_COUNT];

extern osMessageQueueId_t TeachTargetQueueHandle;

ArmMotors_Info_t Switch_Motors; 
float Motor_TargetAngles[CONTROLLED_MOTOR_COUNT];

const uint8_t left_arm_ids[LEFT_ARM_MOTOR_COUNT] = {0x21, 0x22, 0x23, 0x24};
const uint8_t right_arm_ids[RIGHT_ARM_MOTOR_COUNT] = {0x11, 0x12, 0x13, 0x14, 0x15};

const can_port_t motor_ports[TOTAL_MOTOR_COUNT] = {
    CAN1_PORT, CAN1_PORT, CAN1_PORT, CAN1_PORT,
    CAN2_PORT, CAN2_PORT, CAN2_PORT, CAN2_PORT, CAN2_PORT
};

const float init_angles[TOTAL_MOTOR_COUNT] = {
    90.0f, -90.0f, 0.0f, 0.0f,
    0.0f, 90.0f, 90.0f, 0.0f, 0.0f
};

static void Motors_InitAll(void)
{
    float velocity = 8.0f;

    for (int i = 0; i < LEFT_CONTROLLED_MOTOR_COUNT; i++) {
        Motor_TargetAngles[i] = init_angles[i];
    }
    for (int i = 0; i < RIGHT_CONTROLLED_MOTOR_COUNT; i++) {
        Motor_TargetAngles[i + LEFT_CONTROLLED_MOTOR_COUNT] =
            init_angles[i + LEFT_ARM_MOTOR_COUNT];
    }

    osDelay(100);  

    for (int i = 0; i < LEFT_ARM_MOTOR_COUNT; i++) {
        uint32_t control_mode = POS_MODE;
        if (i == LEFT_CLAW_MOTOR_INDEX) {
            control_mode = MIT_MODE;
        } else if (i == LEFT_SCREW_MOTOR_INDEX) {
            control_mode = SPD_MODE;
        }
        uint32_t can_id = left_arm_ids[i] | control_mode;
        uint32_t msg_id = 0x01 + i;
        can_port_t port = motor_ports[i];

        Switch_Motors.left_arm_motors[i] = pvPortMalloc(sizeof(DM_motor_t));
        if (Switch_Motors.left_arm_motors[i] == NULL) {
            continue;
        }
        memset(Switch_Motors.left_arm_motors[i], 0, sizeof(DM_motor_t));
        
        if (i == 0) {
            Motor_DM6248_sign(Switch_Motors.left_arm_motors[i], can_id, msg_id, port); 
        } else if (i == 1) {
            Motor_DM4340_sign(Switch_Motors.left_arm_motors[i], can_id, msg_id, port); 
        } else if (i == 2) {
            Motor_DM4340_sign(Switch_Motors.left_arm_motors[i], can_id, msg_id, port);
        } else if (i == 3) {
            Motor_DM4340_sign(Switch_Motors.left_arm_motors[i], can_id, msg_id, port);
        }

        Motor_DM_Enable(Switch_Motors.left_arm_motors[i]);
        osDelay(10);  

        float raw_target_rad = init_angles[i] * (3.1415926f / 180.0f);

        if (i == LEFT_CLAW_MOTOR_INDEX) {
            MIT_CtrlMotorDM(Switch_Motors.left_arm_motors[i],
                            0.0f, 0.0f, KP_GAIN, KD_GAIN, 0.0f);
        } else if (i == LEFT_SCREW_MOTOR_INDEX) {
            Speed_CtrlMotorDM(Switch_Motors.left_arm_motors[i], 0.0f);
        } else {
            PosSpeed_CtrlMotorDM(Switch_Motors.left_arm_motors[i], raw_target_rad, velocity);
        }
        osDelay(10);
    }

    for (int i = 0; i < RIGHT_ARM_MOTOR_COUNT; i++) {
        uint32_t control_mode = POS_MODE;
        if (i == RIGHT_CLAW_MOTOR_INDEX) {
            control_mode = MIT_MODE;
        } else if (i == RIGHT_SCREW_MOTOR_INDEX) {
            control_mode = SPD_MODE;
        }
        uint32_t can_id = right_arm_ids[i] | control_mode;
        uint32_t msg_id = 0x01 + i; 
        can_port_t port = motor_ports[i + LEFT_ARM_MOTOR_COUNT];

        Switch_Motors.right_arm_motors[i] = pvPortMalloc(sizeof(DM_motor_t));
        if (Switch_Motors.right_arm_motors[i] == NULL) {
            continue;
        }
        memset(Switch_Motors.right_arm_motors[i], 0, sizeof(DM_motor_t));

        if (i == 0) {
            Motor_DM6248_sign(Switch_Motors.right_arm_motors[i], can_id, msg_id, port);
        } else if (i == 1) {
            Motor_DM6248_sign(Switch_Motors.right_arm_motors[i], can_id, msg_id, port); 
        } else if (i == 2) {
            Motor_DM4340_sign(Switch_Motors.right_arm_motors[i], can_id, msg_id, port); 
        } else if (i == 3) {
            Motor_DM4340_sign(Switch_Motors.right_arm_motors[i], can_id, msg_id, port);
        } else if (i == 4) {
            Motor_DM4340_sign(Switch_Motors.right_arm_motors[i], can_id, msg_id, port);
        }

        Motor_DM_Enable(Switch_Motors.right_arm_motors[i]);
        osDelay(10); 

        float raw_target_rad = init_angles[i + LEFT_ARM_MOTOR_COUNT] * (3.1415926f / 180.0f);

        if (i == RIGHT_CLAW_MOTOR_INDEX) {
            MIT_CtrlMotorDM(Switch_Motors.right_arm_motors[i],
                            0.0f, 0.0f, KP_GAIN, KD_GAIN, 0.0f);
        } else if (i == RIGHT_SCREW_MOTOR_INDEX) {
            Speed_CtrlMotorDM(Switch_Motors.right_arm_motors[i], 0.0f);
        } else {
            PosSpeed_CtrlMotorDM(Switch_Motors.right_arm_motors[i], raw_target_rad, velocity);
        }
        osDelay(10);
    }
}


/* 按九电机顺序建立初始姿态，电机模式和初始参数沿用原实现。 */
static void BuildInitialTargets(void)
{
    for (int i = 0; i < TOTAL_MOTOR_COUNT; i++) {
        MotorCommand_t *command = &motor_commands[i];
        *command = (MotorCommand_t){.mode = MOTOR_COMMAND_POSITION_SPEED, .enabled = 1};
        command->target.position = init_angles[i] * (3.1415926f / 180.0f);
        command->target.velocity = 8.0f;
        if (i == LEFT_CLAW_MOTOR_INDEX ||
            i == LEFT_ARM_MOTOR_COUNT + RIGHT_CLAW_MOTOR_INDEX) {
            command->mode = MOTOR_COMMAND_MIT;
            command->target = (MotorTarget_t){.kp = KP_GAIN, .kd = KD_GAIN};
        } else if (i == LEFT_SCREW_MOTOR_INDEX ||
                   i == LEFT_ARM_MOTOR_COUNT + RIGHT_SCREW_MOTOR_INDEX) {
            command->mode = MOTOR_COMMAND_SPEED;
            command->target = (MotorTarget_t){0};
        }
    }
    for (int i = 0; i < LEFT_CONTROLLED_MOTOR_COUNT; i++) {
        Motor_TargetAngles[i] = init_angles[i];
    }
    for (int i = 0; i < RIGHT_CONTROLLED_MOTOR_COUNT; i++) {
        Motor_TargetAngles[LEFT_CONTROLLED_MOTOR_COUNT + i] =
            init_angles[LEFT_ARM_MOTOR_COUNT + i];
    }
}

/* 模式切换时先清除工具输出；普通关节用最新有效反馈建立保持目标。 */
static void PrepareModeEntry(const MotorFeedback_t *feedback)
{
    for (int i = 0; i < TOTAL_MOTOR_COUNT; i++) {
        MotorCommand_t *command = &motor_commands[i];
        command->target = (MotorTarget_t){0};
        if (command->mode == MOTOR_COMMAND_POSITION_SPEED) {
            command->enabled = feedback->motor[i].valid &&
                               isfinite(feedback->motor[i].position);
            if (command->enabled) {
                command->target.position = feedback->motor[i].position;
                command->target.velocity = DEMO_FOLLOW_VELOCITY;
            }
        } else {
            command->enabled = 1;
        }
    }
}

/* 进入模式时缺少角度的关节，等有效反馈到达后再建立保持目标。 */
static void PreparePendingJointHolds(const MotorFeedback_t *feedback)
{
    for (int i = 0; i < TOTAL_MOTOR_COUNT; i++) {
        MotorCommand_t *command = &motor_commands[i];
        if (command->mode == MOTOR_COMMAND_POSITION_SPEED && !command->enabled &&
            feedback->motor[i].valid && isfinite(feedback->motor[i].position)) {
            command->target.position = feedback->motor[i].position;
            command->target.velocity = DEMO_FOLLOW_VELOCITY;
            command->enabled = 1;
        }
    }
}

/* 运行期间所有电机控制帧仅从此处发送。初始化也由同一个任务完成。 */
static void SendMotorCommands(void)
{
    for (int i = 0; i < TOTAL_MOTOR_COUNT; i++) {
        DM_motor_t *motor = i < LEFT_ARM_MOTOR_COUNT ?
            Switch_Motors.left_arm_motors[i] :
            Switch_Motors.right_arm_motors[i - LEFT_ARM_MOTOR_COUNT];
        const MotorCommand_t *command = &motor_commands[i];
        const MotorTarget_t *target = &command->target;
        if (motor == NULL || !command->enabled) {
            continue;
        }
        switch (command->mode) {
            case MOTOR_COMMAND_POSITION_SPEED:
                PosSpeed_CtrlMotorDM(motor, target->position, target->velocity);
                break;
            case MOTOR_COMMAND_SPEED:
                Speed_CtrlMotorDM(motor, target->velocity);
                break;
            case MOTOR_COMMAND_MIT:
                MIT_CtrlMotorDM(motor, target->position, target->velocity,
                                target->kp, target->kd, target->torque_ff);
                break;
            default:
                break;
        }
    }
}

void MotorControl_RequestMode(RobotMode_t mode)
{
    if (mode == ROBOT_MODE_IDLE || mode == ROBOT_MODE_TEACH || mode == ROBOT_MODE_RL) {
        requested_mode = mode;
    }
}

static void RunControlCycle(void)
{
    /* 本周期只采纳一次请求，执行中到达的请求留到下一周期。 */
    RobotMode_t cycle_mode = requested_mode;
    MotorFeedback_t feedback = {0};
    (void)MotorFeedback_GetLatest(&feedback);
    for (int i = 0; i < TOTAL_MOTOR_COUNT; i++) {
        if (HAL_GetTick() - feedback.motor[i].last_receive_time_ms >
            MOTOR_FEEDBACK_TIMEOUT_MS) {
            feedback.motor[i].valid = 0;
        }
    }

    if (cycle_mode != active_mode) {
        TeachControl_Reset();
        RLControl_Reset();
        if (cycle_mode == ROBOT_MODE_IDLE) {
            BuildInitialTargets();
        } else {
            PrepareModeEntry(&feedback);
        }
        active_mode = cycle_mode;
    }
    PreparePendingJointHolds(&feedback);

    switch (active_mode) {
        case ROBOT_MODE_TEACH:
            TeachControl_Update(&feedback, motor_commands);
            ScrewControl_Update(&feedback, motor_commands);
            break;
        case ROBOT_MODE_RL:
            RLControl_Update(motor_commands);
            break;
        case ROBOT_MODE_IDLE:
        default:
            ScrewControl_Update(&feedback, motor_commands);
            break;
    }
    SendMotorCommands();
}

void MotorControl_Task(void *argument)
{
    (void)argument;
    configASSERT(TeachTargetQueueHandle != NULL);
    configASSERT(osMessageQueueGetCapacity(TeachTargetQueueHandle) == 1U);
    configASSERT(osMessageQueueGetMsgSize(TeachTargetQueueHandle) == sizeof(TeachTarget_t));
    configASSERT(pdMS_TO_TICKS(MOTOR_CONTROL_PERIOD_MS) > 0);

    osDelay(1000);
    Motors_InitAll();
    BuildInitialTargets();
    osThreadFlagsSet(MotorFeedbackHandle, MOTOR_FEEDBACK_READY_FLAG);

    TickType_t last_wake_time = xTaskGetTickCount();
    for (;;) {
        RunControlCycle();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_CONTROL_PERIOD_MS));
    }
}
