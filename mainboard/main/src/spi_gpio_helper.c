#include "spi_gpio_helper.h"

#include "globals.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"


/* Helper macros */
#define X_EXPAND_CS_DEC_CONSTRUCT(NAME, PIN) [CS_DEC_##NAME] = PIN,
#define X_EXPAND_CS_DEC_PIN_MASK(NAME, PIN)	 (1 << PIN) |

#define X_EXPAND_DRDY_CONSTRUCT(PIN) PIN,
#define X_EXPAND_DRDY_PIN_MASK(PIN)	 (1 << PIN) |

/* Constants */
const gpio_num_t SPI_CS_PINS[] = {SPI_CS_TABLE(X_EXPAND_CS_DEC_CONSTRUCT)};
const gpio_num_t SPI_DRDY_PINS[NUM_OF_SPI_DEV] = {SPI_DRDY_TABLE(X_EXPAND_DRDY_CONSTRUCT)};

void spi_cs_init(void) {
	esp_err_t ret;

	gpio_config_t conf = {
		.pin_bit_mask = SPI_CS_TABLE(X_EXPAND_CS_DEC_PIN_MASK) 0,
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = 0,
		.pull_down_en = 0,
		.intr_type = GPIO_INTR_DISABLE,
	};
	ret = gpio_config(&conf);
	ESP_ERROR_CHECK(ret);

	for (int i = 0; i < NUM_OF_CS_DEC; i++) {
		ret = gpio_set_level(SPI_CS_PINS[i], GPIO_LOW);
		ESP_ERROR_CHECK(ret);
	}
}

void spi_cs(uint8_t dev_id) {
	if (dev_id > NUM_OF_SPI_DEV || dev_id <= 0) {
		ESP_LOGE(TAG, "Invalid device ID: %d", dev_id);
		return;
	}

	for (int i = 0; i < NUM_OF_CS_DEC; i++) {
		if (dev_id & (1 << i)) {
			gpio_set_level(SPI_CS_PINS[i], GPIO_HIGH);
		} else {
			gpio_set_level(SPI_CS_PINS[i], GPIO_LOW);
		}
	}
}

__attribute__((weak)) void spi_drdy_intr_handler(void* arg){};

void spi_drdy_init(void) {
	esp_err_t ret;

	gpio_config_t conf = {
		.pin_bit_mask = SPI_DRDY_TABLE(X_EXPAND_DRDY_PIN_MASK) 0,
		.mode = GPIO_MODE_INPUT,
		.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type = GPIO_INTR_POSEDGE,
	};
	ret = gpio_config(&conf);
	ESP_ERROR_CHECK(ret);

	FOR_EACH_SPI_DEV(i) {
		ret = gpio_install_isr_service(0);
		ESP_ERROR_CHECK(ret);
		ret = gpio_isr_handler_add(SPI_DRDY_PINS[i], spi_drdy_intr_handler, (void*)(uint64_t)i);
		ESP_ERROR_CHECK(ret);
	}
}

uint32_t spi_drdy_get(void) {
	uint32_t drdy = 0;
	FOR_EACH_SPI_DEV(i) { drdy |= gpio_get_level(SPI_DRDY_PINS[i]) << i; }
	return drdy;
}
