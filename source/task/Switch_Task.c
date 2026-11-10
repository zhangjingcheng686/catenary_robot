#include "Switch_Task.h"
#include "DBusSys.h"
#include "cmsis_os2.h"
#include "motor_DM.h"
#include "can_bsp.h"
#include "ArmControl_Task.h"  

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

void Motors_InitAll(void)
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

void ApplyInitPose(void)
{
    float velocity = 8.0f;

    for (int i = 0; i < LEFT_ARM_MOTOR_COUNT; i++) {
        if (Switch_Motors.left_arm_motors[i]) {
            float ang = init_angles[i];
            float raw_target_rad = ang * (3.1415926f / 180.0f);

            if (i == LEFT_CLAW_MOTOR_INDEX) {
                MIT_CtrlMotorDM(Switch_Motors.left_arm_motors[i],
                                0.0f, 0.0f, KP_GAIN, KD_GAIN, 0.0f);
            } else if (i == LEFT_SCREW_MOTOR_INDEX) {
                Speed_CtrlMotorDM(Switch_Motors.left_arm_motors[i], 0.0f);
            } else {
                PosSpeed_CtrlMotorDM(Switch_Motors.left_arm_motors[i], raw_target_rad, velocity);
            }
            if (i < LEFT_CONTROLLED_MOTOR_COUNT) {
                Motor_TargetAngles[i] = ang;
            }
            osDelay(10);
        }
    }

    for (int i = 0; i < RIGHT_ARM_MOTOR_COUNT; i++) {
        if (Switch_Motors.right_arm_motors[i]) {
            float ang = init_angles[i + LEFT_ARM_MOTOR_COUNT];
            float raw_target_rad = ang * (3.1415926f / 180.0f);

            if (i == RIGHT_CLAW_MOTOR_INDEX) {
                MIT_CtrlMotorDM(Switch_Motors.right_arm_motors[i],
                                0.0f, 0.0f, KP_GAIN, KD_GAIN, 0.0f);
            } else if (i == RIGHT_SCREW_MOTOR_INDEX) {
                Speed_CtrlMotorDM(Switch_Motors.right_arm_motors[i], 0.0f);
            } else {
                PosSpeed_CtrlMotorDM(Switch_Motors.right_arm_motors[i], raw_target_rad, velocity);
            }
            if (i < RIGHT_CONTROLLED_MOTOR_COUNT) {
                Motor_TargetAngles[i + LEFT_CONTROLLED_MOTOR_COUNT] = ang;
            }
            osDelay(10);
        }
    }
}

void Switch_Motors_RefreshAll(void)
{
    for (int i = 0; i < LEFT_ARM_MOTOR_COUNT; i++) {
        if (Switch_Motors.left_arm_motors[i]) Motor_DM_Refresh(Switch_Motors.left_arm_motors[i]);
    }
    for (int i = 0; i < RIGHT_ARM_MOTOR_COUNT; i++) {
        if (Switch_Motors.right_arm_motors[i]) Motor_DM_Refresh(Switch_Motors.right_arm_motors[i]);
    }
}

static void StopMitClaws(void)
{
    DM_motor_t *left_claw = Switch_Motors.left_arm_motors[LEFT_CLAW_MOTOR_INDEX];
    DM_motor_t *right_claw = Switch_Motors.right_arm_motors[RIGHT_CLAW_MOTOR_INDEX];

    if (left_claw != NULL) {
        MIT_CtrlMotorDM(left_claw, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    if (right_claw != NULL) {
        MIT_CtrlMotorDM(right_claw, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

static void StopScrewMotors(void)
{
    DM_motor_t *left_screw = Switch_Motors.left_arm_motors[LEFT_SCREW_MOTOR_INDEX];
    DM_motor_t *right_screw = Switch_Motors.right_arm_motors[RIGHT_SCREW_MOTOR_INDEX];

    if (left_screw != NULL) {
        Speed_CtrlMotorDM(left_screw, 0.0f);
    }
    if (right_screw != NULL) {
        Speed_CtrlMotorDM(right_screw, 0.0f);
    }
}

void Switch_Task(void *argument)
{
    (void)argument;
    osDelay(1000);
    Motors_InitAll(); 

    static int current_mode = 0;
    for (;;) {
        Switch_Motors_RefreshAll();

        int ch4 = remoter.rc.ch[4]; 

        if (ch4 < 980) {
            /* 速度模式电机在任务挂起后仍保持最后一次速度指令，先明确停车。 */
            StopScrewMotors();
            if (current_mode == 2) {
                StopMitClaws();
            }
            if (ClawControlHandle)     osThreadSuspend(ClawControlHandle);
            if (DemonstratorHandle)    osThreadSuspend(DemonstratorHandle); 
            if (ArmControlHandle)      osThreadResume(ArmControlHandle);   

            current_mode = 1;
        }
        else if (ch4 > 1020) {
            if (ClawControlHandle)     osThreadResume(ClawControlHandle);
            if (ArmControlHandle)      osThreadSuspend(ArmControlHandle);   
            if (DemonstratorHandle)    osThreadResume(DemonstratorHandle); 

            current_mode = 2;
        }
        else {
            if (ArmControlHandle)      osThreadSuspend(ArmControlHandle);
            if (DemonstratorHandle)    osThreadSuspend(DemonstratorHandle); 

            if (current_mode != 0) {
                ApplyInitPose();
            }

            if (ClawControlHandle)     osThreadResume(ClawControlHandle);

            current_mode = 0;
        }
        
        osDelay(20); 
    }
}
