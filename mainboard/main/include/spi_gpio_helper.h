#ifndef _SPI_GPIO_HELPER_H
#define _SPI_GPIO_HELPER_H

#include "macro_utils.h"
#include "os.h"

#include <driver/gpio.h>


/* Configs */
#define SPI_CS_TABLE(X) \
	X(A, GPIO_NUM_31)   \
	X(B, GPIO_NUM_30)   \
	X(C, GPIO_NUM_29)

#define SPI_DRDY_TABLE(X) \
	X(GPIO_NUM_18)        \
	X(GPIO_NUM_17)        \
	X(GPIO_NUM_16)        \
	X(GPIO_NUM_15)        \
	X(GPIO_NUM_7)         \
	X(GPIO_NUM_6)         \
	X(GPIO_NUM_5)         \
	X(GPIO_NUM_4)

/* Enums */
#define X_EXPAND_CS_DECODER_ENUM(NAME, ...) CS_DEC_##NAME,
typedef enum {
	SPI_CS_TABLE(X_EXPAND_CS_DECODER_ENUM) NUM_OF_CS_DEC,
} SPI_CS_DEC;
#undef X_EXPAND_CS_DECODER_ENUM

/* Defines */
#define NUM_OF_SPI_DEV (0 SPI_DRDY_TABLE(X_EXPAND_CNT))

/* Macros */
#define FOR_EACH_SPI_CS_BIT(i) for (int i = 0; i < NUM_OF_CS_DEC; i++)
#define FOR_EACH_SPI_DEV(i)	   for (int i = 0; i < NUM_OF_SPI_DEV; i++)

/* Function prototypes */
void spi_cs_init(void);
void spi_cs(uint8_t dev_id);

void spi_drdy_init(void);
uint32_t spi_drdy_get(void);

#endif // _SPI_GPIO_HELPER_H