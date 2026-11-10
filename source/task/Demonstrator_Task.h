#ifndef DEMONSTRATOR_TASK_H
#define DEMONSTRATOR_TASK_H

#include "can_struct.h"

/* --- 运动速度定义 --- */
#define DEMO_FOLLOW_VELOCITY          3.0f     // 普通关节跟随速度

/* --- 电机与数组索引定义 --- */
#define LEFT_CLAW_ARRAY_INDEX          2        // 左夹爪在左臂指针数组中的位置
#define RIGHT_CLAW_ARRAY_INDEX         3        // 右夹爪在右臂指针数组中的位置

// 新增：特定普通关节的全局索引
#define LEFT_JOINT_2_INDEX             1        // 左2电机在全局的目标数组索引
#define RIGHT_JOINT_3_INDEX            5        // 右3电机在全局的目标数组索引

/* --- 示教器夹爪输入与纯力矩输出范围 --- */
#define CLAW_INPUT_MIN_DEG            -90.0f
#define CLAW_INPUT_LOW_DEG            -45.0f
#define CLAW_INPUT_DEADBAND_MIN_DEG   -10.0f
#define CLAW_INPUT_DEADBAND_MAX_DEG    10.0f
#define CLAW_INPUT_HIGH_DEG            45.0f
#define CLAW_INPUT_MAX_DEG             90.0f
#define CLAW_TORQUE_MIN_NM             -12.0f
#define CLAW_TORQUE_LOW_NM             -8.0f
#define CLAW_TORQUE_HIGH_NM             8.0f
#define CLAW_TORQUE_MAX_NM              12.0f

/* --- 普通关节绝对角度安全门禁 --- */
#define NORMAL_JOINT_TARGET_MIN_DEG   -120.0f
#define NORMAL_JOINT_TARGET_MAX_DEG    120.0f

// 新增：左2与右3专属的物理限位
#define SPECIAL_JOINT_TARGET_MIN_DEG  -120.0f
#define SPECIAL_JOINT_TARGET_MAX_DEG   120.0f

void Demonstrator_Task(void *argument);

#endif /* DEMONSTRATOR_TASK_H */
