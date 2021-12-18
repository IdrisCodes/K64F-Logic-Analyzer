/*
 * sump.c
 *
 *  Created on: Dec 11, 2021
 *      Author: idris
 */

#include "sump.h"
#include <string.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"

/* Global variables */
volatile uint8_t sump_buffer[32000];

/* SUMP Configuration */
static uint16_t delayCount = 0U;
static uint16_t readCount = 32000U;
static SUMP_Flags_t flags;
static uint8_t trigger_values = 0U;
static uint8_t trigger_mask = 0U;
static uint32_t sump_divider = 0U;

/* Prototypes */
static void SUMP_SendString (const uint8_t *str);
static void SUMP_Meta_SendString (SUMP_KEY_t key, const uint8_t *str);
static void SUMP_Meta_SendUint32 (SUMP_KEY_t key, uint32_t value);
static void SUMP_Meta_SendMetadata (void);
static void SUMP_SendSamples_Blocking (void);
static SUMP_CMD_t SUMP_GetCmd (void);
static uint32_t SUMP_GetParams (void);
static void SUMP_AcquireSamples (void);

/**
 * Poll for commands.
 */
void SUMP_MainLoop (void)
{
  SUMP_CMD_t cmd = SUMP_CMD_RESET;
  uint32_t params = 0U;

  PIT_StartTimer (PIT, PIT_CHANNEL_0);
  while (1)
    {
      cmd = SUMP_GetCmd ();

      switch (cmd)
	{
	case SUMP_CMD_RESET:
	  EDMA_AbortTransfer (&DMA_CH0_Handle);
	  break;
	case SUMP_CMD_RUN:
	  SUMP_AcquireSamples ();
	  break;
	case SUMP_CMD_ID:
	  SUMP_SendString (DEVICE_ID);
	  break;
	case SUMP_CMD_GET_METADATA:
	  SUMP_Meta_SendMetadata ();
	  break;
	case SUMP_CMD_SET_READ_DELAY_COUNT:
	  params = SUMP_GetParams ();
	  readCount = (uint16_t) ((params >> 24U) & 0x000000FFU);
	  readCount |= (uint16_t) ((params >> 8U) & 0x0000FF00U);
	  readCount = (readCount + 1U) * 4U;

	  delayCount = (uint16_t) ((params >> 8U) & 0x000000FFU);
	  delayCount |= (uint16_t) ((params << 8U) & 0x0000FF00U);
	  delayCount = (delayCount + 1U) * 4U;

	  if (readCount > BUFFER_SIZE)
	    {
	      readCount = BUFFER_SIZE;
	      LED_RED_ON();
	    }
	  break;
	case SUMP_CMD_SET_DIVIDER:
	  params = SUMP_GetParams ();

	  sump_divider = (uint16_t) ((params >> 24U) & 0x000000FFU);
	  sump_divider |= (uint16_t) ((params >> 8U) & 0x0000FF00U);
	  sump_divider |= (uint16_t) ((params << 16U) & 0x00FF0000U);
	  sump_divider = (sump_divider + 1) / 10;
	  if (sump_divider == 0U)
	    {
	      sump_divider = 1U;
	    }
	  PIT_SetTimerPeriod (PIT, PIT_CHANNEL_0, PIT_0_TICKS * sump_divider);
	  break;
	case SUMP_CMD_SET_FLAGS:
	  params = SUMP_GetParams ();
	  flags.inverted = (params >> 31U) & 0x01U;
	  flags.external = (params >> 30U) & 0x01U;
	  flags.channel_group3 = (params >> 29U) & 0x01U;
	  flags.channel_group2 = (params >> 28U) & 0x01U;
	  flags.channel_group1 = (params >> 27U) & 0x01U;
	  flags.channel_group0 = (params >> 26U) & 0x01U;
	  flags.filter = (params >> 25U) & 0x01U;
	  flags.demux = (params >> 24U) & 0x01U;
	  break;
	case SUMP_CMD_SET_TRIGGER_CONFIG_0:
	  params = SUMP_GetParams ();
	  break;
	case SUMP_CMD_SET_TRIGGER_MASK_0:
	  params = SUMP_GetParams ();
	  trigger_mask = (params >> 24U) & 0x0F;
	  break;
	case SUMP_CMD_SET_TRIGGER_VALUE_0:
	  params = SUMP_GetParams ();
	  trigger_values = (params >> 24U) & 0x0F;
	  break;
	default:
	  /**/
	  break;
	}
    }
}

/**
 * Wait for trigger then start sampling.
 */
void SUMP_AcquireSamples (void)
{
  EDMA_PrepareTransferConfig (&DMA_CH0_TRANSFER0_config,
  				      (void*) &(GPIOD->PDIR), 1U, 0,
  				      (void*) sump_buffer, 1U, 1, 1U,
  				      readCount);
  EDMA_SubmitTransfer (&DMA_CH0_Handle, &DMA_CH0_TRANSFER0_config);

  while ((GPIOD->PDIR & trigger_mask) != (trigger_values & trigger_mask))
    {
      ;
    }
  EDMA_StartTransfer (&DMA_CH0_Handle);
}

/**
 * Send a string to the client. The null character is not
 * transmitted here.
 * \param str[in] String to be sent to client
 */
void SUMP_SendString (const uint8_t *str)
{
  assert(str != NULL);

  UART_WriteBlocking (UART0, str, strlen ((const char*) str));
}

/**
 * Send the samples to the client. This is a blocking call.
 */
void SUMP_SendSamples_Blocking (void)
{
  if (!flags.demux)
    {
      UART_WriteBlocking (UART0, (uint8_t*)sump_buffer, readCount);
    }
  else
    {
      for (size_t i = 0U; i < readCount; ++i)
	{
	  UART_WriteBlocking (UART0, (uint8_t*)&sump_buffer[i], 1);
	  UART_WriteBlocking (UART0, (uint8_t*)&sump_buffer[i], 1);
	}
    }
}

/**
 * Send all the available metadata to the client.
 */
void SUMP_Meta_SendMetadata (void)
{
  SUMP_Meta_SendString (SUMP_KEY_DEVICE_NAME, DEVICE_NAME);
  SUMP_Meta_SendString (SUMP_KEY_DEVICE_FW_VERSION, DEVICE_FW_VERSION);
  SUMP_Meta_SendString (SUMP_KEY_ANCILLARY_VERSION, DEVICE_ANCILLARY);

  SUMP_Meta_SendUint32 (SUMP_KEY_NUM_USABLE_PROBES, DEVICE_NOF_PROBES);
  SUMP_Meta_SendUint32 (SUMP_KEY_SAMPLE_MEMORY_BYTES, BUFFER_SIZE);
  SUMP_Meta_SendUint32 (SUMP_KEY_MAX_SAMPLE_RATE_HZ, DEVICE_MAX_SAMPLERATE);
  SUMP_Meta_SendUint32 (SUMP_KEY_PROTOCOL_VERSION, DEVICE_PROTOCOL_VERSION);
}

/**
 * Sends metadata of type string to the client.
 * \param key[in] The metadata key
 * \param str[in] The metadata value
 */
void SUMP_Meta_SendString (SUMP_KEY_t key, const uint8_t *str)
{
  uint8_t temp;
  size_t length;
  assert(str != NULL);
  assert(key != SUMP_KEY_END_OF_METADATA);

  length = strlen ((const char*) str);

  /* Send key */
  temp = (uint8_t) key;
  UART_WriteBlocking (UART0, &temp, 1);

  /* Send string */
  UART_WriteBlocking (UART0, str, length);

  /* Send EOD */
  temp = SUMP_KEY_END_OF_METADATA;
  UART_WriteBlocking (UART0, &temp, 1);
}

/**
 * Send metadata of type 32 bit integer to the client
 * \param key[in] The metadata key
 * \param value[in] The 32 integer metadata value
 */
void SUMP_Meta_SendUint32 (SUMP_KEY_t key, uint32_t value)
{
  uint8_t bytes[4];
  bytes[0] = (uint8_t) (value >> 24U);
  bytes[1] = (uint8_t) (value >> 16U);
  bytes[2] = (uint8_t) (value >> 8U);
  bytes[3] = (uint8_t) (value);
  /* Send key */
  UART_WriteBlocking (UART0, &key, 1);
  UART_WriteBlocking (UART0, bytes, 4);
}

/**
 * Poll for the client's commands.
 * \return cmd
 */
SUMP_CMD_t SUMP_GetCmd (void)
{
  uint8_t cmd = 0x00;
  UART_ReadBlocking (UART0, &cmd, 1);
  return cmd;
}

/**
 * Obtain command parameters if it has any.
 * \return params
 */
uint32_t SUMP_GetParams (void)
{
  uint32_t params = 0U;
  uint8_t bytes[4U];
  UART_ReadBlocking (UART0, bytes, 4);
  params = bytes[0] << 24U;
  params |= bytes[1] << 16U;
  params |= bytes[2] << 8U;
  params |= bytes[3];
  return params;
}


/**
 * DMA major loop complete callback.
 * \param edma_handle
 * \param userData
 * \param transferDone
 * \param tcds
 */
void DMA_User_Callback (edma_handle_t *edma_handle, void *userData,
			bool transferDone,
			uint32_t tcds)
{
  SUMP_SendSamples_Blocking ();
}

