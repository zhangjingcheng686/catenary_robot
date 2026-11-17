#ifndef ARMCONTROL_TASK_H
#define ARMCONTROL_TASK_H

#include "motor_DM.h"
#include "cmsis_os2.h"
#include "MotorControl.h"
#include <stdbool.h>

#define RAD_TO_DEG (180.0f / 3.1415926f)
#define DEG_TO_RAD (3.1415926f / 180.0f)

#define KP_GRIP                      80.0f
#define KD_GRIP                       2.5f
#define TFF_GRIP                      8.0f  

#define CH5_LOW_THRESHOLD             980
#define CH5_HIGH_THRESHOLD            1020
#define CLAW_CLOSE_WAIT_MS            1000
#define CRAB_REACH_TIMEOUT_MS         3000

#define LEFT_CLAW_OPEN_WIRE           0.0f
#define LEFT_CLAW_CLOSED_WIRE       -79.11f
#define LEFT_CLAW_HALF_WIRE         -62.19f
#define RIGHT_CLAW_OPEN_WIRE          0.0f
#define RIGHT_CLAW_CLOSED_WIRE      -68.11f
#define RIGHT_CLAW_HALF_WIRE        -58.19f

#define LEFT_CLAW_OPEN_POLE           0.0f
#define LEFT_CLAW_CLOSED_POLE       -68.11f
#define LEFT_CLAW_HALF_POLE         -48.19f
#define RIGHT_CLAW_OPEN_POLE          0.0f
#define RIGHT_CLAW_CLOSED_POLE      -68.11f
#define RIGHT_CLAW_HALF_POLE        -48.19f

typedef enum {
    CRAB_MODE_WIRE = 0,  
    CRAB_MODE_POLE = 1   
} CrabWalk_Mode_t;

typedef enum {
    MODE_IDLE = 0,            
    MODE_ACTION1,             
    MODE_ACTION2,             
    MODE_ACTION3
} ArmControl_Mode_t;

typedef struct {
    DM_motor_t* m;
    float start_deg;
    float target_deg;
    uint32_t t0_ms;     
} StepJoint_t;

/**
 * @brief 动作1：初始动作
 */
typedef enum {
    ACT1_IDLE = 0,
    ACT1_STEP1,      
    ACT1_STEP2,      
    ACT1_STEP3,      
    ACT1_STEP4,      
    ACT1_STEP5,      
    ACT1_STEP6,      
    ACT1_COMPLETE
} Action1State_t;

typedef struct {
    Action1State_t state;
    uint32_t state_start_time;
    float start_angles[6];
} Action1Control_t;

/**
 * @brief 动作2：杆下水平蟹行
 */
typedef enum {
    ACT2_IDLE = 0,
    ACT2_STEP1,         
    ACT2_STEP2A,        
    ACT2_STEP2B,        
    ACT2_STEP2C,        
    ACT2_STEP2D,        
    ACT2_STEP2E,        
    ACT2_STEP3,         
    ACT2_STEP4A,        
    ACT2_STEP4B,        
    ACT2_STEP5A,        
    ACT2_STEP5B,        
    ACT2_STEP6,         
    ACT2_STEP7A,        
    ACT2_STEP7B,        
    ACT2_STEP8A,        
    ACT2_STEP8B,        
    ACT2_STEP9,         
    ACT2_COMPLETE
} Action2State_t;

typedef struct {
    Action2State_t state;
    uint32_t state_start_time;
    bool step_inited;
    uint32_t claw_wait_start;
    uint32_t move_start_time;
} Action2Control_t;

/**
 * @brief 动作3：杆上水平蟹行 
 */
typedef enum {
    ACT3_IDLE = 0,
    ACT3_STEP1,         
    ACT3_STEP2A,        
    ACT3_STEP2B,        
    ACT3_STEP2C,        
    ACT3_STEP2D,        
    ACT3_STEP2E,        
    ACT3_STEP3,         
    ACT3_STEP4A,        
    ACT3_STEP4B,
    ACT3_STEP4C,        
    ACT3_STEP5A,        
    ACT3_STEP5B,        
    ACT3_STEP6,         
    ACT3_STEP7A,        
    ACT3_STEP7B,        
    ACT3_STEP8A,        
    ACT3_STEP8B,        
    ACT3_STEP9,         
    ACT3_COMPLETE
} Action3State_t;

typedef struct {
    Action3State_t state;
    uint32_t state_start_time;
    bool step_inited;
    uint32_t claw_wait_start;
    uint32_t move_start_time;
} Action3Control_t;

extern Action1Control_t action1_ctrl;
extern Action2Control_t action2_ctrl;
extern Action3Control_t action3_ctrl;
extern CrabWalk_Mode_t crab_walk_mode;

void CheckCrabWalkModeSwitch(void);
float GetClawAngle(int claw_type, int position);

void Action1_Control(void);  
void Action2_Control(void);  
void Action3_Control(void);  

void ResetAction1State(void);
void ResetAction2State(void);
void ResetAction3State(void);

/* 当前动作函数仍为空；上位机协议与 RLCommandQueue 后续接入。 */
void RLControl_Reset(void);
void RLControl_Update(MotorCommand_t commands[TOTAL_MOTOR_COUNT]);

#endif // ARMCONTROL_TASK_H
