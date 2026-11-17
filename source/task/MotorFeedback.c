#include "MotorFeedback.h"
#include "MotorControl.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

extern osMessageQueueId_t MotorFeedbackQueueHandle;

_Static_assert(ROBOT_MOTOR_COUNT == TOTAL_MOTOR_COUNT,
               "Motor feedback must include every motor");

static void RefreshMotorFeedback(DM_motor_t *motor, MotorFeedbackItem_t *feedback)
{
    *feedback = (MotorFeedbackItem_t){0};
    if (motor == NULL) {
        return;
    }

    /* Keep the existing refresh path; CAN reception continues in the ISR. */
    Motor_DM_Refresh(motor);

    if (motor->motor_msg.can_msg.len != 8U) {
        return;
    }

    feedback->position = motor->motor_msg.motor_angle;
    feedback->torque = motor->motor_msg.torque_current;
    feedback->error_code = (uint8_t)motor->error_code;
    feedback->last_receive_time_ms = motor->motor_msg.can_msg.last_receive_time_ms;
    feedback->valid = (HAL_GetTick() - feedback->last_receive_time_ms <=
                       MOTOR_FEEDBACK_TIMEOUT_MS);
}

bool MotorFeedback_GetLatest(MotorFeedback_t *feedback)
{
    if (feedback == NULL || MotorFeedbackQueueHandle == NULL) {
        return false;
    }

    return xQueuePeek((QueueHandle_t)MotorFeedbackQueueHandle, feedback, 0) == pdPASS;
}

void MotorFeedback_Task(void *argument)
{
    (void)argument;

    configASSERT(MotorFeedbackQueueHandle != NULL);
    configASSERT(osMessageQueueGetCapacity(MotorFeedbackQueueHandle) == 1U);
    configASSERT(osMessageQueueGetMsgSize(MotorFeedbackQueueHandle) == sizeof(MotorFeedback_t));

    /* MotorControl sets this flag after all motor objects are initialized. */
    osThreadFlagsWait(MOTOR_FEEDBACK_READY_FLAG, osFlagsWaitAny, osWaitForever);

    MotorFeedback_t feedback = {0};
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        /* Queue order: left motors 1..4, followed by right motors 1..5. */
        for (int i = 0; i < LEFT_ARM_MOTOR_COUNT; i++) {
            RefreshMotorFeedback(Switch_Motors.left_arm_motors[i], &feedback.motor[i]);
        }
        for (int i = 0; i < RIGHT_ARM_MOTOR_COUNT; i++) {
            RefreshMotorFeedback(Switch_Motors.right_arm_motors[i],
                                 &feedback.motor[LEFT_ARM_MOTOR_COUNT + i]);
        }

        feedback.sequence++;
        feedback.publish_time_ms = HAL_GetTick();
        xQueueOverwrite((QueueHandle_t)MotorFeedbackQueueHandle, &feedback);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_FEEDBACK_PERIOD_MS));
    }
}
