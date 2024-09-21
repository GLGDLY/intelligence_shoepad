#ifndef _SPI_APP_H
#define _SPI_APP_H

#include <stddef.h>
#include <stdint.h>

/* Configs */
#define SPI_PIN_MISO GPIO_NUM_13
#define SPI_PIN_MOSI GPIO_NUM_14
#define SPI_PIN_CLK	 GPIO_NUM_21
#define SPI_HOST_ID	 SPI2_HOST

/* Structs */
typedef struct {
	uint8_t dev_id;
	uint8_t *tx_data, *rx_data;
	size_t len;
} spi_cmd_t;

/* Function prototypes */
void spi_app_init(void);
void spi_app_thread(void* par);

void spi_tx_request(spi_cmd_t* cmd);

#endif // _SPI_APP_H