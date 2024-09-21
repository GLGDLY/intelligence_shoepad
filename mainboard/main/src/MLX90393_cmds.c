#include "MLX90393_cmds.h"

#include <string.h>


#define X_EXPAND_MLX90393_CMDS(NAME, CMD, RX_LEN) {#NAME, CMD, RX_LEN},
const mlx90393_cmds_t MLX90393_CMD_OBJECTS[] = {MLX90393_CMDS_TABLE(X_EXPAND_MLX90393_CMDS)};

mlx90393_cmds_t mlx90393_cmds_get(const MLX90393_CMDS cmd) { return MLX90393_CMD_OBJECTS[cmd]; }

static mlx90393_status_t _status_ret_cmds_request(const uint8_t dev_id, const MLX90393_CMDS cmd) {
	const mlx90393_cmds_t cmd_obj = mlx90393_cmds_get(cmd);

	const uint8_t buf_len = cmd_obj.rx_len + 1;

	uint8_t tx_buf[buf_len];
	uint8_t rx_buf[buf_len];
	memset(tx_buf + 1, 0, buf_len - 1);
	memset(rx_buf, 0, buf_len);
	tx_buf[0] = cmd_obj.cmd;

	spi_cmd_t cmd_req = {
		.dev_id = dev_id,
		.tx_data = tx_buf,
		.rx_data = rx_buf,
		.len = buf_len,
	};
	spi_tx_request(&cmd_req);

	mlx90393_status_t status = {.raw = cmd_req.rx_data[1]};
	return status;
}

mlx90393_status_t mlx90393_SB_request(const uint8_t dev_id) { return _status_ret_cmds_request(dev_id, MLX90393_SB); }

mlx90393_status_t mlx90393_SW_request(const uint8_t dev_id) { return _status_ret_cmds_request(dev_id, MLX90393_SW); }

mlx90393_status_t mlx90393_SM_request(const uint8_t dev_id) { return _status_ret_cmds_request(dev_id, MLX90393_SM); }

mlx90393_status_t mlx90393_RT_request(const uint8_t dev_id) { return _status_ret_cmds_request(dev_id, MLX90393_RT); }

mlx90393_data_t mlx90393_RM_request(const uint8_t dev_id) {
	const mlx90393_cmds_t cmd_obj = mlx90393_cmds_get(MLX90393_RM);

	const uint8_t buf_len = 10; // cmd_obj.rx_len + 1

	uint8_t tx_buf[buf_len];
	uint8_t rx_buf[buf_len];
	memset(tx_buf + 1, 0, buf_len - 1);
	memset(rx_buf, 0, buf_len);
	tx_buf[0] = cmd_obj.cmd;

	spi_cmd_t cmd_req = {
		.dev_id = dev_id,
		.tx_data = tx_buf,
		.rx_data = rx_buf,
		.len = buf_len,
	};
	spi_tx_request(&cmd_req);

	mlx90393_data_t data;
	memcpy(data.raw, cmd_req.rx_data + 1, cmd_obj.rx_len);

	return data;
}

bool mlx90393_RM_data_is_valid(const mlx90393_status_t data) { return !data.error; }
