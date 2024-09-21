#include "spi_app.h"

#include "MLX90393_cmds.h"
#include "esp_log.h"
#include "globals.h"
#include "os.h"
#include "portmacro.h"
#include "spi_gpio_helper.h"

#include <driver/spi_master.h>
#include <sdkconfig.h>


/* Globals */
spi_device_handle_t spi;
SemaphoreHandle_t spi_mux = NULL;
SemaphoreHandle_t spi_drdy_sem = NULL;

mlx90393_data_t mlx90393_data[NUM_OF_SPI_DEV] = {0};

/* Methods */
void spi_app_init(void) {
	spi_mux = xSemaphoreCreateMutex();
	spi_drdy_sem = xSemaphoreCreateBinary();

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
	spi_drdy_init(&spi_drdy_sem);
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
		xSemaphoreTake(spi_drdy_sem, ms_to_ticks(1000));
	}
}
