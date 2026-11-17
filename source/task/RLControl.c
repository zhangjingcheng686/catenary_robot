/**
 * @file RLControl.c
 * @brief RL 控制周期入口，保留原有空动作框架，上位机指令后续接入。
 */

#include "RLControl.h"
#include "DBusSys.h"
#include <string.h>
#include <math.h>

StepJoint_t J[10];
CrabWalk_Mode_t crab_walk_mode = CRAB_MODE_WIRE;
Action1Control_t action1_ctrl = { .state = ACT1_IDLE };
Action2Control_t action2_ctrl = { .state = ACT2_IDLE };
Action3Control_t action3_ctrl = { .state = ACT3_IDLE };

void CheckCrabWalkModeSwitch(void)
{
    int chD = remoter.rc.ch[10]; 

    if (chD < 980 ) {
        crab_walk_mode = CRAB_MODE_WIRE;
    }else{
         crab_walk_mode = CRAB_MODE_POLE;
    }
}

float GetClawAngle(int claw_type, int position)
{
    // claw_type: 0=left, 1=right
    // position: 0=open, 1=closed, 2=half
    if (crab_walk_mode == CRAB_MODE_WIRE) {
        switch (claw_type) {
            case 0: 
                switch (position) {
                    case 0: return LEFT_CLAW_OPEN_WIRE;
                    case 1: return LEFT_CLAW_CLOSED_WIRE;
                    case 2: return LEFT_CLAW_HALF_WIRE;
                    default: return LEFT_CLAW_HALF_WIRE;
                }
            case 1: 
                switch (position) {
                    case 0: return RIGHT_CLAW_OPEN_WIRE;
                    case 1: return RIGHT_CLAW_CLOSED_WIRE;
                    case 2: return RIGHT_CLAW_HALF_WIRE;
                    default: return RIGHT_CLAW_HALF_WIRE;
                }
            default: return 0.0f;
        }
    } else { 
        switch (claw_type) {
            case 0: 
                switch (position) {
                    case 0: return LEFT_CLAW_OPEN_POLE;
                    case 1: return LEFT_CLAW_CLOSED_POLE;
                    case 2: return LEFT_CLAW_HALF_POLE;
                    default: return LEFT_CLAW_HALF_POLE;
                }
            case 1: 
                switch (position) {
                    case 0: return RIGHT_CLAW_OPEN_POLE;
                    case 1: return RIGHT_CLAW_CLOSED_POLE;
                    case 2: return RIGHT_CLAW_HALF_POLE;
                    default: return RIGHT_CLAW_HALF_POLE;
                }
            default: return 0.0f;
        }
    }
}

void ResetAction1State(void) 
{
    action1_ctrl.state = ACT1_IDLE;
    action1_ctrl.state_start_time = 0;
    
    for (int i = 0; i < 6; i++) {
        action1_ctrl.start_angles[i] = 0.0f;
    }

    for (int i = 0; i < 8; i++) {
        StepJoint_t* js = &J[i];
        js->start_deg  = 0.0f;
        js->target_deg = 0.0f;
        js->t0_ms      = 0;
        js->m          = NULL; 
    }
}

void ResetAction2State(void)
{
    action2_ctrl.state = ACT2_IDLE;
    action2_ctrl.state_start_time = 0;
    action2_ctrl.step_inited = false;
    action2_ctrl.claw_wait_start = 0;
    action2_ctrl.move_start_time = 0;
}

void ResetAction3State(void)
{
    action3_ctrl.state = ACT3_IDLE;
    action3_ctrl.state_start_time = 0;
    action3_ctrl.step_inited = false;
    action3_ctrl.claw_wait_start = 0;
    action3_ctrl.move_start_time = 0;
}

void Action1_Control(void) 
{

}

void Action2_Control(void)
{

}

void Action3_Control(void)
{

}


static ArmControl_Mode_t last_mode = MODE_IDLE;

void RLControl_Reset(void)
{
    ResetAction1State();
    ResetAction2State();
    ResetAction3State();
    last_mode = MODE_IDLE;
}

void RLControl_Update(MotorCommand_t commands[TOTAL_MOTOR_COUNT])
{
    /* RLCommandQueue 与上位机协议暂未接入，现有空动作不改变目标。 */
    (void)commands;
    CheckCrabWalkModeSwitch();
    
    int ch5 = remoter.rc.ch[5];
    ArmControl_Mode_t current_mode;

    if (ch5 < CH5_LOW_THRESHOLD) {
        current_mode = MODE_ACTION1;       
    } else if (ch5 >= CH5_LOW_THRESHOLD && ch5 <= CH5_HIGH_THRESHOLD) {
        current_mode = MODE_ACTION2; 
    } else {
        current_mode = MODE_ACTION3;
    }

    if (current_mode != last_mode) {
        switch (last_mode) {
            case MODE_ACTION1: ResetAction1State(); break;
            case MODE_ACTION2: ResetAction2State(); break; 
            case MODE_ACTION3: ResetAction3State(); break; 
            default: break;
        }
  
        switch (current_mode) {
            case MODE_ACTION1: ResetAction1State(); break;
            case MODE_ACTION2: ResetAction2State(); break; 
            case MODE_ACTION3: ResetAction3State(); break; 
            default: break;
        }
    }

    switch (current_mode) {
        case MODE_ACTION1: Action1_Control(); break;
        case MODE_ACTION2: Action2_Control(); break;
        case MODE_ACTION3: Action3_Control(); break; 
        default: break;
    }

    last_mode = current_mode;
}
