/*
 * fw.c
 *
 *  Created on: Apr 7, 2021
 *      Author: somebyte
 */

#include "S32K144.h"
#include "../fw.h"
#include "../srec.h"
#include "../../tty/tty.h"
#include "../../tty/uart.h"
#include "../../drv/fsl_flash_driver_c90tfs.h"

#include <string.h>
#include <stdio.h>

typedef void(*app_t)(void);

uint32_t APP_BEGIN_ADDRESS = 0x0000; /* m_interrupts */

enum
{
  PHRASE_START = 0,
  PHRASE_TYPE  = 1,
  PHRASE_SIZE  = 2
};

/* Provide info about the flash blocks through an USER config structure */
flash_user_config_t flashUserConfig;

/* Declare a FLASH config struct which initialized by FlashInit, and will be used by all flash operations */
flash_ssd_config_t flashSSDConfig;

/* Variable used to flag is the memory has been initialized */
uint8_t is_mem_init = 0;

/* Last erased flash sector count */
uint32_t flash_last_erased_sec = 0;

/* Configuration structure flashCfg_0 */
const flash_user_config_t Flash_InitConfig0 =
{
        /* for the S32K144 */
        .PFlashBase  = 0x00000000U,
        .PFlashSize  = 0x00100000U,
        .DFlashBase  = 0x10000000U,
        .EERAMBase   = 0x14000000U,
        .CallBack    = NULL_CALLBACK
};

/* Parse src phrase into boot phrase & verify */
uint8_t
compile_bootphrase (const char _srecstr[], bootphrase_ptr_t resultbp);

void
mem_write (const bootphrase_ptr_t const);

/* Download firmware */
void
download_fw ()
{
  bootphrase_t bp;
  uint8_t    error  = 0;
  uint8_t    is_eof = 0;
  uint32_t   phrase_count = 0;
  char       srecord[MAX_SREC];

  do
    {
      if (uart_gets (srecord, MAX_SREC) == 0)
        {
  	      uart_putc (EOT);
          continue;
	    }

      error = compile_bootphrase (srecord, &bp);
      if (error != ERR_OK)
        {
          uart_putc (error);
          return;
        }

      switch (bp.phrase.type)
        {
        case '1':
        case '2':
        case '3':
          mem_write (&bp);
          ++phrase_count;
          uart_putc (ERR_OK);
          break;
        case '5':
        case '6':
          {
            uint32_t count = (uint32_t) bp.phrase.address;
            if (count != phrase_count)
            {
              uart_putc (ERR_CRC);
              return;
            }
          } break;
        case '7':
        case '8':
        case '9':
          {
            // APP_START_ADDRESS = (uint32_t) bp.phrase.address;
            is_eof = 1; 
            uart_putc (ERR_OK);
          } break;
        default: break;
        }
    }
  while (!is_eof);
}

void
jump_to_fw ()
{
  /* Check if Entry address is erased and return if erased */
  if (APP_BEGIN_ADDRESS < 0x1000)
      return;

  if ((*(uint32_t*)APP_BEGIN_ADDRESS) == 0xFFFFFFFF)
      return;

  /* Set up stack pointer */
  S32_SCB->VTOR = APP_BEGIN_ADDRESS;
  __asm__ __volatile__("msr msp,%0"::"r"(*(uint32_t*)APP_BEGIN_ADDRESS):"memory");
  __asm__ __volatile__("msr psp,%0"::"r"(*(uint32_t*)APP_BEGIN_ADDRESS):"memory");

  ((app_t)*(uint32_t*)(APP_BEGIN_ADDRESS + 4))();
}

uint8_t
compile_bootphrase (const char srecord[], bootphrase_ptr_t bp)
{
  if (!bp)
    return ERR_CRC;

  if (srecord[PHRASE_START] != 'S')
    return ERR_CRC;

  if (strlen (srecord) <= MIN_SREC)
    return ERR_CRC;

  bp->phrase.type = srecord[PHRASE_TYPE];
  sscanf(&srecord[PHRASE_SIZE], "%2X", &(bp->phrase.size));

  if (strlen (srecord) < (2*bp->phrase.size + 4)) /* every byte is 2 symbols */
    return ERR_CRC;                               /* +4 is SXXX              */

  uint32_t checksum = bp->phrase.size;
  uint8_t  crc      = 0;
  uint8_t  i        = 0;

  /* Get address size */
  switch (bp->phrase.type)
    {   
    // 24-bit address; 3 bytes; 6 symbols
    case '2':
    case '6':
    case '8': bp->phrase.address_size = 3; break;
    // 32-bit address; 4 bytes; 8 symbols
    case '3':
    case '7': bp->phrase.address_size = 4; break;
    // 16-bit address; 2 bytes; 4 symbols
    default:  bp->phrase.address_size = 2; break;
    }

  /* Get address */
  for (i = 0; i < bp->phrase.address_size; ++i)
    {  
      sscanf (&srecord[PHRASE_SIZE + 2*(bp->phrase.address_size-i)], "%2X", &(bp->phrase.address[i]));
      checksum += bp->phrase.address[i];
    }

  bp->phrase.data_size = bp->phrase.size - bp->phrase.address_size - 1; /* -n ADDRESS; -1 CRC */

  /* Get data */
  for (i = 0; i < bp->phrase.data_size; ++i)
    {   
      sscanf (&srecord[PHRASE_SIZE + 2*(1+bp->phrase.address_size) + 2*i], "%2X", &(bp->phrase.data[i]));
      checksum += bp->phrase.data[i];
    }

  /* Get CRC */
  sscanf (&srecord[PHRASE_SIZE + 2*(1+bp->phrase.address_size) + 2*i], "%2X", &(bp->phrase.checksum));

  crc = (uint8_t) 0xFF & ~checksum;

  if (crc != bp->phrase.checksum)
    return ERR_CRC;

  return ERR_OK;
}

void mem_write (const bootphrase_ptr_t const bp)
{
  uint32_t flash_prog_address;

  if (is_mem_init == 0)
    {
      FlashInit (&Flash_InitConfig0, &flashSSDConfig); /* Initialize memory */
      is_mem_init = 1;
    }

  flash_prog_address = (*(uint32_t*)bp->phrase.address);

  /* Check overlap with bootloader */
  if (flash_prog_address >= APP_BEGIN_ADDRESS)
    {
      /* Check if block has been erased aligned addresses (first address is assumed to be 0x2000) */
      if ((flash_prog_address % FEATURE_FLS_PF_BLOCK_SECTOR_SIZE) == 0)
        {
          /* Erase sector */
          FlashEraseSector (&flashSSDConfig, flash_prog_address, FEATURE_FLS_DF_BLOCK_SECTOR_SIZE, FlashCommandSequence);

          /* Store the address of the erased sector */
          flash_last_erased_sec = flash_prog_address;
        }
      else
      /* Check that the address from the SREC is within the previously erased sector (check misaligned addresses) */
      if ((flash_last_erased_sec + FEATURE_FLS_PF_BLOCK_SECTOR_SIZE) < flash_prog_address)
        {
          uint32_t tmp_add;

          /* Obtain address of sector to be erased */
          tmp_add = flash_prog_address % FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
          tmp_add = flash_prog_address - tmp_add;

          /* Erase sector */
          FlashEraseSector(&flashSSDConfig, tmp_add, FEATURE_FLS_DF_BLOCK_SECTOR_SIZE, FlashCommandSequence);

          /* Store the address of the erased sector */
          flash_last_erased_sec = tmp_add;
        }

      /* Program phrase */
      FlashProgram (&flashSSDConfig, flash_prog_address, bp->phrase.data_size,  bp->phrase.data, FlashCommandSequence);
    }
}

