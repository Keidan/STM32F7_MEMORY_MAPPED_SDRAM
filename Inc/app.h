/*
 * @file app.h
 * @author Keidan
 * @copyright GNU LESSER GENERAL PUBLIC LICENSE Version 3
 */

#ifndef __APP_H__
#define __APP_H__


#include "cmsis_os.h"

/**
 * @brief Init the SDRAM and configure the MPU.
 */
void app_init(void);


/**
 * @brief Called by the FreeRTOS main task.
 */
void app_task_loop(void);

#endif /* __APP_H__ */
