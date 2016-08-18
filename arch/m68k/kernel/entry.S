/* -*- mode: asm -*-
 *
 *  linux/arch/m68k/kernel/entry.S
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file README.legal in the main directory of this archive
 * for more details.
 *
 * Linux/m68k support by Hamish Macdonald
 *
 * 68060 fixes by Jesper Skov
 *
 */

/*
 * entry.S  contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 */

/*
 * 12/03/96 Jes: Currently we only support m68k single-cpu systems, so
 *               all pointers that used to be 'current' are now entry
 *               number 0 in the 'current_set' list.
 *
 *  6/05/00 RZ:	 addedd writeback completion after return from sighandler
 *		 for 68040
 */

#include <linux/linkage.h>
#include <asm/errno.h>
#include <asm/setup.h>
#include <asm/segment.h>
#include <asm/traps.h>
#include <asm/unistd.h>
#include <asm/asm-offsets.h>
#include <asm/entry.h>

.globl system_call, buserr, trap, resume
.globl sys_call_table
.globl __sys_fork, __sys_clone, __sys_vfork
.globl bad_interrupt
.globl auto_irqhandler_fixup
.globl user_irqvec_fixup
#ifdef CONFIG_M68000
.globl auto_inthandler1, auto_inthandler2, auto_inthandler3, auto_inthandler4,\
	 	auto_inthandler5, auto_inthandler6, auto_inthandler7
#endif

.text
ENTRY(__sys_fork)
	SAVE_SWITCH_STACK
	jbsr	sys_fork
	lea     %sp@(24),%sp
	rts

ENTRY(__sys_clone)
	SAVE_SWITCH_STACK
	pea	%sp@(SWITCH_STACK_SIZE)
	jbsr	m68k_clone
	lea     %sp@(28),%sp
	rts

ENTRY(__sys_vfork)
	SAVE_SWITCH_STACK
	jbsr	sys_vfork
	lea     %sp@(24),%sp
	rts

ENTRY(sys_sigreturn)
	SAVE_SWITCH_STACK
	movel	%sp,%sp@-		  | switch_stack pointer
	pea	%sp@(SWITCH_STACK_SIZE+4) | pt_regs pointer
	jbsr	do_sigreturn
	addql	#8,%sp
	RESTORE_SWITCH_STACK
	rts

ENTRY(sys_rt_sigreturn)
	SAVE_SWITCH_STACK
	movel	%sp,%sp@-		  | switch_stack pointer
	pea	%sp@(SWITCH_STACK_SIZE+4) | pt_regs pointer
	jbsr	do_rt_sigreturn
	addql	#8,%sp
	RESTORE_SWITCH_STACK
	rts

ENTRY(buserr)
	SAVE_ALL_INT
	GET_CURRENT(%d0)
	movel	%sp,%sp@-		| stack frame pointer argument
	jbsr	buserr_c
	addql	#4,%sp
	jra	ret_from_exception

ENTRY(trap)
	SAVE_ALL_INT
	GET_CURRENT(%d0)
	movel	%sp,%sp@-		| stack frame pointer argument
	jbsr	trap_c
	addql	#4,%sp
	jra	ret_from_exception

	| After a fork we jump here directly from resume,
	| so that %d1 contains the previous task
	| schedule_tail now used regardless of CONFIG_SMP
ENTRY(ret_from_fork)
	movel	%d1,%sp@-
	jsr	schedule_tail
	addql	#4,%sp
	jra	ret_from_exception

ENTRY(ret_from_kernel_thread)
	| a3 contains the kernel thread payload, d7 - its argument
	movel	%d1,%sp@-
	jsr	schedule_tail
	movel	%d7,(%sp)
	jsr	%a3@
	addql	#4,%sp
	jra	ret_from_exception

#if defined(CONFIG_COLDFIRE) || !defined(CONFIG_MMU)

#ifdef TRAP_DBG_INTERRUPT

.globl dbginterrupt
ENTRY(dbginterrupt)
	SAVE_ALL_INT
	GET_CURRENT(%d0)
	movel	%sp,%sp@- 		/* stack frame pointer argument */
	jsr	dbginterrupt_c
	addql	#4,%sp
	jra	ret_from_exception
#endif

ENTRY(reschedule)
	/* save top of frame */
	pea	%sp@
	jbsr	set_esp0
	addql	#4,%sp
	pea	ret_from_exception
	jmp	schedule

ENTRY(ret_from_user_signal)
	moveq #__NR_sigreturn,%d0
	trap #0

ENTRY(ret_from_user_rt_signal)
	movel #__NR_rt_sigreturn,%d0
	trap #0

//FIXME
ENTRY(ret_from_exception)
.Lret_from_exception:

#ifdef CONFIG_M68000
	  /* The original 68000 CPU does not store the vector number
	   * on the stack, so we have to do this manually 
	   *
	   * Geert:  
	   * - The 68000 doesn't have a VBR (Vector Base Register), so the exception 
	   *    table is always at address zero, 
	   * - The 68000 doesn't store the format and vector number in the stack
	   *    frame automatically, so we have to use a different routine for each
	   *   vector we want to be able to identify uniquely,
	   * - The 68000 uses a different exception stack frame for bus and address 
	   *    errors.
	   */
ENTRY(auto_inthandler1)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT1*4,%sp@- 		/*  put vector # on stack*/
auto_irqhandler_fixup = . + 2
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(auto_inthandler2)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT2*4,%sp@- 		/*  put vector # on stack*/

	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(auto_inthandler3)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT3*4,%sp@- 		/*  put vector # on stack*/

	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(auto_inthandler4)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT4*4,%sp@- 		/*  put vector # on stack*/

	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(auto_inthandler5)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT5*4,%sp@- 		/*  put vector # on stack*/

	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(auto_inthandler6)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT6*4,%sp@- 		/*  put vector # on stack*/

	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(auto_inthandler7)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#VEC_INT7*4,%sp@- 		/*  put vector # on stack*/

	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

ENTRY(user_inthandler1)
	SAVE_ALL_INT
	user_irqvec_fixup = . + 2 /* FIXME_Matthias: not sure if it is enough to have
							   * this only at this point or if all other
							   * user_inthandlers need that as well
							   */
	movel	%sp,%sp@-
	movel	#(VEC_USER+0)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler2)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+1)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler3)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+2)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler4)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+3)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler5)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+4)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler6)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+5)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler7)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+6)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler8)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+7)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler9)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+8)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler10)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+9)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler11)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+10)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler12)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+11)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler13)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+12)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler14)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+13)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler15)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+14)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler16)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+15)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler17)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+16)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler18)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+17)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler19)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+18)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler20)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+19)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler21)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+20)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler22)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+21)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler23)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+22)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler24)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+23)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler25)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+24)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler26)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+25)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler27)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+26)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler28)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+27)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler29)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+28)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler30)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+29)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler31)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+30)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler32)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+31)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler33)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+32)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler34)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+33)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler35)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+34)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler36)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+35)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler37)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+36)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler38)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+37)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler39)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+38)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler40)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+39)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler41)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+40)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler42)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+41)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler43)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+42)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler44)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+43)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler45)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+44)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler46)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+45)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler47)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+46)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler48)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+47)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler49)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+48)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler50)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+49)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler51)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+50)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler52)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+51)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler53)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+52)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler54)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+53)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler55)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+54)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler56)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+55)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler57)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+56)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler58)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+57)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler59)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+58)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler60)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+59)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler61)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+60)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler62)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+61)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler63)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+62)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler64)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+63)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler65)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+64)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler66)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+65)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler67)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+66)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler68)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+67)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler69)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+68)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler70)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+69)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler71)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+70)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler72)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+71)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler73)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+72)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler74)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+73)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler75)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+74)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler76)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+75)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler77)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+76)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler78)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+77)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler79)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+78)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler80)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+79)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler81)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+80)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler82)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+81)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler83)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+82)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler84)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+83)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler85)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+84)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler86)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+85)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler87)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+86)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler88)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+87)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler89)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+88)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler90)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+89)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler91)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+90)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler92)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+91)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler93)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+92)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler94)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+93)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler95)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+94)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler96)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+95)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler97)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+96)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler98)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+97)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler99)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+98)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler100)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+99)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler101)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+100)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler102)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+101)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler103)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+102)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler104)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+103)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler105)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+104)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler106)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+105)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler107)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+106)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler108)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+107)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler109)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+108)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler110)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+109)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler111)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+110)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler112)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+111)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler113)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+112)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler114)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+113)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler115)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+114)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler116)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+115)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler117)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+116)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler118)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+117)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler119)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+118)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler120)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+119)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler121)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+120)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler122)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+121)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler123)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+122)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler124)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+123)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler125)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+124)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler126)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+125)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler127)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+126)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler128)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+127)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler129)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+128)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler130)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+129)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler131)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+130)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler132)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+131)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler133)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+132)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler134)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+133)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler135)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+134)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler136)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+135)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler137)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+136)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler138)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+137)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler139)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+138)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler140)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+139)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler141)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+140)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler142)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+141)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler143)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+142)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler144)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+143)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler145)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+144)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler146)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+145)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler147)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+146)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler148)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+147)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler149)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+148)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler150)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+149)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler151)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+150)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler152)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+151)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler153)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+152)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler154)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+153)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler155)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+154)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler156)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+155)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler157)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+156)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler158)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+157)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler159)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+158)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler160)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+159)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler161)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+160)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler162)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+161)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler163)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+162)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler164)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+163)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler165)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+164)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler166)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+165)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler167)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+166)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler168)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+167)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler169)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+168)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler170)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+169)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler171)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+170)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler172)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+171)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler173)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+172)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler174)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+173)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler175)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+174)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler176)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+175)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler177)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+176)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler178)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+177)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler179)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+178)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler180)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+179)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler181)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+180)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler182)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+181)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler183)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+182)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler184)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+183)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler185)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+184)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler186)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+185)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler187)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+186)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler188)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+187)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler189)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+188)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler190)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+189)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler191)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+190)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
ENTRY(user_inthandler192)
	SAVE_ALL_INT
	movel	%sp,%sp@-
	movel	#(VEC_USER+191)*4,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception
	
	
/*  copied from arch/m68k/68000/entry.S  */
badsys:
	movel	#-ENOSYS,%sp@(PT_OFF_D0)
	jra	ret_from_exception

do_trace:
	movel	#-ENOSYS,%sp@(PT_OFF_D0) /* needed for strace*/
	subql	#4,%sp
	SAVE_SWITCH_STACK
	jbsr	syscall_trace_enter
	RESTORE_SWITCH_STACK
	addql	#4,%sp
	movel	%sp@(PT_OFF_ORIG_D0),%d1
	movel	#-ENOSYS,%d0
	cmpl	#NR_syscalls,%d1
	jcc	1f
	lsl	#2,%d1
	lea	sys_call_table, %a0
	jbsr	%a0@(%d1)

1:	movel	%d0,%sp@(PT_OFF_D0)	/* save the return value */
	subql	#4,%sp			/* dummy return address */
	SAVE_SWITCH_STACK
	jbsr	syscall_trace_leave

ret_from_signal:
	RESTORE_SWITCH_STACK
	addql	#4,%sp
	jra	ret_from_exception


ENTRY(system_call)
	SAVE_ALL_SYS

	/* save top of frame*/
	pea	%sp@
	jbsr	set_esp0
	addql	#4,%sp

	movel	%sp@(PT_OFF_ORIG_D0),%d0

	movel	%sp,%d1			/* get thread_info pointer */
	andl	#-THREAD_SIZE,%d1
	movel	%d1,%a2
	btst	#(TIF_SYSCALL_TRACE%8),%a2@(TINFO_FLAGS+(31-TIF_SYSCALL_TRACE)/8)
	jne	do_trace
	cmpl	#NR_syscalls,%d0
	jcc	badsys
	lsl	#2,%d0
	lea	sys_call_table,%a0
	movel	%a0@(%d0), %a0
	jbsr	%a0@
	movel	%d0,%sp@(PT_OFF_D0)	/* save the return value*/
	
/*
 * Beware - when entering resume, prev (the current task) is
 * in a0, next (the new task) is in a1, so don't change these
 * registers until their contents are no longer needed.
 */
ENTRY(resume)
	movel	%a0,%d1				/* save prev thread in d1 */
	movew	%sr,%a0@(TASK_THREAD+THREAD_SR)	/* save sr */
	SAVE_SWITCH_STACK
	movel	%sp,%a0@(TASK_THREAD+THREAD_KSP) /* save kernel stack */
	movel	%usp,%a3			/* save usp */
	movel	%a3,%a0@(TASK_THREAD+THREAD_USP)

	movel	%a1@(TASK_THREAD+THREAD_USP),%a3 /* restore user stack */
	movel	%a3,%usp
	movel	%a1@(TASK_THREAD+THREAD_KSP),%sp /* restore new thread stack */
	RESTORE_SWITCH_STACK
	movew	%a1@(TASK_THREAD+THREAD_SR),%sr	/* restore thread status reg */
	rts	
/* end copy from arch/m68k/68000/entry.S  */

/* FIXME_Matthias: not sure if this works, because the vector number is 
       possibly missing on 68000 */

ENTRY(bad_inthandler)
	SAVE_ALL_INT
/* FIXME_Matthias: this is a _bad_ hack, find better solution */
	movel	#255,%d0
	movel	%sp,%sp@-
	jsr	handle_badint
	addql	#4,%sp
	jra	ret_from_exception

#endif


#else

do_trace_entry:
	movel	#-ENOSYS,%sp@(PT_OFF_D0)| needed for strace
	subql	#4,%sp
	SAVE_SWITCH_STACK
	jbsr	syscall_trace
	RESTORE_SWITCH_STACK
	addql	#4,%sp
	movel	%sp@(PT_OFF_ORIG_D0),%d0
	cmpl	#NR_syscalls,%d0
	jcs	syscall
badsys:
	movel	#-ENOSYS,%sp@(PT_OFF_D0)
	jra	ret_from_syscall

do_trace_exit:
	subql	#4,%sp
	SAVE_SWITCH_STACK
	jbsr	syscall_trace
	RESTORE_SWITCH_STACK
	addql	#4,%sp
	jra	.Lret_from_exception

ENTRY(ret_from_signal)
	movel	%curptr@(TASK_STACK),%a1
	tstb	%a1@(TINFO_FLAGS+2)
	jge	1f
	jbsr	syscall_trace
1:	RESTORE_SWITCH_STACK
	addql	#4,%sp
/* on 68040 complete pending writebacks if any */
#ifdef CONFIG_M68040
	bfextu	%sp@(PT_OFF_FORMATVEC){#0,#4},%d0
	subql	#7,%d0				| bus error frame ?
	jbne	1f
	movel	%sp,%sp@-
	jbsr	berr_040cleanup
	addql	#4,%sp
1:
#endif
	jra	.Lret_from_exception

ENTRY(system_call)
	SAVE_ALL_SYS

	GET_CURRENT(%d1)
	movel	%d1,%a1

	| save top of frame
	movel	%sp,%curptr@(TASK_THREAD+THREAD_ESP0)

	| syscall trace?
	tstb	%a1@(TINFO_FLAGS+2)
	jmi	do_trace_entry
	cmpl	#NR_syscalls,%d0
	jcc	badsys
syscall:
	jbsr	@(sys_call_table,%d0:l:4)@(0)
	movel	%d0,%sp@(PT_OFF_D0)	| save the return value
ret_from_syscall:
	|oriw	#0x0700,%sr
	movel	%curptr@(TASK_STACK),%a1
	movew	%a1@(TINFO_FLAGS+2),%d0
	jne	syscall_exit_work
1:	RESTORE_ALL

syscall_exit_work:
	btst	#5,%sp@(PT_OFF_SR)	| check if returning to kernel
	bnes	1b			| if so, skip resched, signals
	lslw	#1,%d0
	jcs	do_trace_exit
	jmi	do_delayed_trace
	lslw	#8,%d0
	jne	do_signal_return
	pea	resume_userspace
	jra	schedule


ENTRY(ret_from_exception)
.Lret_from_exception:
	btst	#5,%sp@(PT_OFF_SR)	| check if returning to kernel
	bnes	1f			| if so, skip resched, signals
	| only allow interrupts when we are really the last one on the
	| kernel stack, otherwise stack overflow can occur during
	| heavy interrupt load
	andw	#ALLOWINT,%sr

resume_userspace:
	movel	%curptr@(TASK_STACK),%a1
	moveb	%a1@(TINFO_FLAGS+3),%d0
	jne	exit_work
1:	RESTORE_ALL

exit_work:
	| save top of frame
	movel	%sp,%curptr@(TASK_THREAD+THREAD_ESP0)
	lslb	#1,%d0
	jne	do_signal_return
	pea	resume_userspace
	jra	schedule


do_signal_return:
	|andw	#ALLOWINT,%sr
	subql	#4,%sp			| dummy return address
	SAVE_SWITCH_STACK
	pea	%sp@(SWITCH_STACK_SIZE)
	bsrl	do_notify_resume
	addql	#4,%sp
	RESTORE_SWITCH_STACK
	addql	#4,%sp
	jbra	resume_userspace

do_delayed_trace:
	bclr	#7,%sp@(PT_OFF_SR)	| clear trace bit in SR
	pea	1			| send SIGTRAP
	movel	%curptr,%sp@-
	pea	LSIGTRAP
	jbsr	send_sig
	addql	#8,%sp
	addql	#4,%sp
	jbra	resume_userspace

/* This is the main interrupt handler for autovector interrupts */
ENTRY(auto_inthandler)
	SAVE_ALL_INT
	GET_CURRENT(%d0)
					|  put exception # in d0
	bfextu	%sp@(PT_OFF_FORMATVEC){#4,#10},%d0
	subw	#VEC_SPUR,%d0

	movel	%sp,%sp@-
	movel	%d0,%sp@-		|  put vector # on stack
auto_irqhandler_fixup = . + 2
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception

/* Handler for user defined interrupt vectors */

ENTRY(user_inthandler)
	SAVE_ALL_INT
	GET_CURRENT(%d0)
					|  put exception # in d0
	bfextu	%sp@(PT_OFF_FORMATVEC){#4,#10},%d0
user_irqvec_fixup = . + 2
	subw	#VEC_USER,%d0

	movel	%sp,%sp@-
	movel	%d0,%sp@-		|  put vector # on stack
	jsr	do_IRQ			|  process the IRQ
	addql	#8,%sp			|  pop parameters off stack
	jra	ret_from_exception


/* Handler for uninitialized and spurious interrupts */

ENTRY(bad_inthandler)
	SAVE_ALL_INT
	GET_CURRENT(%d0)

	movel	%sp,%sp@-
	jsr	handle_badint
	addql	#4,%sp
	jra	ret_from_exception

resume:
	/*
	 * Beware - when entering resume, prev (the current task) is
	 * in a0, next (the new task) is in a1,so don't change these
	 * registers until their contents are no longer needed.
	 */

	/* save sr */
	movew	%sr,%a0@(TASK_THREAD+THREAD_SR)

	/* save fs (sfc,%dfc) (may be pointing to kernel memory) */
	movec	%sfc,%d0
	movew	%d0,%a0@(TASK_THREAD+THREAD_FS)

	/* save usp */
	/* it is better to use a movel here instead of a movew 8*) */
	movec	%usp,%d0
	movel	%d0,%a0@(TASK_THREAD+THREAD_USP)

	/* save non-scratch registers on stack */
	SAVE_SWITCH_STACK

	/* save current kernel stack pointer */
	movel	%sp,%a0@(TASK_THREAD+THREAD_KSP)

	/* save floating point context */
#ifndef CONFIG_M68KFPU_EMU_ONLY
#ifdef CONFIG_M68KFPU_EMU
	tstl	m68k_fputype
	jeq	3f
#endif
	fsave	%a0@(TASK_THREAD+THREAD_FPSTATE)

#if defined(CONFIG_M68060)
#if !defined(CPU_M68060_ONLY)
	btst	#3,m68k_cputype+3
	beqs	1f
#endif
	/* The 060 FPU keeps status in bits 15-8 of the first longword */
	tstb	%a0@(TASK_THREAD+THREAD_FPSTATE+2)
	jeq	3f
#if !defined(CPU_M68060_ONLY)
	jra	2f
#endif
#endif /* CONFIG_M68060 */
#if !defined(CPU_M68060_ONLY)
1:	tstb	%a0@(TASK_THREAD+THREAD_FPSTATE)
	jeq	3f
#endif
2:	fmovemx	%fp0-%fp7,%a0@(TASK_THREAD+THREAD_FPREG)
	fmoveml	%fpcr/%fpsr/%fpiar,%a0@(TASK_THREAD+THREAD_FPCNTL)
3:
#endif	/* CONFIG_M68KFPU_EMU_ONLY */
	/* Return previous task in %d1 */
	movel	%curptr,%d1

	/* switch to new task (a1 contains new task) */
	movel	%a1,%curptr

	/* restore floating point context */
#ifndef CONFIG_M68KFPU_EMU_ONLY
#ifdef CONFIG_M68KFPU_EMU
	tstl	m68k_fputype
	jeq	4f
#endif
#if defined(CONFIG_M68060)
#if !defined(CPU_M68060_ONLY)
	btst	#3,m68k_cputype+3
	beqs	1f
#endif
	/* The 060 FPU keeps status in bits 15-8 of the first longword */
	tstb	%a1@(TASK_THREAD+THREAD_FPSTATE+2)
	jeq	3f
#if !defined(CPU_M68060_ONLY)
	jra	2f
#endif
#endif /* CONFIG_M68060 */
#if !defined(CPU_M68060_ONLY)
1:	tstb	%a1@(TASK_THREAD+THREAD_FPSTATE)
	jeq	3f
#endif
2:	fmovemx	%a1@(TASK_THREAD+THREAD_FPREG),%fp0-%fp7
	fmoveml	%a1@(TASK_THREAD+THREAD_FPCNTL),%fpcr/%fpsr/%fpiar
3:	frestore %a1@(TASK_THREAD+THREAD_FPSTATE)
4:
#endif	/* CONFIG_M68KFPU_EMU_ONLY */

	/* restore the kernel stack pointer */
	movel	%a1@(TASK_THREAD+THREAD_KSP),%sp

	/* restore non-scratch registers */
	RESTORE_SWITCH_STACK

	/* restore user stack pointer */
	movel	%a1@(TASK_THREAD+THREAD_USP),%a0
	movel	%a0,%usp

	/* restore fs (sfc,%dfc) */
	movew	%a1@(TASK_THREAD+THREAD_FS),%a0
	movec	%a0,%sfc
	movec	%a0,%dfc

	/* restore status register */
	movew	%a1@(TASK_THREAD+THREAD_SR),%sr

	rts

#endif /* CONFIG_MMU && !CONFIG_COLDFIRE */
