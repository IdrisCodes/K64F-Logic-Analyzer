/*
 * sump.h
 *
 *  Created on: Dec 11, 2021
 *      Author: idris
 *      Based on the work of Erich Styger and Reiner Geiger
 */

#ifndef SUMP_H_
#define SUMP_H_

/* Device properties*/

/** Extended SUMP protocol metadata **/
/* Null-terminated string metadata */
#define DEVICE_ID ((const uint8_t*)"1ALS")
#define DEVICE_NAME             ((const uint8_t*)"FRDM-K64F LogicAnalyzer")
#define DEVICE_FW_VERSION       ((const uint8_t*)"V3.8")
#define DEVICE_ANCILLARY        ((const uint8_t*)"V2.0")

/* 32 bit unsigned integer metadata */
#define DEVICE_MAX_SAMPLERATE (10000000U)
#define BUFFER_SIZE (32000U)

/* 8 bit unsigned integer metadata */
#define DEVICE_NOF_PROBES       (4U) /* number of probes */
#define DEVICE_PROTOCOL_VERSION (2U) /* always 2! */

/* Metadata keys */
typedef enum
{
	SUMP_KEY_DEVICE_NAME = 0x01U,
	SUMP_KEY_DEVICE_FW_VERSION = 0x02U,
	SUMP_KEY_ANCILLARY_VERSION = 0x03U,
	SUMP_KEY_NUM_USABLE_PROBES = 0x20U,
	SUMP_KEY_SAMPLE_MEMORY_BYTES = 0x21U,
	SUMP_KEY_SAMPLE_DYNAMIC_MEMORY_BYTES = 0x22U,
	SUMP_KEY_MAX_SAMPLE_RATE_HZ = 0x23U,
	SUMP_KEY_PROTOCOL_VERSION = 0x24U,
	SUMP_KEY_NUM_USABLE_PROBES_SHORT = 0x40U,
	SUMP_KEY_PROTOCOL_VERSION_SHORT = 0x41U,
	SUMP_KEY_END_OF_METADATA = 0x00U
} SUMP_KEY_t;

/* SUMP Commands */
typedef enum
{
	SUMP_CMD_RESET = 0x00,
	SUMP_CMD_RUN = 0x01,
	SUMP_CMD_ID = 0x02,

	SUMP_CMD_SELFTEST = 0x03,
	SUMP_CMD_GET_METADATA = 0x04,
	SUMP_CMD_RLE = 0x05,
	SUMP_CMD_RUN_ADVANCED_TRIGGER = 0x0F,
	SUMP_CMD_SET_DIVIDER = 0x80,
	SUMP_CMD_SET_READ_DELAY_COUNT = 0x81,
	SUMP_CMD_SET_FLAGS = 0x82,
	SUMP_CMD_FLAGS_TEST_DATA = (1 << 10),
	SUMP_CMD_WRITE_TRIGGER_SELECT = 0x9E,
	SUMP_CMD_WRITE_TRIGGER_DATA = 0x9F,

	SUMP_CMD_SET_TRIGGER_MASK_0 = 0xC0,
	SUMP_CMD_SET_TRIGGER_VALUE_0 = 0xC1,
	SUMP_CMD_SET_TRIGGER_CONFIG_0 = 0xC2,

	SUMP_CMD_SET_TRIGGER_MASK_1 = 0xC4,
	SUMP_CMD_SET_TRIGGER_VALUE_1 = 0xC5,
	SUMP_CMD_SET_TRIGGER_CONFIG_1 = 0xC6,

	SUMP_CMD_SET_TRIGGER_MASK_2 = 0xC8,
	SUMP_CMD_SET_TRIGGER_VALUE_2 = 0xC9,
	SUMP_CMD_SET_TRIGGER_CONFIG_2 = 0xCA,

	SUMP_CMD_SET_TRIGGER_MASK_3 = 0xCC,
	SUMP_CMD_SET_TRIGGER_VALUE_3 = 0xCD,
	SUMP_CMD_SET_TRIGGER_CONFIG_3 = 0xCE
} SUMP_CMD_t;

/* SUMP flags*/
typedef struct
{
	unsigned demux :1;
	unsigned filter :1;
	unsigned channel_group3 :1;
	unsigned channel_group2 :1;
	unsigned channel_group1 :1;
	unsigned channel_group0 :1;
	unsigned external :1;
	unsigned inverted :1;

} SUMP_Flags_t;

/* Exported functions */
void SUMP_MainLoop(void);

#endif /*SUMP_H_ */
