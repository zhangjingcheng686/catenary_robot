#ifndef ROBOT_MESSAGES_H
#define ROBOT_MESSAGES_H

#include <stdint.h>

#define ROBOT_MOTOR_COUNT  9
#define TEACH_TARGET_COUNT 7

/* 示教目标，输入含义沿用当前示教器协议。 */
typedef struct {
    /* 左1、左2、左夹爪、右1、右2、右3、右夹爪，单位为度。 */
    float input_deg[TEACH_TARGET_COUNT];
    uint32_t receive_time_ms;
} TeachTarget_t;

/* 为后续 MIT 控制预留完整目标字段。 */
typedef struct {
    float position;       /* rad */
    float velocity;       /* rad/s */
    float kp;
    float kd;
    float torque_ff;      /* N·m */
} MotorTarget_t;

typedef struct {
    MotorTarget_t motor[ROBOT_MOTOR_COUNT];
    uint32_t sequence;
    uint32_t receive_time_ms;
} RLCommand_t;

/* 单个电机的反馈。 */
typedef struct {
    float position;       /* rad */
    float torque;         /* N·m */
    uint32_t last_receive_time_ms;
    uint8_t error_code;
    uint8_t valid;
} MotorFeedbackItem_t;

/* 一份消息包含全部9个电机的反馈。 */
typedef struct {
    MotorFeedbackItem_t motor[ROBOT_MOTOR_COUNT];
    uint32_t sequence;
    uint32_t publish_time_ms;
} MotorFeedback_t;

#endif
