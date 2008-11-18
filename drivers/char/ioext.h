/*
 * Shared data structure for GVP IO-Extender support.
 *
 * Merge of ioext.h and ser_ioext.h
 */
#ifndef _IOEXT_H_
#define _IOEXT_H_

#include <linux/netdevice.h>

#include "16c552.h"

#define MAX_IOEXT 5 /*
		     * The maximum number of io-extenders is 5, as you
		     * can't have more than 5 ZII boards in any Amiga.
		     */

#define UART_CLK 7372800

#define IOEXT_BAUD_BASE (UART_CLK / 16)

#define IOEXT_MAX_LINES 2

#define IOEXT_PAR_PLIP  0x0001
#define IOEXT_PAR_LP    0x0002


/*
 * Macros for the serial driver.
 */
#define curruart(info) ((struct uart_16c550 *)(info->port))

#define ser_DTRon(info)  curruart(info)->MCR |=  DTR
#define ser_RTSon(info)  curruart(info)->MCR |=  RTS
#define ser_DTRoff(info) curruart(info)->MCR &= ~DTR
#define ser_RTSoff(info) curruart(info)->MCR &= ~RTS


/*
 * CNTR defines (copied from the GVP SCSI-driver file gvp11.h
 */
#define GVP_BUSY	(1<<0)
#define GVP_IRQ_PEND	(1<<1)
#define GVP_IRQ_ENA	(1<<3)
#define GVP_DIR_WRITE   (1<<4)


/*
 * CTRL defines
 */
#define PORT0_MIDI   (1<<0)  /* CLR = DRIVERS         SET = MIDI      */
#define PORT1_MIDI   (1<<1)  /* CLR = DRIVERS         SET = MIDI      */
#define PORT0_DRIVER (1<<2)  /* CLR = RS232,          SET = MIDI      */
#define PORT1_DRIVER (1<<3)  /* CLR = RS232,          SET = MIDI      */
#define IRQ_SEL      (1<<4)  /* CLR = INT2,           SET = INT6      */
#define ROM_BANK_SEL (1<<5)  /* CLR = LOW 32K,        SET = HIGH 32K  */
#define PORT0_CTRL   (1<<6)  /* CLR = RTSx or RXRDYx, SET = RTSx ONLY */
#define PORT1_CTRL   (1<<7)  /* CLR = RTSx or RXRDYx, SET = RTSx ONLY */


/*
 * This is the struct describing the registers on the IO-Extender.
 * NOTE: The board uses a dual uart (16c552), which should be equal to
 * two 16c550 uarts.
 */
typedef struct {
	char gap0[0x41];
	volatile unsigned char CNTR;	/* GVP DMAC CNTR (status register)   */
	char gap1[0x11e];
	struct uart_16c550 uart0;	/* The first uart                    */
	char gap2[0xf0];
	struct uart_16c550 uart1;	/* The second uart                   */
	char gap3[0xf0];
	struct IOEXT_par par;		/* The parallel port                 */
	char gap4[0xfb];
	volatile unsigned char CTRL;	/* The control-register on the board */
} IOEXT_struct;


typedef struct {
	int num_uarts;
	int line[IOEXT_MAX_LINES];
	volatile struct uart_16c550 *uart[IOEXT_MAX_LINES];
	IOEXT_struct *board;
	int spurious_count;
	unsigned char par_use;		/* IOEXT_PAR_xxx */
#if defined(CONFIG_GVPIOEXT_PLIP) || defined(CONFIG_GVPIOEXT_PLIP_MODULE)
	struct nt_device *dev;
#endif
#if defined(CONFIG_GVPIOEXT_LP) || defined(CONFIG_GVPIOEXT_LP_MODULE)
	struct lp_struct *lp_table;
	int lp_dev;
	int lp_interrupt;
#endif
} IOExtInfoType;

/* Number of detected boards.  */
extern int ioext_num;
extern IOExtInfoType ioext_info[MAX_IOEXT];

void ioext_plip_interrupt(struct net_device *dev, int *spurious_count);
void ioext_lp_interrupt(int dev, int *spurious_count);

extern struct net_device ioext_dev_plip[3];
extern struct lp_struct ioext_lp_table[1];

#endif
