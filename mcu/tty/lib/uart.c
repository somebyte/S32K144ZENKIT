#include "S32K144.h"
#include "../uart.h"

#include <string.h>
#include <stdio.h>

LPUART_Type* LPUART_PTR   = NULL;
uint32_t     LPUART_INDEX = 0;

void
uart_init (uint32_t settings)
{
  switch (settings & UART_IFCMASK)
    {
    case 1:
      LPUART_INDEX = PCC_LPUART0_INDEX;
      LPUART_PTR   = LPUART0;
    break;
    case 2:
      LPUART_INDEX = PCC_LPUART1_INDEX;
      LPUART_PTR   = LPUART1;
    break;
    case 4:
    default:
      LPUART_INDEX = PCC_LPUART2_INDEX;
      LPUART_PTR   = LPUART2;
    break;
    }

  PORT_Type* PORT_PTR = NULL;
  uint32_t pin = 0;
  uint32_t alt = 0;

  switch (settings & (UART_PIN_RXMASK|UART_IFCMASK))
    {
    /* PORTA */
    case 0x00000011: // rx = 2
      pin = 2;
      alt = 6;
    case 0x00000014: // rx = 8
      pin = pin?pin:8;
      alt = alt?alt:2;
      PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock */
      PORT_PTR = PORTA; 
    break;
    /* PORTB */
    case 0x00000021: // rx = 0
      pin = 0;
      alt = 2;
      PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTB; 
    break;
    /* PORTC */
    case 0x00000041: // rx = 2
      pin = 2;
      alt = 4;
    case 0x00000012: // rx = 6
      pin = pin?pin:6;
      alt = alt?alt:2;
    case 0x00000022: // rx = 8
      pin = pin?pin:8;
      alt = alt?alt:2;
      PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTC; 
    break;
    /* PORTD */ 
    case 0x00000042: // rx = 13
      pin = 13;
      alt = 3;
    case 0x00000024: // rx = 6
      pin = pin?pin:6;
      alt = alt?alt:2;
    case 0x00000044: // rx = 17
      pin = pin?pin:17;
      alt = alt?alt:3;
      PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTD; 
    break;
    default:
      pin = 6;
      alt = 2;
      PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTD; 
    break;
    }

  PORT_PTR->PCR[pin] |= PORT_PCR_MUX(alt); /* RX PIN MUX */

  PORT_PTR = NULL;
  pin = 0;
  alt = 0;

  switch (settings & (UART_PIN_TXMASK|UART_IFCMASK))
    {
     /* PORTA */
    case 0x00000101: // tx = 3
      pin = 3;
      alt = 6;
    case 0x00000104: // tx = 9
      pin = pin?pin:9;
      alt = alt?alt:2;
      PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock */
      PORT_PTR = PORTA; 
    break;
    /* PORTB */
    case 0x00000201: // tx = 1
      pin = 1;
      alt = 2;
      PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTB; 
    break;
    /* PORTC */
    case 0x00000401: // tx = 3
      pin = 3;
      alt = 4;
    case 0x00000102: // tx = 7
      pin = pin?pin:7;
      alt = alt?alt:2;
    case 0x00000202: // tx = 9
      pin = pin?pin:9;
      alt = alt?alt:2;
      PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTC; 
    break;
    /* PORTD */ 
    case 0x00000402: // tx = 14
      pin = 14;
      alt = 3;
    case 0x00000204: // tx = 7
      pin = pin?pin:7;
      alt = alt?alt:2;
      PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTD; 
    break;
    /* PORTE */ 
    case 0x00000404: // tx = 12
      pin = 12;
      alt = 3;
      PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTE; 
    break;
    default:
      pin = 7;
      alt = 2;
      PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
      PORT_PTR = PORTD; 
    break;
    }

  PORT_PTR->PCR[pin] |= PORT_PCR_MUX(alt); /* TX PIN MUX */
    
  PCC->PCCn[LPUART_INDEX] &= ~PCC_PCCn_CGC_MASK;   /* CLK disable */
  PCC->PCCn[LPUART_INDEX] |=  PCC_PCCn_PCS(0b110)| /* Clock Src= 6 (SPLLDIV2_CLK)   */
                              PCC_PCCn_CGC_MASK;   /* Enable clock for LPUART1 regs */

  uint32_t sbr = 1 + (uint32_t)(80000000/(30*9600*((settings & UART_BMASK)>>16)));
  /* p. 1702 of RM */
  /* Use SPPLDIV2 with divisor 2: 160Mhz/2 = 80MHz. See "clocks.c". */
  /* SBR = SRC_CLK / (BAUD_RATE x (OSR + 1))       */
  /* Example: 80000000/(921600x30) = 2.89 ~ 3      */
  LPUART_PTR->BAUD = (uint32_t)0;
  LPUART_PTR->BAUD = LPUART_BAUD_SBR(sbr)| /* Baud Rate Modulo Divisor [12:0] */
                     LPUART_BAUD_SBNS(0)|  /* Stop Bit Number Select: 0b - One stp bit; 1b - Two stop bits */
                     LPUART_BAUD_RXEDGIE(0)|
                     LPUART_BAUD_LBKDIE(0) |
                     LPUART_BAUD_RESYNCDIS(0)|
                     LPUART_BAUD_BOTHEDGE(0) |
                     LPUART_BAUD_MATCFG(0)|
                     LPUART_BAUD_RIDMAE(0)|
                     LPUART_BAUD_RDMAE(0) |
                     LPUART_BAUD_TDMAE(0) |
                     LPUART_BAUD_OSR(0x1C)| /* OSR = 29 */
                     LPUART_BAUD_M10(0)|
		     LPUART_BAUD_MAEN2(0)|
		     LPUART_BAUD_MAEN1(0);

  LPUART_PTR->CTRL = LPUART_CTRL_RE(1)| /* RE=1: Receiver enabled    */
                     LPUART_CTRL_TE(1); /* TE=1: Transmitter enabled */
}

void
uart_putc (char ch)
{
     /* Wait for transmit buffer to be empty */
     while ((LPUART_PTR->STAT & LPUART_STAT_TDRE_MASK)>>LPUART_STAT_TDRE_SHIFT==0);
     /* Send data */
     LPUART_PTR->DATA = ch;
}

void
uart_puts (char *outputbuf, uint32_t n)
{
    if (!outputbuf)
      return;

    char ch = 0;
    uint32_t count = 0;

    if (n == 0)
    {
        n = strlen(outputbuf);
    }

    while (1)
    {
        ch = outputbuf[count];

        if (count == n-1 || ch == '\0')
        {
            ch = '\r';
            uart_putc (ch);
            ch = '\n';
        }

        uart_putc (ch);

        if (ch == '\n')
        {
            break;
        }

        ++count;
    }
}

char
uart_getc (void)
{
    if (LPUART_PTR == NULL)
      return -1;

    char ch;
    /* Wait for received buffer to be full */
    while ((LPUART_PTR->STAT & LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT == 0);
    /* Read received data*/
    ch = (char) LPUART_PTR->DATA;
    return ch;
}

int
uart_gets (char *inputbuf, uint32_t n)
{
    if (!inputbuf)
      return -1;

    inputbuf[0] = '\0';

    char         ch       = 0;
    unsigned int count    = 0;
    unsigned int isfilled = 0;

    while (1)
      {
        ch = uart_getc();

        if (!isfilled)
          {
            if ((ch >= 'a' && ch <= 'z')||
                (ch >= 'A' && ch <= 'Z')||
                (ch >= '0' && ch <= '9')||
		(ch == ' ')
               )
              {
                inputbuf[count]  = ch;
                ++count;
              }
            else
            if (ch != '\r' && ch != '\n' && ch != '\0')
              {
                inputbuf[count] = '?';
                ++count;
              }

            if (count == n-1)
              {
                isfilled = 1;
                inputbuf[count] = '\0';
              }
          }

        if (ch == '\r' || ch == '\n' || ch == 0)
          {
            inputbuf[count] = '\0';
            break;
          }
      }

    return count;
}

void uart_reset (void)
{
  /* Set to after reset state an disable UART Tx/Rx */
  LPUART_PTR->CTRL = 0x00000000;
  LPUART_PTR->BAUD = 0x00000000;

  /* Disable clock to UART */
  PCC->PCCn[LPUART_INDEX] &= ~PCC_PCCn_CGC_MASK;
}



