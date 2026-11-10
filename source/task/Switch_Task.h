#ifndef SWITCH_TASK_H
#define SWITCH_TASK_H

#include "motor_DM.h"
#include "cmsis_os2.h"

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
#define KP_GAIN             35.0f      
#define KD_GAIN             1.5f    

typedef struct
{
    DM_motor_t *left_arm_motors[LEFT_ARM_MOTOR_COUNT];
    DM_motor_t *right_arm_motors[RIGHT_ARM_MOTOR_COUNT];
} ArmMotors_Info_t;

extern osThreadId_t ArmControlHandle;
extern osThreadId_t ClawControlHandle;
extern osThreadId_t DemonstratorHandle;

extern const uint8_t left_arm_ids[LEFT_ARM_MOTOR_COUNT];
extern const uint8_t right_arm_ids[RIGHT_ARM_MOTOR_COUNT];

extern const can_port_t motor_ports[TOTAL_MOTOR_COUNT]; 
extern const float init_angles[TOTAL_MOTOR_COUNT]; 

extern ArmMotors_Info_t Switch_Motors;
extern float Motor_TargetAngles[CONTROLLED_MOTOR_COUNT];

void Motors_InitAll(void);
void ApplyInitPose(void);
void Switch_Motors_RefreshAll(void);

#endif // SWITCH_TASK_H
