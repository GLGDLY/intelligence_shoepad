#ifndef _CONFIG_H
#define _CONFIG_H

#include <esp_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEBUG 1

#define TAG "mainboard"

typedef enum {
	GPIO_LOW,
	GPIO_HIGH,
	NUM_OF_GPIO_LEVEL,
} GPIO_LEVEL;

#endif // _CONFIG_H