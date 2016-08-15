/*
 * linux/arch/m68k/kernel/ints.c -- Linux/m68k general interrupt handling code
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/irq.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/page.h>
#include <asm/machdep.h>
#include <asm/cacheflush.h>
#include <asm/irq_regs.h>

#ifdef CONFIG_Q40
#include <asm/q40ints.h>
#endif

extern u32 auto_irqhandler_fixup[];
extern u16 user_irqvec_fixup[];

static int m68k_first_user_vec;

static struct irq_chip auto_irq_chip = {
	.name		= "auto",
	.irq_startup	= m68k_irq_startup,
	.irq_shutdown	= m68k_irq_shutdown,
};

static struct irq_chip user_irq_chip = {
	.name		= "user",
	.irq_startup	= m68k_irq_startup,
	.irq_shutdown	= m68k_irq_shutdown,
};

/*
 * void init_IRQ(void)
 *
 * Parameters:	None
 *
 * Returns:	Nothing
 *
 * This function should be called during kernel startup to initialize
 * the IRQ handling routines.
 */

void __init init_IRQ(void)
{
	int i;

	for (i = IRQ_AUTO_1; i <= IRQ_AUTO_7; i++)
		irq_set_chip_and_handler(i, &auto_irq_chip, handle_simple_irq);

	mach_init_IRQ();
}

/**
 * m68k_setup_auto_interrupt
 * @handler: called from auto vector interrupts
 *
 * setup the handler to be called from auto vector interrupts instead of the
 * standard do_IRQ(), it will be called with irq numbers in the range
 * from IRQ_AUTO_1 - IRQ_AUTO_7.
 */
void __init m68k_setup_auto_interrupt(void (*handler)(unsigned int, struct pt_regs *))
{
	if (handler)
		*auto_irqhandler_fixup = (u32)handler;
#ifndef CONFIG_M68000
	flush_icache();
#endif
}

/**
 * m68k_setup_user_interrupt
 * @vec: first user vector interrupt to handle
 * @cnt: number of active user vector interrupts
 *
 * setup user vector interrupts, this includes activating the specified range
 * of interrupts, only then these interrupts can be requested (note: this is
 * different from auto vector interrupts).
 */
void __init m68k_setup_user_interrupt(unsigned int vec, unsigned int cnt)
{
	int i;

	BUG_ON(IRQ_USER + cnt > NR_IRQS);
	m68k_first_user_vec = vec;
	for (i = 0; i < cnt; i++)
		irq_set_chip_and_handler(i, &user_irq_chip, handle_simple_irq);
	*user_irqvec_fixup = vec - IRQ_USER;
#ifndef CONFIG_M68000
	flush_icache();
#endif
}

/**
 * m68k_setup_irq_controller
 * @chip: irq chip which controls specified irq
 * @handle: flow handler which handles specified irq
 * @irq: first irq to be managed by the controller
 * @cnt: number of irqs to be managed by the controller
 *
 * Change the controller for the specified range of irq, which will be used to
 * manage these irq. auto/user irq already have a default controller, which can
 * be changed as well, but the controller probably should use m68k_irq_startup/
 * m68k_irq_shutdown.
 */
void m68k_setup_irq_controller(struct irq_chip *chip,
			       irq_flow_handler_t handle, unsigned int irq,
			       unsigned int cnt)
{
	int i;

	for (i = 0; i < cnt; i++) {
		irq_set_chip(irq + i, chip);
		if (handle)
			irq_set_handler(irq + i, handle);
	}
}

unsigned int m68k_irq_startup_irq(unsigned int irq)
{
	if (irq <= IRQ_AUTO_7)
#ifdef CONFIG_M68000
		switch (irq) {
               case IRQ_AUTO_1:
                       vectors[VEC_INT1] = auto_inthandler1;
                       break;
               case IRQ_AUTO_2:
                       vectors[VEC_INT2] = auto_inthandler2;
                       break;
               case IRQ_AUTO_3:
                       vectors[VEC_INT3] = auto_inthandler3;
                       break;
               case IRQ_AUTO_4:
                       vectors[VEC_INT4] = auto_inthandler4;
                       break;
               case IRQ_AUTO_5:
                       vectors[VEC_INT5] = auto_inthandler5;
                       break;
               case IRQ_AUTO_6:
                       vectors[VEC_INT6] = auto_inthandler6;
                       break;
               case IRQ_AUTO_7:
                       vectors[VEC_INT7] = auto_inthandler7;
                       break;
		}	
#else
		vectors[VEC_SPUR + irq] = auto_inthandler;
#endif
	else
#ifdef CONFIG_M68000
		switch (irq) {
		case IRQ_USER:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler1;
			break;
		case IRQ_USER + 1:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler2;
			break;
		case IRQ_USER + 2:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler3;
			break;
		case IRQ_USER + 3:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler4;
			break;
		case IRQ_USER + 4:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler5;
			break;
		case IRQ_USER + 5:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler6;
			break;
		case IRQ_USER + 6:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler7;
			break;
		case IRQ_USER + 7:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler8;
			break;
		case IRQ_USER + 8:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler9;
			break;
		case IRQ_USER + 9:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler10;
			break;
		case IRQ_USER + 10:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler11;
			break;
		case IRQ_USER + 11:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler12;
			break;
		case IRQ_USER + 12:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler13;
			break;
		case IRQ_USER + 13:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler14;
			break;
		case IRQ_USER + 14:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler15;
			break;
		case IRQ_USER + 15:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler16;
			break;
		case IRQ_USER + 16:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler17;
			break;
		case IRQ_USER + 17:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler18;
			break;
		case IRQ_USER + 18:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler19;
			break;
		case IRQ_USER + 19:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler20;
			break;
		case IRQ_USER + 20:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler21;
			break;
		case IRQ_USER + 21:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler22;
			break;
		case IRQ_USER + 22:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler23;
			break;
		case IRQ_USER + 23:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler24;
			break;
		case IRQ_USER + 24:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler25;
			break;
		case IRQ_USER + 25:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler26;
			break;
		case IRQ_USER + 26:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler27;
			break;
		case IRQ_USER + 27:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler28;
			break;
		case IRQ_USER + 28:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler29;
			break;
		case IRQ_USER + 29:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler30;
			break;
		case IRQ_USER + 30:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler31;
			break;
		case IRQ_USER + 31:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler32;
			break;
		case IRQ_USER + 32:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler33;
			break;
		case IRQ_USER + 33:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler34;
			break;
		case IRQ_USER + 34:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler35;
			break;
		case IRQ_USER + 35:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler36;
			break;
		case IRQ_USER + 36:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler37;
			break;
		case IRQ_USER + 37:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler38;
			break;
		case IRQ_USER + 38:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler39;
			break;
		case IRQ_USER + 39:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler40;
			break;
		case IRQ_USER + 40:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler41;
			break;
		case IRQ_USER + 41:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler42;
			break;
		case IRQ_USER + 42:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler43;
			break;
		case IRQ_USER + 43:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler44;
			break;
		case IRQ_USER + 44:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler45;
			break;
		case IRQ_USER + 45:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler46;
			break;
		case IRQ_USER + 46:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler47;
			break;
		case IRQ_USER + 47:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler48;
			break;
		case IRQ_USER + 48:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler49;
			break;
		case IRQ_USER + 49:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler50;
			break;
		case IRQ_USER + 50:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler51;
			break;
		case IRQ_USER + 51:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler52;
			break;
		case IRQ_USER + 52:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler53;
			break;
		case IRQ_USER + 53:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler54;
			break;
		case IRQ_USER + 54:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler55;
			break;
		case IRQ_USER + 55:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler56;
			break;
		case IRQ_USER + 56:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler57;
			break;
		case IRQ_USER + 57:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler58;
			break;
		case IRQ_USER + 58:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler59;
			break;
		case IRQ_USER + 59:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler60;
			break;
		case IRQ_USER + 60:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler61;
			break;
		case IRQ_USER + 61:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler62;
			break;
		case IRQ_USER + 62:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler63;
			break;
		case IRQ_USER + 63:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler64;
			break;
		case IRQ_USER + 64:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler65;
			break;
		case IRQ_USER + 65:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler66;
			break;
		case IRQ_USER + 66:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler67;
			break;
		case IRQ_USER + 67:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler68;
			break;
		case IRQ_USER + 68:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler69;
			break;
		case IRQ_USER + 69:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler70;
			break;
		case IRQ_USER + 70:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler71;
			break;
		case IRQ_USER + 71:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler72;
			break;
		case IRQ_USER + 72:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler73;
			break;
		case IRQ_USER + 73:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler74;
			break;
		case IRQ_USER + 74:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler75;
			break;
		case IRQ_USER + 75:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler76;
			break;
		case IRQ_USER + 76:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler77;
			break;
		case IRQ_USER + 77:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler78;
			break;
		case IRQ_USER + 78:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler79;
			break;
		case IRQ_USER + 79:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler80;
			break;
		case IRQ_USER + 80:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler81;
			break;
		case IRQ_USER + 81:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler82;
			break;
		case IRQ_USER + 82:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler83;
			break;
		case IRQ_USER + 83:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler84;
			break;
		case IRQ_USER + 84:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler85;
			break;
		case IRQ_USER + 85:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler86;
			break;
		case IRQ_USER + 86:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler87;
			break;
		case IRQ_USER + 87:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler88;
			break;
		case IRQ_USER + 88:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler89;
			break;
		case IRQ_USER + 89:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler90;
			break;
		case IRQ_USER + 90:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler91;
			break;
		case IRQ_USER + 91:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler92;
			break;
		case IRQ_USER + 92:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler93;
			break;
		case IRQ_USER + 93:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler94;
			break;
		case IRQ_USER + 94:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler95;
			break;
		case IRQ_USER + 95:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler96;
			break;
		case IRQ_USER + 96:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler97;
			break;
		case IRQ_USER + 97:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler98;
			break;
		case IRQ_USER + 98:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler99;
			break;
		case IRQ_USER + 99:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler100;
			break;
		case IRQ_USER + 100:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler101;
			break;
		case IRQ_USER + 101:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler102;
			break;
		case IRQ_USER + 102:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler103;
			break;
		case IRQ_USER + 103:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler104;
			break;
		case IRQ_USER + 104:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler105;
			break;
		case IRQ_USER + 105:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler106;
			break;
		case IRQ_USER + 106:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler107;
			break;
		case IRQ_USER + 107:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler108;
			break;
		case IRQ_USER + 108:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler109;
			break;
		case IRQ_USER + 109:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler110;
			break;
		case IRQ_USER + 110:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler111;
			break;
		case IRQ_USER + 111:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler112;
			break;
		case IRQ_USER + 112:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler113;
			break;
		case IRQ_USER + 113:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler114;
			break;
		case IRQ_USER + 114:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler115;
			break;
		case IRQ_USER + 115:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler116;
			break;
		case IRQ_USER + 116:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler117;
			break;
		case IRQ_USER + 117:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler118;
			break;
		case IRQ_USER + 118:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler119;
			break;
		case IRQ_USER + 119:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler120;
			break;
		case IRQ_USER + 120:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler121;
			break;
		case IRQ_USER + 121:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler122;
			break;
		case IRQ_USER + 122:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler123;
			break;
		case IRQ_USER + 123:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler124;
			break;
		case IRQ_USER + 124:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler125;
			break;
		case IRQ_USER + 125:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler126;
			break;
		case IRQ_USER + 126:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler127;
			break;
		case IRQ_USER + 127:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler128;
			break;
		case IRQ_USER + 128:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler129;
			break;
		case IRQ_USER + 129:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler130;
			break;
		case IRQ_USER + 130:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler131;
			break;
		case IRQ_USER + 131:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler132;
			break;
		case IRQ_USER + 132:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler133;
			break;
		case IRQ_USER + 133:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler134;
			break;
		case IRQ_USER + 134:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler135;
			break;
		case IRQ_USER + 135:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler136;
			break;
		case IRQ_USER + 136:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler137;
			break;
		case IRQ_USER + 137:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler138;
			break;
		case IRQ_USER + 138:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler139;
			break;
		case IRQ_USER + 139:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler140;
			break;
		case IRQ_USER + 140:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler141;
			break;
		case IRQ_USER + 141:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler142;
			break;
		case IRQ_USER + 142:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler143;
			break;
		case IRQ_USER + 143:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler144;
			break;
		case IRQ_USER + 144:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler145;
			break;
		case IRQ_USER + 145:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler146;
			break;
		case IRQ_USER + 146:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler147;
			break;
		case IRQ_USER + 147:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler148;
			break;
		case IRQ_USER + 148:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler149;
			break;
		case IRQ_USER + 149:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler150;
			break;
		case IRQ_USER + 150:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler151;
			break;
		case IRQ_USER + 151:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler152;
			break;
		case IRQ_USER + 152:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler153;
			break;
		case IRQ_USER + 153:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler154;
			break;
		case IRQ_USER + 154:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler155;
			break;
		case IRQ_USER + 155:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler156;
			break;
		case IRQ_USER + 156:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler157;
			break;
		case IRQ_USER + 157:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler158;
			break;
		case IRQ_USER + 158:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler159;
			break;
		case IRQ_USER + 159:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler160;
			break;
		case IRQ_USER + 160:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler161;
			break;
		case IRQ_USER + 161:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler162;
			break;
		case IRQ_USER + 162:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler163;
			break;
		case IRQ_USER + 163:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler164;
			break;
		case IRQ_USER + 164:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler165;
			break;
		case IRQ_USER + 165:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler166;
			break;
		case IRQ_USER + 166:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler167;
			break;
		case IRQ_USER + 167:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler168;
			break;
		case IRQ_USER + 168:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler169;
			break;
		case IRQ_USER + 169:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler170;
			break;
		case IRQ_USER + 170:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler171;
			break;
		case IRQ_USER + 171:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler172;
			break;
		case IRQ_USER + 172:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler173;
			break;
		case IRQ_USER + 173:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler174;
			break;
		case IRQ_USER + 174:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler175;
			break;
		case IRQ_USER + 175:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler176;
			break;
		case IRQ_USER + 176:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler177;
			break;
		case IRQ_USER + 177:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler178;
			break;
		case IRQ_USER + 178:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler179;
			break;
		case IRQ_USER + 179:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler180;
			break;
		case IRQ_USER + 180:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler181;
			break;
		case IRQ_USER + 181:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler182;
			break;
		case IRQ_USER + 182:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler183;
			break;
		case IRQ_USER + 183:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler184;
			break;
		case IRQ_USER + 184:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler185;
			break;
		case IRQ_USER + 185:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler186;
			break;
		case IRQ_USER + 186:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler187;
			break;
		case IRQ_USER + 187:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler188;
			break;
		case IRQ_USER + 188:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler189;
			break;
		case IRQ_USER + 189:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler190;
			break;
		case IRQ_USER + 190:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler191;
			break;
		case IRQ_USER + 191:
			vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler192;
			break;
	}
#else
		vectors[m68k_first_user_vec + irq - IRQ_USER] = user_inthandler;
#endif
	return 0;
}

unsigned int m68k_irq_startup(struct irq_data *data)
{
	return m68k_irq_startup_irq(data->irq);
}

void m68k_irq_shutdown(struct irq_data *data)
{
	unsigned int irq = data->irq;

	if (irq <= IRQ_AUTO_7)
		vectors[VEC_SPUR + irq] = bad_inthandler;
	else
		vectors[m68k_first_user_vec + irq - IRQ_USER] = bad_inthandler;
}


unsigned int irq_canonicalize(unsigned int irq)
{
#ifdef CONFIG_Q40
	if (MACH_IS_Q40 && irq == 11)
		irq = 10;
#endif
	return irq;
}

EXPORT_SYMBOL(irq_canonicalize);


asmlinkage void handle_badint(struct pt_regs *regs)
{
	atomic_inc(&irq_err_count);
	pr_warn("unexpected interrupt from %u\n", regs->vector);
}
