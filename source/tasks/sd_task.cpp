#include "tasks/sd_task.h"
#include "SDBlockDevice.h"
#include "BlockDevice.h"
#include "stm32746g_discovery_sd.h"
#include <stdio.h>
#include <errno.h>
#include <functional>

#define BLOCK_START_ADDR 0 /* Block start address      */
#define NUM_OF_BLOCKS 5    /* Total number of blocks   */
#define BUFFER_WORDS_SIZE                                                      \
  ((BLOCKSIZE * NUM_OF_BLOCKS) >> 2) /* Total data size in bytes */

uint32_t aTxBuffer[BUFFER_WORDS_SIZE];
uint32_t aRxBuffer[BUFFER_WORDS_SIZE];

// prototypes
static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght,
                        uint32_t uwOffset);
static uint8_t Buffercmp(uint32_t *pBuffer1, uint32_t *pBuffer2,
                         uint16_t BufferLength);

void Tasks::SD::begin() {
  uint8_t SD_state = MSD_OK;
  static uint8_t prev_status = SD_PRESENT;

  SD_state = BSP_SD_Init();
  if (SD_state != MSD_OK) {
    if (SD_state == MSD_ERROR_SD_NOT_PRESENT) {
      printf("SD Card is not present - exiting");
      wait_ns(1000 * 5);
      exit(1);
    } else {
      printf("SD Initialization : Failed.\n");
      wait_ns(1000 * 5);
      exit(1);
    }
  }

  SD_state =
      BSP_SD_Erase(BLOCK_START_ADDR, (BLOCK_START_ADDR + NUM_OF_BLOCKS - 1));

  /* Wait until SD card is ready to use for new operation */
  while (BSP_SD_GetCardState() != SD_TRANSFER_OK) {
  }
  if (SD_state != MSD_OK) {
    printf("SD ERASE : FAILED.\n");
    printf("SD Test Aborted.\n");
  } else {
    printf("SD ERASE : OK.\n");

    /* Fill the buffer to write */
    Fill_Buffer(aTxBuffer, BUFFER_WORDS_SIZE, 0x2300);

    SD_state =
        BSP_SD_WriteBlocks(aTxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS, 10000);
    /* Wait until SD card is ready to use for new operation */
    while (BSP_SD_GetCardState() != SD_TRANSFER_OK) {
    }

    if (SD_state != MSD_OK) {
      printf("SD WRITE : FAILED.\n");
      printf("SD Test Aborted.\n");
    } else {
      printf("SD WRITE : OK.\n");

      SD_state =
          BSP_SD_ReadBlocks(aRxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS, 10000);
      /* Wait until SD card is ready to use for new operation */
      while (BSP_SD_GetCardState() != SD_TRANSFER_OK) {
      }

      if (SD_state != MSD_OK) {
        printf("SD READ : FAILED.\n");
        printf("SD Test Aborted.\n");
      } else {
        printf("SD READ : OK.\n");
        if (Buffercmp(aTxBuffer, aRxBuffer, BUFFER_WORDS_SIZE) > 0) {
          printf("SD COMPARE : FAILED.\n");
          printf("SD Test Aborted.\n");
        } else {
          printf("SD Test : OK.\n");
          printf("SD can be removed.\n");
        }
      }
    }
  }

  while (true) {
    if (BSP_SD_IsDetected() != SD_PRESENT) {
      if (prev_status != SD_NOT_PRESENT) {
        BSP_SD_Init();
        prev_status = SD_NOT_PRESENT;
        printf("SD removed\n");
      }
    } else if (prev_status != SD_PRESENT) {
      printf("SD detected\n");
      prev_status = SD_PRESENT;
    }
  }
}

/**
 * @brief  Fills buffer with user predefined data.
 * @param  pBuffer: pointer on the buffer to fill
 * @param  uwBufferLenght: size of the buffer to fill
 * @param  uwOffset: first value to fill on the buffer
 * @retval None
 */
static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLength,
                        uint32_t uwOffset) {
  uint32_t tmpIndex = 0;

  /* Put in global buffer different values */
  for (tmpIndex = 0; tmpIndex < uwBufferLength; tmpIndex++) {
    pBuffer[tmpIndex] = tmpIndex + uwOffset;
  }
}

/**
 * @brief  Compares two buffers.
 * @param  pBuffer1, pBuffer2: buffers to be compared.
 * @param  BufferLength: buffer's length
 * @retval 1: pBuffer identical to pBuffer1
 *         0: pBuffer differs from pBuffer1
 */
static uint8_t Buffercmp(uint32_t *pBuffer1, uint32_t *pBuffer2,
                         uint16_t BufferLength) {
  while (BufferLength--) {
    if (*pBuffer1 != *pBuffer2) {
      return 1;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}
