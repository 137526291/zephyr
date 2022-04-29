/*
 * Copyright (c) 2017 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) 2019 Marc Reilly
 * Copyright (c) 2019 PHYTEC Messtechnik GmbH
 * Copyright (c) 2020 Endian Technologies AB
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT sitronix_st7789v_fmc

#include "display_st7789v_fmc.h"

#include <device.h>
// #include <drivers/spi.h>
#include <drivers/gpio.h>
#include <pm/device.h>
#include <sys/byteorder.h>
#include <drivers/display.h>

#define LOG_LEVEL CONFIG_DISPLAY_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(display_st7789v_fmc);

// #define ST7789V_CS_PIN		DT_INST_SPI_DEV_CS_GPIOS_PIN(0)
// #define ST7789V_CMD_DATA_PIN	DT_INST_GPIO_PIN(0, cmd_data_gpios)
// #define ST7789V_CMD_DATA_FLAGS	DT_INST_GPIO_FLAGS(0, cmd_data_gpios)
#define ST7789V_RESET_PIN	DT_INST_GPIO_PIN(0, reset_gpios)
#define ST7789V_RESET_FLAGS	DT_INST_GPIO_FLAGS(0, reset_gpios)
#define ST7789V_BK_PIN	DT_INST_GPIO_PIN(0, bk_gpios)
#define ST7789V_BK_FLAGS	DT_INST_GPIO_FLAGS(0, bk_gpios)

static uint8_t st7789v_porch_param[] = DT_INST_PROP(0, porch_param);
static uint8_t st7789v_cmd2en_param[] = DT_INST_PROP(0, cmd2en_param);
static uint8_t st7789v_pwctrl1_param[] = DT_INST_PROP(0, pwctrl1_param);
static uint8_t st7789v_pvgam_param[] = DT_INST_PROP(0, pvgam_param);
static uint8_t st7789v_nvgam_param[] = DT_INST_PROP(0, nvgam_param);
static uint8_t st7789v_ram_param[] = DT_INST_PROP(0, ram_param);
static uint8_t st7789v_rgb_param[] = DT_INST_PROP(0, rgb_param);

#define LCD_RAM   ((uint32_t)0x6001FFFE)     //disp Data ADDR
#define LCD_REG   ((uint32_t)0x60000000)	 //disp Reg  ADDR

static void Write_Data(uint8_t data)
{										    	   
	data = data;
    *(volatile uint8_t *) LCD_RAM = data;		 
}


//写寄存器函数
//regval:寄存器值
static void Write_Command(uint8_t regva)
{ 
	regva = regva;
    *(volatile uint8_t *) LCD_REG = regva;//写入要写的寄存器序号	 
}

struct st7789v_data {
	const struct device *fmc_sram_dev;

#if DT_INST_NODE_HAS_PROP(0, reset_gpios)
	const struct device *reset_gpio;
#endif

#if DT_INST_NODE_HAS_PROP(0, bk_gpios)
	const struct device *bk_gpios;
#endif

	uint16_t height;
	uint16_t width;
	uint16_t x_offset;
	uint16_t y_offset;
};

#ifdef CONFIG_ST7789V_RGB565
#define ST7789V_PIXEL_SIZE 2u
#else
#define ST7789V_PIXEL_SIZE 3u
#endif

static void st7789v_set_lcd_margins(const struct device *dev,
				    uint16_t x_offset, uint16_t y_offset)
{
	struct st7789v_data *data = dev->data;

	data->x_offset = x_offset;
	data->y_offset = y_offset;
}

static void st7789v_transmit(const struct device *dev, uint8_t cmd,
			     uint8_t *tx_data, size_t tx_count)
{
	struct st7789v_data *data = dev->data;

	Write_Command(cmd);

	if (tx_data != NULL) {
		for (int i=0; i<tx_count; i++) {
			Write_Data(tx_data[i]);
		}
	}
}

static void st7789v_exit_sleep(const struct device *dev)
{
	st7789v_transmit(dev, ST7789V_CMD_SLEEP_OUT, NULL, 0);
	k_sleep(K_MSEC(120));
}

static void st7789v_reset_display(const struct device *dev)
{
	LOG_DBG("Resetting display");
#if DT_INST_NODE_HAS_PROP(0, reset_gpios)
	struct st7789v_data *data = dev->data;

	k_sleep(K_MSEC(1));
	gpio_pin_set(data->reset_gpio, ST7789V_RESET_PIN, 1);
	k_sleep(K_MSEC(6));
	gpio_pin_set(data->reset_gpio, ST7789V_RESET_PIN, 0);
	k_sleep(K_MSEC(20));
#else
	st7789v_transmit(dev, ST7789V_CMD_SW_RESET, NULL, 0);
	k_sleep(K_MSEC(5));
#endif
}

static int st7789v_blanking_on(const struct device *dev)
{
	st7789v_transmit(dev, ST7789V_CMD_DISP_OFF, NULL, 0);
	return 0;
}

static int st7789v_blanking_off(const struct device *dev)
{
	st7789v_transmit(dev, ST7789V_CMD_DISP_ON, NULL, 0);
	return 0;
}

static int st7789v_read(const struct device *dev,
			const uint16_t x,
			const uint16_t y,
			const struct display_buffer_descriptor *desc,
			void *buf)
{
	return -ENOTSUP;
}

static void st7789v_set_mem_area(const struct device *dev, const uint16_t x,
				 const uint16_t y, const uint16_t w, const uint16_t h)
{
	struct st7789v_data *data = dev->data;
	uint16_t tx_data[2];

	uint16_t ram_x = x + data->x_offset;
	uint16_t ram_y = y + data->y_offset;

	//MSB first. so wap
	tx_data[0] = sys_cpu_to_be16(ram_x);
	tx_data[1] = sys_cpu_to_be16(ram_x + w - 1);
	st7789v_transmit(dev, ST7789V_CMD_CASET, (uint8_t *)&tx_data[0], 4);

	tx_data[0] = sys_cpu_to_be16(ram_y);
	tx_data[1] = sys_cpu_to_be16(ram_y + h - 1);
	st7789v_transmit(dev, ST7789V_CMD_RASET, (uint8_t *)&tx_data[0], 4);
}

static int st7789v_write(const struct device *dev,
			 const uint16_t x,
			 const uint16_t y,
			 const struct display_buffer_descriptor *desc,
			 const void *buf)
{
	struct st7789v_data *data = dev->data;
	const uint8_t *write_data_start = (uint8_t *) buf;
	uint16_t write_cnt;
	uint16_t nbr_of_writes;
	uint16_t write_h;

	__ASSERT(desc->width <= desc->pitch, "Pitch is smaller then width");
	__ASSERT((desc->pitch * ST7789V_PIXEL_SIZE * desc->height) <= desc->buf_size,
			"Input buffer to small");

	LOG_DBG("Writing %dx%d (w,h) @ %dx%d (x,y)",
		desc->width, desc->height, x, y);
	st7789v_set_mem_area(dev, x, y, desc->width, desc->height);

	if (desc->pitch > desc->width) {
		write_h = 1U;
		nbr_of_writes = desc->height;
	} else {
		write_h = desc->height;
		nbr_of_writes = 1U;
	}

	st7789v_transmit(dev, ST7789V_CMD_RAMWR,
			 (void *) write_data_start,
			 desc->width * ST7789V_PIXEL_SIZE * write_h);

	write_data_start += (desc->pitch * ST7789V_PIXEL_SIZE);
	for (write_cnt = 1U; write_cnt < nbr_of_writes; ++write_cnt) {

		uint8_t *pbuf = (void *)write_data_start;
		uint32_t len = desc->width * ST7789V_PIXEL_SIZE * write_h;
		for (int i=0; i<len; i++) {
			Write_Data(pbuf[i]);
		}
		write_data_start += (desc->pitch * ST7789V_PIXEL_SIZE);
	}

	return 0;
}

static void *st7789v_get_framebuffer(const struct device *dev)
{
	return NULL;
}

static int st7789v_set_brightness(const struct device *dev,
			   const uint8_t brightness)
{
	return -ENOTSUP;
}

static int st7789v_set_contrast(const struct device *dev,
			 const uint8_t contrast)
{
	return -ENOTSUP;
}

static void st7789v_get_capabilities(const struct device *dev,
			      struct display_capabilities *capabilities)
{
	struct st7789v_data *data = dev->data;

	memset(capabilities, 0, sizeof(struct display_capabilities));
	capabilities->x_resolution = data->width;
	capabilities->y_resolution = data->height;

#ifdef CONFIG_ST7789V_RGB565
	capabilities->supported_pixel_formats = PIXEL_FORMAT_RGB_565;
	capabilities->current_pixel_format = PIXEL_FORMAT_RGB_565;
#else
	capabilities->supported_pixel_formats = PIXEL_FORMAT_RGB_888;
	capabilities->current_pixel_format = PIXEL_FORMAT_RGB_888;
#endif
	capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int st7789v_set_pixel_format(const struct device *dev,
			     const enum display_pixel_format pixel_format)
{
#ifdef CONFIG_ST7789V_RGB565
	if (pixel_format == PIXEL_FORMAT_RGB_565) {
#else
	if (pixel_format == PIXEL_FORMAT_RGB_888) {
#endif
		return 0;
	}
	LOG_ERR("Pixel format change not implemented");
	return -ENOTSUP;
}

static int st7789v_set_orientation(const struct device *dev,
			    const enum display_orientation orientation)
{
	if (orientation == DISPLAY_ORIENTATION_NORMAL) {
		return 0;
	}
	LOG_ERR("Changing display orientation not implemented");
	return -ENOTSUP;
}

static void st7789v_lcd_init(const struct device *dev)
{
	struct st7789v_data *data = dev->data;
	uint8_t tmp;

	st7789v_set_lcd_margins(dev, data->x_offset,
				data->y_offset);

	st7789v_transmit(dev, ST7789V_CMD_CMD2EN, st7789v_cmd2en_param,
			 sizeof(st7789v_cmd2en_param));

	st7789v_transmit(dev, ST7789V_CMD_PORCTRL, st7789v_porch_param,
			 sizeof(st7789v_porch_param));

	/* Digital Gamma Enable, default disabled */
	tmp = 0x00;
	st7789v_transmit(dev, ST7789V_CMD_DGMEN, &tmp, 1);

	/* Frame Rate Control in Normal Mode, default value */
	tmp = 0x0f;
	st7789v_transmit(dev, ST7789V_CMD_FRCTRL2, &tmp, 1);

	tmp = DT_INST_PROP(0, gctrl);
	st7789v_transmit(dev, ST7789V_CMD_GCTRL, &tmp, 1);

	tmp = DT_INST_PROP(0, vcom);
	st7789v_transmit(dev, ST7789V_CMD_VCOMS, &tmp, 1);

#if (DT_INST_NODE_HAS_PROP(0, vrhs) && \
	DT_INST_NODE_HAS_PROP(0, vdvs))
	tmp = 0x01;
	st7789v_transmit(dev, ST7789V_CMD_VDVVRHEN, &tmp, 1);

	tmp = DT_INST_PROP(0, vrhs);
	st7789v_transmit(dev, ST7789V_CMD_VRH, &tmp, 1);

	tmp = DT_INST_PROP(0, vdvs);
	st7789v_transmit(dev, ST7789V_CMD_VDS, &tmp, 1);
#endif

	st7789v_transmit(dev, ST7789V_CMD_PWCTRL1, st7789v_pwctrl1_param,
			 sizeof(st7789v_pwctrl1_param));

	/* Memory Data Access Control */
	tmp = DT_INST_PROP(0, mdac);
	st7789v_transmit(dev, ST7789V_CMD_MADCTL, &tmp, 1);

	/* Interface Pixel Format */
	tmp = DT_INST_PROP(0, colmod);
	st7789v_transmit(dev, ST7789V_CMD_COLMOD, &tmp, 1);

	tmp = DT_INST_PROP(0, lcm);
	st7789v_transmit(dev, ST7789V_CMD_LCMCTRL, &tmp, 1);

	tmp = DT_INST_PROP(0, gamma);
	st7789v_transmit(dev, ST7789V_CMD_GAMSET, &tmp, 1);

	st7789v_transmit(dev, ST7789V_CMD_INV_ON, NULL, 0);

	st7789v_transmit(dev, ST7789V_CMD_PVGAMCTRL, st7789v_pvgam_param,
			 sizeof(st7789v_pvgam_param));

	st7789v_transmit(dev, ST7789V_CMD_NVGAMCTRL, st7789v_nvgam_param,
			 sizeof(st7789v_nvgam_param));

	st7789v_transmit(dev, ST7789V_CMD_RAMCTRL, st7789v_ram_param,
			 sizeof(st7789v_ram_param));

	st7789v_transmit(dev, ST7789V_CMD_RGBCTRL, st7789v_rgb_param,
			 sizeof(st7789v_rgb_param));
}

static int st7789v_init(const struct device *dev)
{
	struct st7789v_data *data = dev->data;

	data->fmc_sram_dev = device_get_binding(DT_INST_LABEL(0));
	if (data->fmc_sram_dev == NULL) {
		LOG_ERR("Could not get fmc nor/sram device for LCD");
		// return -EPERM;
	}

#if DT_INST_NODE_HAS_PROP(0, reset_gpios)
	data->reset_gpio = device_get_binding(
			DT_INST_GPIO_LABEL(0, reset_gpios));
	if (data->reset_gpio == NULL) {
		LOG_ERR("Could not get GPIO port for display reset");
		return -EPERM;
	}

	if (gpio_pin_configure(data->reset_gpio, ST7789V_RESET_PIN,
			       GPIO_OUTPUT_INACTIVE | ST7789V_RESET_FLAGS)) {
		LOG_ERR("Couldn't configure reset pin");
		return -EIO;
	}
#endif

#if DT_INST_NODE_HAS_PROP(0, bk_gpios)
	data->bk_gpio = device_get_binding(
			DT_INST_GPIO_LABEL(0, bk_gpio));
	if (data->bk_gpio == NULL) {
		LOG_ERR("Could not get GPIO port for display back light");
		return -EPERM;
	}

	if (gpio_pin_configure(data->bk_gpio, ST7789V_BK_PIN,
			       GPIO_OUTPUT_INACTIVE | ST7789V_BK_FLAGS)) {
		LOG_ERR("Couldn't configure bk pin");
		return -EIO;
	}
#endif

	st7789v_reset_display(dev);

	st7789v_blanking_on(dev);

	// st7789v_lcd_init(dev);

	st7789v_exit_sleep(dev);

	return 0;
}

#ifdef CONFIG_PM_DEVICE
static int st7789v_pm_action(const struct device *dev,
			     enum pm_device_action action)
{
	int ret = 0;

	switch (action) {
	case PM_DEVICE_ACTION_RESUME:
		st7789v_exit_sleep(dev);
		break;
	case PM_DEVICE_ACTION_SUSPEND:
		st7789v_transmit(dev, ST7789V_CMD_SLEEP_IN, NULL, 0);
		break;
	default:
		ret = -ENOTSUP;
		break;
	}

	return ret;
}
#endif /* CONFIG_PM_DEVICE */

static const struct display_driver_api st7789v_api = {
	.blanking_on = st7789v_blanking_on,
	.blanking_off = st7789v_blanking_off,
	.write = st7789v_write,
	.read = st7789v_read,
	.get_framebuffer = st7789v_get_framebuffer,
	.set_brightness = st7789v_set_brightness,
	.set_contrast = st7789v_set_contrast,
	.get_capabilities = st7789v_get_capabilities,
	.set_pixel_format = st7789v_set_pixel_format,
	.set_orientation = st7789v_set_orientation,
};

static struct st7789v_data st7789v_data = {
	.width = DT_INST_PROP(0, width),
	.height = DT_INST_PROP(0, height),
	.x_offset = DT_INST_PROP(0, x_offset),
	.y_offset = DT_INST_PROP(0, y_offset),
};

// PM_DEVICE_DT_INST_DEFINE(0, st7789v_pm_action);

DEVICE_DT_INST_DEFINE(0, &st7789v_init,
	      PM_DEVICE_DT_INST_GET(0), &st7789v_data, NULL, POST_KERNEL,
	      CONFIG_DISPLAY_INIT_PRIORITY, &st7789v_api);