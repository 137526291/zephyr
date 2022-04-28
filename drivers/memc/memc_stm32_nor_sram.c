/*
 * Copyright (c) 2020 Teslabs Engineering S.L.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_fmc_nor_sram

#include <device.h>
#include <soc.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(memc_stm32_nor_sram, CONFIG_MEMC_LOG_LEVEL);

/** SRAM controller register offset. */
#define NORSRAM_EXTENDED_OFFSET 0x0104UL

/** FMC SDRAM controller bank configuration fields. */
struct memc_stm32_nor_sram_bank_config {
	FMC_NORSRAM_InitTypeDef init;
	FMC_NORSRAM_TimingTypeDef timing;
};

/** FMC NORSRAM controller configuration fields. */
struct memc_stm32_nor_sram_config {
	FMC_NORSRAM_TypeDef *sram;
	uint32_t power_up_delay;

	uint32_t NSBank;
	uint32_t DataAddressMux;
	uint32_t MemoryType;
	uint32_t MemoryDataWidth;
	uint32_t BurstAccessMode;
	uint32_t WaitSignalPolarity;
	uint32_t WaitSignalActive;
	uint32_t WriteOperation;
	uint32_t WaitSignal;
	uint32_t ExtendedMode;
	uint32_t AsynchronousWait;
	uint32_t WriteBurst;
	uint32_t ContinuousClock;
	uint32_t WriteFifo;
	uint32_t PageSize;

	const struct memc_stm32_nor_sram_bank_config *banks;
	size_t banks_len;
};

static int memc_stm32_nor_sram_init(const struct device *dev)
{
	const struct memc_stm32_nor_sram_config *config = dev->config;

	SRAM_HandleTypeDef sram = { 0 };
	
	sram.Instance = config->sram;
	sram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
	
	for (size_t i = 0U; i < config->banks_len; i++) {
		sram.State = HAL_SRAM_STATE_RESET;
		memcpy(&sram.Init, &config->banks[i].init, sizeof(sram.Init));

		HAL_SRAM_Init(&sram,
			(FMC_NORSRAM_TimingTypeDef *)&config->banks[i].timing,
			NULL);
	}

	k_usleep(config->power_up_delay);
	return 0;
}

/** SDRAM bank/s configuration initialization macro. */
#define BANK_CONFIG(node_id)                                                   \
	{ .init = {                                                            \
	    .NSBank = DT_REG_ADDR(node_id),                                    \
	    .DataAddressMux = DT_PROP_BY_IDX(node_id, st_sram_control, 0),  \
	    .MemoryType = DT_PROP_BY_IDX(node_id, st_sram_control, 1),     \
	    .MemoryDataWidth = DT_PROP_BY_IDX(node_id, st_sram_control, 2),   \
	    .BurstAccessMode = DT_PROP_BY_IDX(node_id, st_sram_control, 3),\
	    .WaitSignalPolarity = DT_PROP_BY_IDX(node_id, st_sram_control, 4),	\
	    .WaitSignalActive = FMC_SDRAM_WRITE_PROTECTION_DISABLE,	\
	    .WriteOperation = DT_PROP_BY_IDX(node_id, st_sram_control, 5),	\
	    .WaitSignal = DT_PROP_BY_IDX(node_id, st_sram_control, 6),	\
	    .ExtendedMode = DT_PROP_BY_IDX(node_id, st_sram_control, 7),    \
	  	.AsynchronousWait = DT_PROP_BY_IDX(node_id, st_sram_control, 8),\
		.WriteBurst = DT_PROP_BY_IDX(node_id, st_sram_control, 9),		\
		.ContinuousClock = DT_PROP_BY_IDX(node_id, st_sram_control, 10),\
		.WriteFifo = DT_PROP_BY_IDX(node_id, st_sram_control, 11),		\
		.PageSize = DT_PROP_BY_IDX(node_id, st_sram_control, 12),		\
	  },                                                                   \
	  .timing = {                                                          \
	    .AddressSetupTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 0),  \
	    .AddressHoldTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 1), \
	    .DataSetupTime = DT_PROP_BY_IDX(node_id, st_sram_timing, 2),    \
	    .BusTurnAroundDuration = DT_PROP_BY_IDX(node_id, st_sram_timing, 3),	\
	    .CLKDivision = DT_PROP_BY_IDX(node_id, st_sram_timing, 4),  \
	    .DataLatency = DT_PROP_BY_IDX(node_id, st_sram_timing, 5),	\
	    .AccessMode = DT_PROP_BY_IDX(node_id, st_sram_timing, 6),	\
	  }	\
	},

/** SDRAM bank/s configuration. */
static const struct memc_stm32_nor_sram_bank_config bank_config[] = {
	DT_INST_FOREACH_CHILD(0, BANK_CONFIG)
};

/** SDRAM configuration. */
static const struct memc_stm32_nor_sram_config config = {
	.sram = (FMC_NORSRAM_TypeDef *)(DT_REG_ADDR(DT_INST_PARENT(0))),
	.power_up_delay = DT_INST_PROP(0, power_up_delay),

	.banks = bank_config,
	.banks_len = ARRAY_SIZE(bank_config),
};

DEVICE_DT_INST_DEFINE(0, memc_stm32_nor_sram_init, NULL,
	      NULL, &config, POST_KERNEL, CONFIG_MEMC_INIT_PRIORITY, NULL);
