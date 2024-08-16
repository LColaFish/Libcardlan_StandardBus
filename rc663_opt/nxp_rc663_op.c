/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "nxp_rc663_op.h"
#include "libcardlan_StandardBus_util.h"


#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#endif
#if defined(NDK_CODE_DEBUG)
#define TARGET_DEBUG
#endif

#if defined(TARGET_ANDROID)
#ifndef LOG_TAG
#define LOG_TAG __FILE__
#endif
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#elif defined(TARGET_DEBUG)
#include <stdio.h>
#define  LOGI    printf
#define  LOGD    printf
#define  LOGE    printf
#else 
#define  LOGI    
#define  LOGD    
#define  LOGE
#endif

static const char *TAG="CardlanDevCtrl";

static const char	*device =	"/dev/spidev0.0";
static uint8_t		mode	=	0;
static uint8_t 		bits	=	8;
//static uint32_t		speed	=	10000000;
static uint32_t		speed	=	5000000;

static uint16_t 	delay	=	0;
static int			fd		=	0;

int init_spi(void)
{
	int ret = 0;

	fd = open(device, O_RDWR);
	if (fd < 0)
		LOGI("can't open device");

	/* Set spi mode W */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		LOGI("can't set spi mode");

	/* Set spi mode R */
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		LOGI("can't get spi mode");

	/* Set bits per word W */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		LOGI("can't set bits per word");

	/* Set bits per word R */
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		LOGI("can't get bits per word");

	/* Set max speed hz W */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		LOGI("can't set max speed hz");

	/* Set max speed hz R */
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		LOGI("can't get max speed hz");

	LOGI("Spi mode: %d\n", mode);
	LOGI("Bits per word: %d\n", bits);
	LOGI("Max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return ret;
}

int set_spi_fd(int spi_fd)
{
	int ret = 0;
    fd = spi_fd;

	return ret;
}
int get_spi_fd(int *spi_fd)
{
    *spi_fd = fd;
	return 0;
}

static void spi_send_2byte(uint8_t addr, uint8_t  value)
{
	int		ret		=  0;
	uint8_t tx[2]	= {0};
	uint8_t rx[2]	= {0};

	tx[0] = addr;
	tx[1] = value;

	struct spi_ioc_transfer tr = 
	{
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
        

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		LOGI("can't send spi message");

    //LOGI("status[%d] send==%#X, %#X\n", ret, tx[0], tx[1]);
}

static void spi_recv_2byte(uint8_t addr, uint8_t* value)
{
	int		ret		=  0;
	uint8_t tx[2]	= {0};
	uint8_t rx[2]	= {0};

	tx[0] = addr;
	tx[1] = 0x00;

	struct spi_ioc_transfer tr = 
	{
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		LOGI("can't send spi message");

	*value = rx[1];

    //LOGI("status[%d] recv=%#X %#X==>%#X %#X\n", ret, tx[0], tx[1], rx[0], rx[1]);
    
}

void recv_from_rc663_byte(uint8_t addr, uint8_t* value)
{
	/* Refer to MFR531.pdf 9.1.4.1 Table8 */
	addr = (uint8_t)((addr << 1) | 0x01);

	spi_recv_2byte(addr, value);
}

void send_to_rc663_byte(uint8_t addr, uint8_t value)
{
	/* Refer to MFR531.pdf 9.1.4.2 Table10 */
	addr = (uint8_t)((addr << 1) & 0xfe);

	spi_send_2byte(addr, value);
}

uint8_t ReadIO(uint8_t address)
{
	uint8_t ret = 0;
	recv_from_rc663_byte(address, &ret);

	return ret;
}

void WriteIO(uint8_t address, uint8_t value)
{
	send_to_rc663_byte(address, value);
	
}

void deinit_spi(void)
{
	close(fd);
}

