#ifndef _MLX90393_CMDS_H
#define _MLX90393_CMDS_H

#include "spi_app.h"

#include <stdbool.h>

/* Configs */
#define MLX90393_CMDS_TABLE(X) \
	X(SB, 0x1F, 1)             \
	X(SW, 0x2F, 1)             \
	X(SM, 0x3F, 1)             \
	X(RM, 0x4F, 9)             \
	X(RT, 0xF0, 1)

/* Enums */
#define X_EXPAND_MLX90393_CMDS_EMUM(NAME, ...) MLX90393_##NAME,
typedef enum {
	MLX90393_CMDS_TABLE(X_EXPAND_MLX90393_CMDS_EMUM) NUM_OF_MLX90393_CMDS,
} MLX90393_CMDS;
#undef X_EXPAND_MLX90393_CMDS_EMUM

/* Structs */
typedef struct {
	const char* const name;
	const uint8_t cmd;
	const uint8_t rx_len;
} mlx90393_cmds_t;

typedef union {
	uint8_t raw;
	struct {
		uint8_t d : 2;
		uint8_t rs : 1;
		uint8_t sed : 1;
		uint8_t error : 1;
		uint8_t is_sm : 1;
		uint8_t is_woc : 1;
		uint8_t is_burst : 1;
	};
} mlx90393_status_t;

typedef union {
	uint8_t raw[9];
	struct {
		mlx90393_status_t status;
		uint16_t T;
		uint16_t X;
		uint16_t Y;
		uint16_t Z;
	};
} mlx90393_data_t;

/* Function prototypes */
mlx90393_cmds_t mlx90393_cmds_get(const MLX90393_CMDS cmd);

mlx90393_status_t mlx90393_SB_request(const uint8_t dev_id);
mlx90393_status_t mlx90393_SW_request(const uint8_t dev_id);
mlx90393_status_t mlx90393_SM_request(const uint8_t dev_id);
mlx90393_status_t mlx90393_RT_request(const uint8_t dev_id);
mlx90393_data_t mlx90393_RM_request(const uint8_t dev_id);

bool mlx90393_RM_data_is_valid(const mlx90393_status_t data);


#endif // _MLX90393_CMDS_H