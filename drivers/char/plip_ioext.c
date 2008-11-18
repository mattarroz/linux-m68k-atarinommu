/*
 * plip_ioext: A parallel port "network" driver for GVP IO-Extender.
 *
 * Authors:	See drivers/net/plip.c
 *              IO-Extender version by Steve Bennett, <msteveb@ozemail.com.au>
 *
 * This driver is for use with a 5-bit cable (LapLink (R) cable).
 */

static const char *version = "NET3 PLIP version 2.2/m68k";

#define __NO_VERSION__

#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/termios.h>
#include <linux/tty.h>
#include <linux/serial.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/amigahw.h>
#include <asm/amigaints.h>
#include <linux/zorro.h>

#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/if_ether.h>

#include <asm/system.h>

#include <linux/in.h>
#include <linux/delay.h>
/*#include <linux/lp_m68k.h>*/

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/if_plip.h>

#include <linux/tqueue.h>
#include <linux/ioport.h>
#include <linux/bitops.h>
#include <asm/byteorder.h>

#include "ioext.h"

#define DEBUG 0

/* Map 'struct device *' to our control structure */
#define PLIP_DEV(DEV) (&ioext_info[(DEV)->irq])

/************************************************************************
**
** PLIP definitions
**
*************************************************************************
*/

/* Use 0 for production, 1 for verification, >2 for debug */
#ifndef NET_DEBUG
#define NET_DEBUG 2
#endif
static unsigned int net_debug = NET_DEBUG;

/* In micro second */
#define PLIP_DELAY_UNIT       1

/* Connection time out = PLIP_TRIGGER_WAIT * PLIP_DELAY_UNIT usec */
#define PLIP_TRIGGER_WAIT	 500

/* Nibble time out = PLIP_NIBBLE_WAIT * PLIP_DELAY_UNIT usec */
#define PLIP_NIBBLE_WAIT        3000

#define PAR_DATA(dev)     ((dev)->base_addr+0)
#define PAR_STATUS(dev)   ((dev)->base_addr+2)
#define PAR_CONTROL(dev)  ((dev)->base_addr+4)

static void enable_par_irq(struct device *dev, int on);
static int plip_init(struct device *dev);

/* Bottom halfs */
static void plip_kick_bh(struct device *dev);
static void plip_bh(struct device *dev);

/* Functions for DEV methods */
static int plip_rebuild_header(struct sk_buff *skb);
static int plip_tx_packet(struct sk_buff *skb, struct device *dev);
static int plip_open(struct device *dev);
static int plip_close(struct device *dev);
static struct enet_statistics *plip_get_stats(struct device *dev);
static int plip_config(struct device *dev, struct ifmap *map);
static int plip_ioctl(struct device *dev, struct ifreq *ifr, int cmd);

enum plip_connection_state {
	PLIP_CN_NONE=0,
	PLIP_CN_RECEIVE,
	PLIP_CN_SEND,
	PLIP_CN_CLOSING,
	PLIP_CN_ERROR
};

enum plip_packet_state {
	PLIP_PK_DONE=0,
	PLIP_PK_TRIGGER,
	PLIP_PK_LENGTH_LSB,
	PLIP_PK_LENGTH_MSB,
	PLIP_PK_DATA,
	PLIP_PK_CHECKSUM
};

enum plip_nibble_state {
	PLIP_NB_BEGIN,
	PLIP_NB_1,
	PLIP_NB_2,
};

struct plip_local {
	enum plip_packet_state state;
	enum plip_nibble_state nibble;
	union {
		struct {
#if defined(__LITTLE_ENDIAN)
			unsigned char lsb;
			unsigned char msb;
#elif defined(__BIG_ENDIAN)
			unsigned char msb;
			unsigned char lsb;
#else
#error  "Please fix the endianness defines in <asm/byteorder.h>"
#endif
		} b;
		unsigned short h;
	} length;
	unsigned short byte;
	unsigned char  checksum;
	unsigned char  data;
	struct sk_buff *skb;
};

struct net_local {
	struct enet_statistics enet_stats;
	struct tq_struct immediate;
	struct tq_struct deferred;
	struct plip_local snd_data;
	struct plip_local rcv_data;
	unsigned long  trigger;
	unsigned long  nibble;
	enum plip_connection_state connection;
	unsigned short timeout_count;
	char is_deferred;
	int (*orig_rebuild_header)(struct sk_buff *skb);
};

struct device ioext_dev_plip[] = {
	{
		"plip0",
		0, 0, 0, 0,    /* memory */
		0, 0,    /* base, irq */
		0, 0, 0, NULL, plip_init
	},
	{
		"plip1",
		0, 0, 0, 0,    /* memory */
		0, 0,    /* base, irq */
		0, 0, 0, NULL, plip_init
	},
	{
		"plip2",
		0, 0, 0, 0,    /* memory */
		0, 0,    /* base, irq */
		0, 0, 0, NULL, plip_init
	}
};

/*
 * Check for and handle an interrupt for this PLIP device.
 *
 */
void ioext_plip_interrupt(struct device *dev, int *spurious_count)
{
	struct net_local *nl;
	struct plip_local *rcv;
	unsigned char c0;
	unsigned long flags;

	nl = (struct net_local *)dev->priv;
	rcv = &nl->rcv_data;

	c0 = z_readb(PAR_STATUS(dev));

	if (dev->interrupt) {
		return;
	}

	if ((c0 & 0xf8) != 0xc0) {
		/* Not for us */
		++*spurious_count;
		return;
	}

	*spurious_count = 0;
	dev->interrupt = 1;

	local_irq_save(flags);

	switch (nl->connection) {
	case PLIP_CN_CLOSING:
		dev->tbusy = 0;
	case PLIP_CN_NONE:
	case PLIP_CN_SEND:
		dev->last_rx = jiffies;
		rcv->state = PLIP_PK_TRIGGER;
		nl->connection = PLIP_CN_RECEIVE;
		nl->timeout_count = 0;
		queue_task(&nl->immediate, &tq_immediate);
		mark_bh(IMMEDIATE_BH);
		local_irq_restore(flags);
#if 0
		printk("%s: receive irq in SEND/NONE/CLOSING (%d) ok\n",
		       dev->name, nl->connection);
#endif
		break;

	case PLIP_CN_RECEIVE:
		local_irq_restore(flags);
		printk("%s: receive interrupt when receiving packet\n",
		       dev->name);
		break;

	case PLIP_CN_ERROR:
		local_irq_restore(flags);
		printk("%s: receive interrupt in error state\n", dev->name);
		break;
	}
}


/* Bottom half handler for the delayed request.
   This routine is kicked by do_timer().
   Request `plip_bh' to be invoked. */
static void
plip_kick_bh(struct device *dev)
{
	struct net_local *nl = (struct net_local *)dev->priv;

	if (nl->is_deferred) {
		queue_task(&nl->immediate, &tq_immediate);
		mark_bh(IMMEDIATE_BH);
	}
}

/* Forward declarations of internal routines */
static int plip_none(struct device *, struct net_local *,
		     struct plip_local *, struct plip_local *);
static int plip_receive_packet(struct device *, struct net_local *,
			       struct plip_local *, struct plip_local *);
static int plip_send_packet(struct device *, struct net_local *,
			    struct plip_local *, struct plip_local *);
static int plip_connection_close(struct device *, struct net_local *,
				 struct plip_local *, struct plip_local *);
static int plip_error(struct device *, struct net_local *,
		      struct plip_local *, struct plip_local *);
static int plip_bh_timeout_error(struct device *dev, struct net_local *nl,
				 struct plip_local *snd,
				 struct plip_local *rcv,
				 int error);

#define OK        0
#define TIMEOUT   1
#define ERROR     2

typedef int (*plip_func)(struct device *dev, struct net_local *nl,
			 struct plip_local *snd, struct plip_local *rcv);

static plip_func connection_state_table[] =
{
	plip_none,
	plip_receive_packet,
	plip_send_packet,
	plip_connection_close,
	plip_error
};

/*
** enable_par_irq()
**
** Enable or disable parallel irq for 'dev' according to 'on'.
**
** It is NOT possible to disable only the parallel irq.
** So we disable the board interrupt instead. This means that
** during reception of a PLIP packet, no serial interrupts can
** happen. Sorry.
*/
static void enable_par_irq(struct device *dev, int on)
{
	if (on) {
		PLIP_DEV(dev)->board->CNTR |= GVP_IRQ_ENA;
	}
	else {
		PLIP_DEV(dev)->board->CNTR &= ~GVP_IRQ_ENA;
	}
}

/* Bottom half handler of PLIP. */
static void
plip_bh(struct device *dev)
{
	struct net_local *nl = (struct net_local *)dev->priv;
	struct plip_local *snd = &nl->snd_data;
	struct plip_local *rcv = &nl->rcv_data;
	plip_func f;
	int r;

	nl->is_deferred = 0;
	f = connection_state_table[nl->connection];
	if ((r = (*f)(dev, nl, snd, rcv)) != OK
	    && (r = plip_bh_timeout_error(dev, nl, snd, rcv, r)) != OK) {
		nl->is_deferred = 1;
		queue_task(&nl->deferred, &tq_timer);
	}
}

static int
plip_bh_timeout_error(struct device *dev, struct net_local *nl,
		      struct plip_local *snd, struct plip_local *rcv,
		      int error)
{
	unsigned char c0;
	unsigned long flags;

	local_irq_save(flags);
	if (nl->connection == PLIP_CN_SEND) {

		if (error != ERROR) { /* Timeout */
			nl->timeout_count++;
			if ((snd->state == PLIP_PK_TRIGGER
			     && nl->timeout_count <= 10)
			    || nl->timeout_count <= 3) {
				local_irq_restore(flags);
				/* Try again later */
				return TIMEOUT;
			}
			c0 = z_readb(PAR_STATUS(dev));
			printk(KERN_INFO "%s: transmit timeout(%d,%02x)\n",
			       dev->name, snd->state, c0);
		}
		nl->enet_stats.tx_errors++;
		nl->enet_stats.tx_aborted_errors++;
	} else if (nl->connection == PLIP_CN_RECEIVE) {
		if (rcv->state == PLIP_PK_TRIGGER) {
			/* Transmission was interrupted. */
			local_irq_restore(flags);
			return OK;
		}
		if (error != ERROR) { /* Timeout */
			if (++nl->timeout_count <= 3) {
				local_irq_restore(flags);
				/* Try again later */
				return TIMEOUT;
			}
			c0 = z_readb(PAR_STATUS(dev));
			printk(KERN_INFO "%s: receive timeout(%d,%02x)\n",
			       dev->name, rcv->state, c0);
		}
		nl->enet_stats.rx_dropped++;
	}
	rcv->state = PLIP_PK_DONE;
	if (rcv->skb) {
		kfree_skb(rcv->skb);
		rcv->skb = NULL;
	}
	snd->state = PLIP_PK_DONE;
	if (snd->skb) {
		dev_kfree_skb(snd->skb);
		snd->skb = NULL;
	}
	enable_par_irq(dev, 0);
	dev->tbusy = 1;
	nl->connection = PLIP_CN_ERROR;
	z_writeb(0x00, PAR_DATA(dev));
	local_irq_restore(flags);

	return TIMEOUT;
}

static int
plip_none(struct device *dev, struct net_local *nl,
	  struct plip_local *snd, struct plip_local *rcv)
{
	return OK;
}

/* PLIP_RECEIVE --- receive a byte(two nibbles)
   Returns OK on success, TIMEOUT on timeout */
inline static int
plip_receive(struct device *dev, unsigned short nibble_timeout,
	     enum plip_nibble_state *ns_p, unsigned char *data_p)
{
	unsigned char c0, c1;
	unsigned int cx;

	switch (*ns_p) {
	case PLIP_NB_BEGIN:
		cx = nibble_timeout;
		while (1) {
			c0 = z_readb(PAR_STATUS(dev));
			udelay(PLIP_DELAY_UNIT);
			if ((c0 & 0x80) == 0) {
				c1 = z_readb(PAR_STATUS(dev));
				if (c0 == c1)
					break;
			}
			if (--cx == 0)
				return TIMEOUT;
		}
#if 0
		printk("received first nybble: %02X -> %02X\n",
		       c0, (c0 >> 3) & 0x0F);
#endif
		*data_p = (c0 >> 3) & 0x0f;
		z_writeb(0x10, PAR_DATA(dev)); /* send ACK */
		*ns_p = PLIP_NB_1;

	case PLIP_NB_1:
		cx = nibble_timeout;
		while (1) {
			c0 = z_readb(PAR_STATUS(dev));
			udelay(PLIP_DELAY_UNIT);
			if (c0 & 0x80) {
				c1 = z_readb(PAR_STATUS(dev));
				if (c0 == c1)
					break;
			}
			if (--cx == 0)
				return TIMEOUT;
		}
#if 0
		printk("received second nybble: %02X -> %02X\n",
		       c0, (c0 << 1) & 0xF0);
#endif
		*data_p |= (c0 << 1) & 0xf0;
		z_writeb(0x00, PAR_DATA(dev)); /* send ACK */
		*ns_p = PLIP_NB_BEGIN;
	case PLIP_NB_2:
		break;
	}
	return OK;
}

/* PLIP_RECEIVE_PACKET --- receive a packet */
static int
plip_receive_packet(struct device *dev, struct net_local *nl,
		    struct plip_local *snd, struct plip_local *rcv)
{
	unsigned short nibble_timeout = nl->nibble;
	unsigned char *lbuf;
	unsigned long flags;

	switch (rcv->state) {
	case PLIP_PK_TRIGGER:
		enable_par_irq(dev, 0);
		dev->interrupt = 0;
		z_writeb(0x01, PAR_DATA(dev)); /* send ACK */
		if (net_debug > 2)
			printk(KERN_DEBUG "%s: receive start\n", dev->name);
		rcv->state = PLIP_PK_LENGTH_LSB;
		rcv->nibble = PLIP_NB_BEGIN;

	case PLIP_PK_LENGTH_LSB:
		if (snd->state != PLIP_PK_DONE) {
			if (plip_receive(dev, nl->trigger,
					 &rcv->nibble, &rcv->length.b.lsb)) {
				/* collision, here dev->tbusy == 1 */
				rcv->state = PLIP_PK_DONE;
				nl->is_deferred = 1;
				nl->connection = PLIP_CN_SEND;
				queue_task(&nl->deferred, &tq_timer);
				enable_par_irq(dev, 1);
				return OK;
			}
		} else {
			if (plip_receive(dev, nibble_timeout,
					 &rcv->nibble, &rcv->length.b.lsb))
				return TIMEOUT;
		}
		rcv->state = PLIP_PK_LENGTH_MSB;

	case PLIP_PK_LENGTH_MSB:
		if (plip_receive(dev, nibble_timeout,
				 &rcv->nibble, &rcv->length.b.msb))
			return TIMEOUT;
		if (rcv->length.h > dev->mtu + dev->hard_header_len
		    || rcv->length.h < 8) {
			printk(KERN_INFO "%s: bogus packet size %d.\n",
			       dev->name, rcv->length.h);
			return ERROR;
		}
		/* Malloc up new buffer. */
		rcv->skb = dev_alloc_skb(rcv->length.h);
		if (rcv->skb == NULL) {
			printk(KERN_INFO "%s: Memory squeeze.\n", dev->name);
			return ERROR;
		}
		skb_put(rcv->skb,rcv->length.h);
		rcv->skb->dev = dev;
		rcv->state = PLIP_PK_DATA;
		rcv->byte = 0;
		rcv->checksum = 0;

	case PLIP_PK_DATA:
		lbuf = rcv->skb->data;
		do
			if (plip_receive(dev, nibble_timeout,
					 &rcv->nibble, &lbuf[rcv->byte]))
				return TIMEOUT;
		while (++rcv->byte < rcv->length.h);
		do
			rcv->checksum += lbuf[--rcv->byte];
		while (rcv->byte);
		rcv->state = PLIP_PK_CHECKSUM;

	case PLIP_PK_CHECKSUM:
		if (plip_receive(dev, nibble_timeout,
				 &rcv->nibble, &rcv->data))
			return TIMEOUT;
		if (rcv->data != rcv->checksum) {
			nl->enet_stats.rx_crc_errors++;
			if (net_debug)
				printk(KERN_INFO "%s: checksum error\n",
				       dev->name);
			return ERROR;
		}
		rcv->state = PLIP_PK_DONE;

	case PLIP_PK_DONE:
		/* Inform the upper layer for the arrival of a packet. */
		rcv->skb->protocol=eth_type_trans(rcv->skb, dev);
		netif_rx(rcv->skb);
		nl->enet_stats.rx_packets++;
		rcv->skb = NULL;
		if (net_debug > 2)
			printk(KERN_DEBUG "%s: receive end\n", dev->name);

		/* Close the connection. */
		z_writeb (0x00, PAR_DATA(dev));

		local_irq_save(flags);
		if (snd->state != PLIP_PK_DONE) {
			nl->connection = PLIP_CN_SEND;
			local_irq_restore(flags);
			queue_task(&nl->immediate, &tq_immediate);
			mark_bh(IMMEDIATE_BH);
			enable_par_irq(dev, 1);
			return OK;
		} else {
			nl->connection = PLIP_CN_NONE;
			local_irq_restore(flags);
			enable_par_irq(dev, 1);
			return OK;
		}
	}
	return OK;
}

/* PLIP_SEND --- send a byte (two nibbles)
   Returns OK on success, TIMEOUT when timeout    */
inline static int
plip_send(struct device *dev, unsigned short nibble_timeout,
	  enum plip_nibble_state *ns_p, unsigned char data)
{
	unsigned char c0;
	unsigned int cx;

	switch (*ns_p) {
	case PLIP_NB_BEGIN:
		z_writeb((data & 0x0f), PAR_DATA(dev));
		*ns_p = PLIP_NB_1;

	case PLIP_NB_1:
		z_writeb(0x10 | (data & 0x0f), PAR_DATA(dev));
		cx = nibble_timeout;
		while (1) {
			c0 = z_readb(PAR_STATUS(dev));
			if ((c0 & 0x80) == 0)
				break;
			if (--cx == 0)
				return TIMEOUT;
			udelay(PLIP_DELAY_UNIT);
		}
		z_writeb(0x10 | (data >> 4), PAR_DATA(dev));
		*ns_p = PLIP_NB_2;

	case PLIP_NB_2:
		z_writeb((data >> 4), PAR_DATA(dev));
		cx = nibble_timeout;
		while (1) {
			c0 = z_readb(PAR_STATUS(dev));
			if (c0 & 0x80)
				break;
			if (--cx == 0)
				return TIMEOUT;
			udelay(PLIP_DELAY_UNIT);
		}
		*ns_p = PLIP_NB_BEGIN;
		return OK;
	}
	return OK;
}

/* PLIP_SEND_PACKET --- send a packet */
static int
plip_send_packet(struct device *dev, struct net_local *nl,
		 struct plip_local *snd, struct plip_local *rcv)
{
	unsigned short nibble_timeout = nl->nibble;
	unsigned char *lbuf;
	unsigned char c0;
	unsigned int cx;
	unsigned long flags;

	if (snd->skb == NULL || (lbuf = snd->skb->data) == NULL) {
		printk(KERN_INFO "%s: send skb lost\n", dev->name);
		snd->state = PLIP_PK_DONE;
		snd->skb = NULL;
		return ERROR;
	}

	if (snd->length.h == 0) {
		return OK;
	}

	switch (snd->state) {
	case PLIP_PK_TRIGGER:
		if ((z_readb(PAR_STATUS(dev)) & 0xf8) != 0x80)
			return TIMEOUT;

		/* Trigger remote rx interrupt. */
		z_writeb(0x08, PAR_DATA(dev));
		cx = nl->trigger;
		while (1) {
			udelay(PLIP_DELAY_UNIT);
                        local_irq_save(flags);
			if (nl->connection == PLIP_CN_RECEIVE) {
				local_irq_restore(flags);
				/* interrupted */
				nl->enet_stats.collisions++;
				if (net_debug > 1)
					printk(KERN_INFO "%s: collision.\n",
					       dev->name);
				return OK;
			}
			c0 = z_readb(PAR_STATUS(dev));
			if (c0 & 0x08) {
				enable_par_irq(dev, 0);
				if (net_debug > 2)
					printk(KERN_DEBUG "%s: send start\n",
					       dev->name);
				snd->state = PLIP_PK_LENGTH_LSB;
				snd->nibble = PLIP_NB_BEGIN;
				nl->timeout_count = 0;
				local_irq_restore(flags);
				break;
			}
			local_irq_restore(flags);
			if (--cx == 0) {
				z_writeb(0x00, PAR_DATA(dev));
				return TIMEOUT;
			}
		}

	case PLIP_PK_LENGTH_LSB:
		if (plip_send(dev, nibble_timeout,
			      &snd->nibble, snd->length.b.lsb))
			return TIMEOUT;
		snd->state = PLIP_PK_LENGTH_MSB;

	case PLIP_PK_LENGTH_MSB:
		if (plip_send(dev, nibble_timeout,
			      &snd->nibble, snd->length.b.msb))
			return TIMEOUT;
		snd->state = PLIP_PK_DATA;
		snd->byte = 0;
		snd->checksum = 0;

	case PLIP_PK_DATA:
		do
			if (plip_send(dev, nibble_timeout,
				      &snd->nibble, lbuf[snd->byte]))
				return TIMEOUT;
		while (++snd->byte < snd->length.h);
		do
			snd->checksum += lbuf[--snd->byte];
		while (snd->byte);
		snd->state = PLIP_PK_CHECKSUM;

	case PLIP_PK_CHECKSUM:
		if (plip_send(dev, nibble_timeout,
			      &snd->nibble, snd->checksum))
			return TIMEOUT;

		dev_kfree_skb(snd->skb);
		nl->enet_stats.tx_packets++;
		snd->state = PLIP_PK_DONE;

	case PLIP_PK_DONE:
		/* Close the connection */
		z_writeb (0x00, PAR_DATA(dev));
		snd->skb = NULL;
		if (net_debug > 2)
			printk(KERN_DEBUG "%s: send end\n", dev->name);
		nl->connection = PLIP_CN_CLOSING;
		nl->is_deferred = 1;
		queue_task(&nl->deferred, &tq_timer);
		enable_par_irq(dev, 1);
		return OK;
	}
	return OK;
}

static int
plip_connection_close(struct device *dev, struct net_local *nl,
		      struct plip_local *snd, struct plip_local *rcv)
{
	unsigned long flags;

        local_irq_save(flags);
	if (nl->connection == PLIP_CN_CLOSING) {
		nl->connection = PLIP_CN_NONE;
		dev->tbusy = 0;
		mark_bh(NET_BH);
	}
	local_irq_restore(flags);
	return OK;
}

/* PLIP_ERROR --- wait till other end settled */
static int
plip_error(struct device *dev, struct net_local *nl,
	   struct plip_local *snd, struct plip_local *rcv)
{
	unsigned char status;

	status = z_readb(PAR_STATUS(dev));
	if ((status & 0xf8) == 0x80) {
		if (net_debug > 2)
			printk(KERN_DEBUG "%s: reset interface.\n", dev->name);
		nl->connection = PLIP_CN_NONE;
		dev->tbusy = 0;
		dev->interrupt = 0;
		enable_par_irq(dev, 1);
		mark_bh(NET_BH);
	} else {
		nl->is_deferred = 1;
		queue_task(&nl->deferred, &tq_timer);
	}

	return OK;
}

/* We don't need to send arp, for plip is point-to-point. */
static int
plip_rebuild_header(struct sk_buff *skb)
{
	struct device *dev = skb->dev;
	struct net_local *nl = (struct net_local *)dev->priv;
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	int i;

	if ((dev->flags & IFF_NOARP)==0)
		return nl->orig_rebuild_header(skb);

	if (eth->h_proto != __constant_htons(ETH_P_IP)
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	    && eth->h_proto != __constant_htons(ETH_P_IPV6)
#endif
		) {
		printk(KERN_ERR "plip_rebuild_header: Don't know how to resolve type %d addresses?\n", (int)eth->h_proto);
		memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
		return 0;
	}

	for (i=0; i < ETH_ALEN - sizeof(u32); i++)
		eth->h_dest[i] = 0xfc;
#if 0
	*(u32 *)(eth->h_dest+i) = dst;
#else
	/* Do not want to include net/route.h here.
	 * In any case, it is TOP of silliness to emulate
	 * hardware addresses on PtP link. --ANK
	 */
	*(u32 *)(eth->h_dest+i) = 0;
#endif
	return 0;
}

static int
plip_tx_packet(struct sk_buff *skb, struct device *dev)
{
	struct net_local *nl = (struct net_local *)dev->priv;
	struct plip_local *snd = &nl->snd_data;
	unsigned long flags;

	if (dev->tbusy)
		return 1;

	if (test_and_set_bit(0, (void*)&dev->tbusy) != 0) {
		printk(KERN_ERR "%s: Transmitter access conflict.\n",
		       dev->name);
		return 1;
	}

	if (skb->len > dev->mtu + dev->hard_header_len) {
		printk(KERN_ERR "%s: packet too big, %d.\n",
		       dev->name, (int)skb->len);
		dev->tbusy = 0;
		return 0;
	}

	if (net_debug > 2)
		printk(KERN_DEBUG "%s: send request\n", dev->name);

	local_irq_save(flags);
	dev->trans_start = jiffies;
	snd->skb = skb;
	snd->length.h = skb->len;
	snd->state = PLIP_PK_TRIGGER;
	if (nl->connection == PLIP_CN_NONE) {
		nl->connection = PLIP_CN_SEND;
		nl->timeout_count = 0;
	}
	queue_task(&nl->immediate, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
	local_irq_restore(flags);

	return 0;
}

/* Open/initialize the board.  This is called (in the current kernel)
   sometime after booting when the 'ifconfig' program is run.

 */
static int
plip_open(struct device *dev)
{
	struct net_local *nl = (struct net_local *)dev->priv;
	struct in_device *in_dev;

#if defined(CONFIG_GVPIOEXT_LP) || defined(CONFIG_GVPIOEXT_LP_MODULE)
	/* Yes, there is a race condition here. Fix it later */
	if (PLIP_DEV(dev)->par_use & IOEXT_PAR_LP) {
		/* Can't open if lp is in use */
#if DEBUG
		printk("par is in use by lp\n");
#endif
		return(-EBUSY);
	}
#endif
	PLIP_DEV(dev)->par_use |= IOEXT_PAR_PLIP;

#if DEBUG
	printk("plip_open(): sending 00 to data port\n");
#endif

	/* Clear the data port. */
	z_writeb (0x00, PAR_DATA(dev));

#if DEBUG
	printk("plip_open(): sent\n");
#endif

	/* Initialize the state machine. */
	nl->rcv_data.state = nl->snd_data.state = PLIP_PK_DONE;
	nl->rcv_data.skb = nl->snd_data.skb = NULL;
	nl->connection = PLIP_CN_NONE;
	nl->is_deferred = 0;

	/* Fill in the MAC-level header.
	   (ab)Use "dev->broadcast" to store point-to-point MAC address.

	   PLIP doesn't have a real mac address, but we need to create one
	   to be DOS compatible.  */
	memset(dev->dev_addr,  0xfc, ETH_ALEN);
	memset(dev->broadcast, 0xfc, ETH_ALEN);

	if ((in_dev=dev->ip_ptr) != NULL) {
		/*
		 *	Any address will do - we take the first
		 */
		struct in_ifaddr *ifa=in_dev->ifa_list;
		if (ifa != NULL) {
			memcpy(dev->dev_addr+2, &ifa->ifa_local, 4);
			memcpy(dev->broadcast+2, &ifa->ifa_address, 4);
		}
	}

	dev->interrupt = 0;
	dev->start = 1;
	dev->tbusy = 0;

	MOD_INC_USE_COUNT;

	/* Enable rx interrupt. */
	enable_par_irq(dev, 1);

	return 0;
}

/* The inverse routine to plip_open (). */
static int
plip_close(struct device *dev)
{
	struct net_local *nl = (struct net_local *)dev->priv;
	struct plip_local *snd = &nl->snd_data;
	struct plip_local *rcv = &nl->rcv_data;
	unsigned long flags;

	dev->tbusy = 1;
	dev->start = 0;
        local_irq_save(flags);
	nl->is_deferred = 0;
	nl->connection = PLIP_CN_NONE;
	local_irq_restore(flags);
	z_writeb(0x00, PAR_DATA(dev));

	snd->state = PLIP_PK_DONE;
	if (snd->skb) {
		dev_kfree_skb(snd->skb);
		snd->skb = NULL;
	}
	rcv->state = PLIP_PK_DONE;
	if (rcv->skb) {
		kfree_skb(rcv->skb);
		rcv->skb = NULL;
	}

	PLIP_DEV(dev)->par_use &= ~IOEXT_PAR_PLIP;

	MOD_DEC_USE_COUNT;
	return 0;
}

static struct enet_statistics *
plip_get_stats(struct device *dev)
{
	struct net_local *nl = (struct net_local *)dev->priv;
	struct enet_statistics *r = &nl->enet_stats;

	return r;
}

static int
plip_config(struct device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP)
		return -EBUSY;

	printk(KERN_INFO "%s: This interface is autodetected (ignored).\n",
	       dev->name);

	return 0;
}

static int
plip_ioctl(struct device *dev, struct ifreq *rq, int cmd)
{
	struct net_local *nl = (struct net_local *) dev->priv;
	struct plipconf *pc = (struct plipconf *) &rq->ifr_data;

	switch(pc->pcmd) {
	case PLIP_GET_TIMEOUT:
		pc->trigger = nl->trigger;
		pc->nibble  = nl->nibble;
		break;
	case PLIP_SET_TIMEOUT:
		nl->trigger = pc->trigger;
		nl->nibble  = pc->nibble;
		break;
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

/*
 * Detect and initialize all IO-Extenders in this system.
 *
 * Both PLIP and serial devices are configured.
 */
int plip_init(struct device *dev)
{
	IOEXT_struct *board;
	struct net_local *nl;

	if (ioext_num == 0) {
		printk(KERN_INFO "%s\n", version);
	}

	board = PLIP_DEV(dev)->board;
	dev->base_addr = (unsigned long)&board->par.DATA;

	/* Cheat and use irq to index into our table */
	dev->irq = ioext_num;

	printk(KERN_INFO "%s: IO-Extender parallel port at 0x%08lX\n", dev->name, dev->base_addr);

	/* Fill in the generic fields of the device structure. */
	ether_setup(dev);

	/* Then, override parts of it */
	dev->hard_start_xmit  = plip_tx_packet;
	dev->open    = plip_open;
	dev->stop    = plip_close;
	dev->get_stats     = plip_get_stats;
	dev->set_config    = plip_config;
	dev->do_ioctl    = plip_ioctl;
	dev->tx_queue_len  = 10;
	dev->flags          = IFF_POINTOPOINT|IFF_NOARP;

	/* Set the private structure */
	dev->priv = kmalloc(sizeof (struct net_local), GFP_KERNEL);
	if (dev->priv == NULL) {
		printk(KERN_ERR "%s: out of memory\n", dev->name);
		return -ENOMEM;
	}
	memset(dev->priv, 0, sizeof(struct net_local));
	nl = (struct net_local *) dev->priv;

	nl->orig_rebuild_header = dev->rebuild_header;
	dev->rebuild_header   = plip_rebuild_header;

	/* Initialize constants */
	nl->trigger  = PLIP_TRIGGER_WAIT;
	nl->nibble  = PLIP_NIBBLE_WAIT;

	/* Initialize task queue structures */
	nl->immediate.next = NULL;
	nl->immediate.sync = 0;
	nl->immediate.routine = (void *)(void *)plip_bh;
	nl->immediate.data = dev;

	nl->deferred.next = NULL;
	nl->deferred.sync = 0;
	nl->deferred.routine = (void *)(void *)plip_kick_bh;
	nl->deferred.data = dev;

	/* Don't enable interrupts yet */

	return 0;
}
