/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TASK_PRIO          (configMAX_PRIORITIES - 1)
#define CONSUMER_LINE_SIZE 3
#define BUFFER_SIZE 5u
SemaphoreHandle_t xSemaphore_writter;
SemaphoreHandle_t xSemaphore_reader;
SemaphoreHandle_t xSemaphore_counting;

typedef struct {
    uint8_t data[BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} CircularBuffer;

uint8_t message[] = {"ITESO Rules\0"};
CircularBuffer buffer;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void writter_task(void *pvParameters);
static void reader_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Inits the buffer*/
    buffer.head = 0;
    buffer.tail = 0;
    xSemaphore_counting = xSemaphoreCreateCounting(BUFFER_SIZE, BUFFER_SIZE);

    if (xTaskCreate(writter_task, "WRITTER_TASK", configMINIMAL_STACK_SIZE + 128, NULL, TASK_PRIO, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    if (xTaskCreate(reader_task, "READER_TASK", configMINIMAL_STACK_SIZE + 128, NULL, TASK_PRIO, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    /* Start scheduling. */
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task producer_task.
 */
static void writter_task(void *pvParameters)
{
   for(int i = 0; i < BUFFER_SIZE; i++)
   {
        xSemaphoreTake(xSemaphore_counting, portMAX_DELAY);
        buffer.data[buffer.head] = message[i];
        buffer.head = (buffer.head + 1) % BUFFER_SIZE;
        taskYIELD();
   }
}

/*!
 * @brief Task consumer_task.
 */
static void reader_task(void *pvParameters)
{
    for(int i = 0; i < (BUFFER_SIZE / 2); i++)
    {
        /**We "read" but only make a give to say we have available space*/
        xSemaphoreGive(xSemaphore_counting);
        taskYIELD();
    }
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        PRINTF("Buffer[%d] = %c ",i,buffer.data[i] );
    }
}
