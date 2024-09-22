/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "globals.h"
#include "mqtt_app.h"
#include "os.h"
#include "spi_app.h"

RtosDefineTaskSized(spi_app_task, spi_app_thread, 1024);

void app_main(void) {
	ESP_LOGI(TAG, "[APP] Startup..");
	ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	RtosStaticTaskCreate(spi_app_task, configMAX_PRIORITIES, NULL);

	vTaskStartScheduler();

	// TODO: test and add back mqtt stuff later
	// mqtt5_app_start();
}
