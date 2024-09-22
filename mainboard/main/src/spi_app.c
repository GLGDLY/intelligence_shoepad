#include "spi_app.h"

#include "MLX90393_cmds.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "globals.h"
#include "os.h"
#include "portmacro.h"
#include "spi_gpio_helper.h"

#include <driver/spi_master.h>
#include <sdkconfig.h>


/* Globals */
spi_device_handle_t spi;
SemaphoreHandle_t spi_mux = NULL;

uint32_t dev_ready = 0;
portMUX_TYPE dev_ready_lock = portMUX_INITIALIZER_UNLOCKED;

mlx90393_data_t mlx90393_data[NUM_OF_SPI_DEV] = {0};

const uint32_t bitfield_all_spi_dev_ready = (1 << NUM_OF_SPI_DEV) - 1;

/* Methods */
void spi_app_init(void) {
	spi_mux = xSemaphoreCreateMutex();

	ESP_LOGI(TAG, "Initializing bus SPI%d...", SPI2_HOST + 1);

	spi_bus_config_t buscfg = {
		.miso_io_num = SPI_PIN_MISO, // MISO
		.mosi_io_num = SPI_PIN_MOSI, // MOSI
		.sclk_io_num = SPI_PIN_CLK,	 // SCLK
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
	};

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = SPI_MASTER_FREQ_10M, // 10 MHz
		.mode = 3,							   // CPOL = 1, CPHA = 1
		.spics_io_num = -1,
		.queue_size = 8,
	};

	// Initialize the SPI bus
	esp_err_t ret;
	ret = spi_bus_initialize(SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);
	ret = spi_bus_add_device(SPI_HOST_ID, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);

	spi_cs_init();
	spi_drdy_init();
}

void spi_tx_request(spi_cmd_t* cmd) {
	if (cmd->len <= 0)
		return;

	spi_cs(cmd->dev_id);

	xSemaphoreTake(spi_mux, portMAX_DELAY);

	spi_transaction_t tx = {
		.length = cmd->len,
		.tx_buffer = cmd->tx_data,
		.rx_buffer = cmd->rx_data,
	};
	esp_err_t ret = spi_device_polling_transmit(spi, &tx);
	ESP_ERROR_CHECK(ret);

	xSemaphoreGive(spi_mux);
}

void spi_drdy_intr_handler(void* arg) {
	taskENTER_CRITICAL_ISR(&dev_ready_lock);
	dev_ready |= 1 << ((uint64_t)arg);
	taskEXIT_CRITICAL_ISR(&dev_ready_lock);

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (dev_ready == bitfield_all_spi_dev_ready) {
		RtosStaticTask_t spi_app_task;
		xHigherPriorityTaskWoken = pdTRUE;
		vTaskNotifyGiveFromISR(spi_app_task.handle, &xHigherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void spi_app_thread(void* par) {
	spi_app_init();

	FOR_EACH_SPI_DEV(i) {
		while (1) {
			mlx90393_status_t status = mlx90393_SB_request(i);
			if (mlx90393_RM_data_is_valid(status)) {
				break;
			} else {
				ESP_LOGE(TAG, "Init SPI dev: %d failed", i);
			}
			delay(100);
		}
	}

	while (1) {
		uint32_t drdy = spi_drdy_get();
		if (drdy) {
			FOR_EACH_SPI_DEV(i) {
				if (drdy & (1 << i)) {
					mlx90393_data[i] = mlx90393_RM_request(i);
				} else {
					memset(&mlx90393_data[i], 0, sizeof(mlx90393_data_t));
				}
			}
		}

#ifdef DEBUG
		FOR_EACH_SPI_DEV(i) {
			ESP_LOGI(TAG, "Dev: %d, T: %d, X: %d, Y: %d, Z: %d", i, mlx90393_data[i].T, mlx90393_data[i].X,
					 mlx90393_data[i].Y, mlx90393_data[i].Z);
		}
		ESP_LOGI(TAG, "--------------------------------------------");
		delay(100);
#endif

		// wait for 100ms for all devices to be ready, else timeout and directly request data
		ulTaskNotifyTake(pdTRUE, ms_to_ticks(100));
	}
}
