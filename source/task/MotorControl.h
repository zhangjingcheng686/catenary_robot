#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "motor_DM.h"
#include "robot_messages.h"

#define LEFT_ARM_MOTOR_COUNT          4
#define RIGHT_ARM_MOTOR_COUNT         5
#define LEFT_CONTROLLED_MOTOR_COUNT   3
#define RIGHT_CONTROLLED_MOTOR_COUNT  4
#define CONTROLLED_MOTOR_COUNT        (LEFT_CONTROLLED_MOTOR_COUNT + RIGHT_CONTROLLED_MOTOR_COUNT)
#define TOTAL_MOTOR_COUNT             (LEFT_ARM_MOTOR_COUNT + RIGHT_ARM_MOTOR_COUNT)
#define LEFT_CLAW_MOTOR_INDEX         2
#define RIGHT_CLAW_MOTOR_INDEX        3
#define LEFT_SCREW_MOTOR_INDEX        3
#define RIGHT_SCREW_MOTOR_INDEX       4
#define KP_GAIN                       35.0f
#define KD_GAIN                       1.5f
#define MOTOR_CONTROL_PERIOD_MS       20U

typedef struct {
    DM_motor_t *left_arm_motors[LEFT_ARM_MOTOR_COUNT];
    DM_motor_t *right_arm_motors[RIGHT_ARM_MOTOR_COUNT];
} ArmMotors_Info_t;

typedef enum {
    ROBOT_MODE_IDLE = 0,
    ROBOT_MODE_RL,
    ROBOT_MODE_TEACH
} RobotMode_t;

typedef enum {
    MOTOR_COMMAND_POSITION_SPEED = 0,
    MOTOR_COMMAND_SPEED,
    MOTOR_COMMAND_MIT
} MotorCommandMode_t;

/* 控制函数生成目标，只有 MotorControl_Task 将其发送到电机。 */
typedef struct {
    MotorCommandMode_t mode;
    MotorTarget_t target;
    uint8_t enabled;
} MotorCommand_t;

extern const uint8_t left_arm_ids[LEFT_ARM_MOTOR_COUNT];
extern const uint8_t right_arm_ids[RIGHT_ARM_MOTOR_COUNT];
extern const can_port_t motor_ports[TOTAL_MOTOR_COUNT];
extern const float init_angles[TOTAL_MOTOR_COUNT];
extern ArmMotors_Info_t Switch_Motors;
extern float Motor_TargetAngles[CONTROLLED_MOTOR_COUNT];

/* 单写者 RobotManager 提交请求，控制任务在下一个周期边界采纳。 */
void MotorControl_RequestMode(RobotMode_t mode);
void MotorControl_Task(void *argument);

#endif /* MOTOR_CONTROL_H */
