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

uint32_t APP_BEGIN_ADDRESS = 0x00009000; /* m_interrupts */
uint32_t offset = 0;

enum
{
  PHRASE_START = 0,
  PHRASE_TYPE  = 1,
  PHRASE_SIZE  = 2
};

/* Provide info about the flash blocks through an USER config structure */
flash_user_config_t flashUserConfig;

/* Declare a FLASH config struct which initialized by FlashInit, and will be used by all flash operations */
flash_ssd_config_t  flashSSDConfig;

/* Variable used to flag is the memory has been initialized */
uint8_t is_mem_init = 0;

/* Last erased flash sector count */
uint32_t flash_last_erased_sec = 0;

/* Configuration structure flashCfg_0 */
const flash_user_config_t Flash_InitConfig0 =
{
        /* for the S32K144 */
        .PFlashBase  = 0x00000000U,
        .PFlashSize  = 0x00080000U,
        .DFlashBase  = 0x10000000U,
        .EERAMBase   = 0x14000000U,
        .CallBack    = NULL_CALLBACK
};

/* Parse src phrase into boot phrase & verify */
int compile_bootphrase (const char _srecstr[], bootphrase_ptr_t resultbp);
int mem_write          (const bootphrase_ptr_t const);

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
  	  uart_putc (NUL);
          continue;
	}

      if (compile_bootphrase (srecord, &bp) != 0)
        {
	  uart_puts ("compile boot phrase error", MAX_CANON);
          uart_putc (CAN);
          return;
        }

      switch (bp.phrase.type)
        {
        case '1':
        case '2':
        case '3':
          {
            if (mem_write (&bp) != 0)
              {
        	uart_putc (CAN);
                is_eof = 1;
                continue;
              }
            ++phrase_count;
          }
        break;
        case '5':
          {
            uint32_t count = *(uint32_t*)&(bp.phrase.address[0]);
            if (count != phrase_count)
              {
        	uart_puts ("download firmware: checksum error", MAX_CANON);
                uart_putc (CAN);
                is_eof = 1;
                continue;
              }
          }
        case '6':
        break;
        case '7':
        case '8':
        case '9':
          {
            // APP_START_ADDRESS = (uint32_t) bp.phrase.address;
            is_eof = 1; 
          }
        break;
        default:
        break;
        }
      uart_putc (ACK);
    }
  while (!is_eof);
}

void
jump_to_fw ()
{
  /* The volume of memory for bootloader is 32KB */
  if (APP_BEGIN_ADDRESS < 0x00008000 || APP_BEGIN_ADDRESS >= Flash_InitConfig0.PFlashSize)
    {
      return;
    }

  /* Check if Entry address is erased and return if erased */
  if ((*(uint32_t*)APP_BEGIN_ADDRESS) == 0xFFFFFFFF)
    {
      return;
    }

  /* Relocate vector table */
  S32_SCB->VTOR = (uint32_t) APP_BEGIN_ADDRESS;
  /* Set up stack pointer */
  __asm__ __volatile__("msr msp,%0"::"r"(*(uint32_t*)APP_BEGIN_ADDRESS):"memory");
  __asm__ __volatile__("msr psp,%0"::"r"(*(uint32_t*)APP_BEGIN_ADDRESS):"memory");

  ((app_t)*(uint32_t*)(APP_BEGIN_ADDRESS + 4))();
}

int
compile_bootphrase (const char srecord[], bootphrase_ptr_t bp)
{
  if (!bp)
    {
      return -1;
    }

  if (srecord[PHRASE_START] != 'S')
    {
      return -1;
    }

  if (strlen (srecord) <= MIN_SREC)
    {
      return -1;
    }

  bp->phrase.type = srecord[PHRASE_TYPE];
  sscanf(&srecord[PHRASE_SIZE], "%2X", &(bp->phrase.size));

  if (strlen (srecord) < (2*bp->phrase.size + 4)) /* every byte is 2 symbols */
    {                                             /* +4 is SXXX              */
      return -1;
    }

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
  memset(bp->phrase.data, 0, MAX_DATA_BP);
  for (i = 0; i < bp->phrase.data_size; ++i)
    {   
      sscanf (&srecord[PHRASE_SIZE + 2*(1+bp->phrase.address_size) + 2*i], "%2X", &(bp->phrase.data[i]));
      checksum += bp->phrase.data[i];
    }

  /* Get CRC */
  sscanf (&srecord[PHRASE_SIZE + 2*(1+bp->phrase.address_size) + 2*i], "%2X", &(bp->phrase.checksum));

  crc = (uint8_t) 0xFF & ~checksum;

  if (crc != bp->phrase.checksum)
    {
      return -1;
    }

  return 0;
}

int
mem_write (const bootphrase_ptr_t const bp)
{
  uint32_t flash_prog_address;

  if (is_mem_init == 0)
    {
      FlashInit (&Flash_InitConfig0, &flashSSDConfig); /* Initialize memory */
      is_mem_init = 1;
    }

  flash_prog_address = *(uint32_t*)&(bp->phrase.address[0]);

  /* Check overlap with bootloader */
  if (flash_prog_address >= APP_BEGIN_ADDRESS)
    {
      /* Check if block has been erased aligned addresses (first address is assumed to be 0x2000) */
      if ((flash_prog_address % FEATURE_FLS_PF_BLOCK_SECTOR_SIZE) == 0)
        {
          /* Erase sector */
	  switch (FlashEraseSector (&flashSSDConfig, flash_prog_address, FEATURE_FLS_PF_BLOCK_SECTOR_SIZE, FlashCommandSequence))
	    {
	    case FTFx_OK:          uart_puts ("1) Erase sector: Operation was successful",                MAX_CANON);
	      break;
	    case FTFx_ERR_ACCERR:  uart_puts ("1) Erase sector: Operation failed due to an access error", MAX_CANON);
	      return -1;
	    case FTFx_ERR_PVIOL:   uart_puts ("1) Erase sector: Operation failed due to a protection violation", MAX_CANON);
	      return -1;
	    case FTFx_ERR_MGSTAT0: uart_puts ("1) Erase sector: Operation failed due to an error was detected during execution of an FTFx command", MAX_CANON);
	      return -1;
	    case FTFx_ERR_SIZE:    uart_puts ("1) Erase sector: Operation failed due to misaligned size", MAX_CANON);
	      return -1;
	    default:               uart_puts ("1) Erase sector: Unknown error", MAX_CANON);
	      return -1;
	    }

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
          switch (FlashEraseSector (&flashSSDConfig, flash_prog_address, FEATURE_FLS_PF_BLOCK_SECTOR_SIZE, FlashCommandSequence))
            {
	    case FTFx_OK:          uart_puts ("2) Erase sector: Operation was successful",                MAX_CANON);
	      break;
	    case FTFx_ERR_ACCERR:  uart_puts ("2) Erase sector: Operation failed due to an access error", MAX_CANON);
	      return -1;
	    case FTFx_ERR_PVIOL:   uart_puts ("2) Erase sector: Operation failed due to a protection violation", MAX_CANON);
	      return -1;
	    case FTFx_ERR_MGSTAT0: uart_puts ("2) Erase sector: Operation failed due to an error was detected during execution of an FTFx command", MAX_CANON);
	      return -1;
	    case FTFx_ERR_SIZE:    uart_puts ("2) Erase sector: Operation failed due to misaligned size", MAX_CANON);
	      return -1;
	    default:               uart_puts ("2) Erase sector: Unknown error", MAX_CANON);
	      return -1;
            }

          /* Store the address of the erased sector */
          flash_last_erased_sec = tmp_add;
        }

      /* Program phrase */
      switch (FlashProgram (&flashSSDConfig, flash_prog_address, bp->phrase.data_size,  bp->phrase.data, FlashCommandSequence))
        {
	case FTFx_OK:          uart_puts ("Flash program: Operation was successful\r\n", MAX_CANON); break;
	case FTFx_ERR_ACCERR:  uart_puts ("Flash program: Operation failed due to an access error\r\n", MAX_CANON);
	  return -1;
	case FTFx_ERR_SIZE:    uart_puts ("Flash program: Operation failed due to misaligned size\r\n", MAX_CANON);
	  return -1;
	default:               uart_puts ("Flash program: Unknown error", MAX_CANON);
	  return -1;
        }

      return 0;
    }
  return -1;
}

