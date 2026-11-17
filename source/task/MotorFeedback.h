#ifndef MOTOR_FEEDBACK_H
#define MOTOR_FEEDBACK_H

#include <stdbool.h>
#include "cmsis_os2.h"
#include "robot_messages.h"

#define MOTOR_FEEDBACK_READY_FLAG  (1U << 0)
#define MOTOR_FEEDBACK_PERIOD_MS   20U
#define MOTOR_FEEDBACK_TIMEOUT_MS  500U

extern osThreadId_t MotorFeedbackHandle;

void MotorFeedback_Task(void *argument);

/* Copies the latest snapshot without removing it or waiting for an update. */
bool MotorFeedback_GetLatest(MotorFeedback_t *feedback);

#endif
