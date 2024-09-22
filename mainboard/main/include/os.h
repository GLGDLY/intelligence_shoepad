#ifndef _OS_H
#define _OS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/*
(TaskFunction_t pxTaskCode, const char* const pcName,
											 const uint32_t ulStackDepth, void* const pvParameters,
											 UBaseType_t uxPriority, StackType_t* const puxStackBuffer,
											 StaticTask_t* const pxTaskBuffer)
 */

#define RtosStaticTaskSized_t(size)     \
	struct {                            \
		const char* const name;         \
		const uint32_t stack_size;      \
		TaskFunction_t func;            \
		StaticTask_t tb;                \
		TaskHandle_t handle;            \
		StackType_t stack_buffer[size]; \
	}
#define RtosStaticTask_t RtosStaticTaskSized_t()

#define RtosDefineTaskSized(task_to_create, task_func, size) \
	RtosStaticTaskSized_t(size) task_to_create = {           \
		.stack_buffer = {0},                                 \
		.name = #task_func,                                  \
		.stack_size = size,                                  \
		.func = task_func,                                   \
		.tb = {0},                                           \
		.handle = NULL,                                      \
	}

#define RtosDefineTask(task_to_create, task_func) RtosDefineTaskSized(task_to_create, task_func, 256)

#define RtosStaticTaskCreate(task, prio, par)                                                                        \
	do {                                                                                                             \
		task.handle =                                                                                                \
			xTaskCreateStatic(task.func, task.name, task.stack_size, (void*)par, prio, task.stack_buffer, &task.tb); \
	} while (0)

#define ms_to_ticks(ms) (ms / portTICK_PERIOD_MS)

#define delay(ms)		   vTaskDelay(ms_to_ticks(ms))
#define delay_until(t, ms) vTaskDelayUntil(t, ms_to_ticks(ms))

#endif // _OS_H