/*
 *  linux/include/asm/traps.h
 *
 *  Copyright (C) 1993        Hamish Macdonald
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#ifndef _M68K_TRAPS_H
#define _M68K_TRAPS_H

#ifndef __ASSEMBLY__

#include <linux/linkage.h>
#include <asm/ptrace.h>

typedef void (*e_vector)(void);
extern e_vector *_ramvec;

#ifdef CONFIG_M68000
#define vectors		((e_vector *)0)
/* assembler routines */
asmlinkage void system_call(void);
asmlinkage void buserr(void);
asmlinkage void trap(void);
asmlinkage void trap3(void);
asmlinkage void trap4(void);
asmlinkage void trap5(void);
asmlinkage void trap6(void);
asmlinkage void trap7(void);
asmlinkage void trap8(void);
asmlinkage void trap9(void);
asmlinkage void trap10(void);
asmlinkage void trap11(void);
asmlinkage void trap12(void);
asmlinkage void trap13(void);
asmlinkage void trap14(void);
asmlinkage void trap15(void);
asmlinkage void trap33(void);
asmlinkage void trap34(void);
asmlinkage void trap35(void);
asmlinkage void trap36(void);
asmlinkage void trap37(void);
asmlinkage void trap38(void);
asmlinkage void trap39(void);
asmlinkage void trap40(void);
asmlinkage void trap41(void);
asmlinkage void trap42(void);
asmlinkage void trap43(void);
asmlinkage void trap44(void);
asmlinkage void trap45(void);
asmlinkage void trap46(void);
asmlinkage void trap47(void);
asmlinkage void auto_inthandler1(void);
asmlinkage void auto_inthandler2(void);
asmlinkage void auto_inthandler3(void);
asmlinkage void auto_inthandler4(void);
asmlinkage void auto_inthandler5(void);
asmlinkage void auto_inthandler6(void);
asmlinkage void auto_inthandler7(void);
asmlinkage void user_inthandler1(void);
asmlinkage void user_inthandler2(void);
asmlinkage void user_inthandler3(void);
asmlinkage void user_inthandler4(void);
asmlinkage void user_inthandler5(void);
asmlinkage void user_inthandler6(void);
asmlinkage void user_inthandler7(void);
asmlinkage void user_inthandler8(void);
asmlinkage void user_inthandler9(void);
asmlinkage void user_inthandler10(void);
asmlinkage void user_inthandler11(void);
asmlinkage void user_inthandler12(void);
asmlinkage void user_inthandler13(void);
asmlinkage void user_inthandler14(void);
asmlinkage void user_inthandler15(void);
asmlinkage void user_inthandler16(void);
asmlinkage void user_inthandler17(void);
asmlinkage void user_inthandler18(void);
asmlinkage void user_inthandler19(void);
asmlinkage void user_inthandler20(void);
asmlinkage void user_inthandler21(void);
asmlinkage void user_inthandler22(void);
asmlinkage void user_inthandler23(void);
asmlinkage void user_inthandler24(void);
asmlinkage void user_inthandler25(void);
asmlinkage void user_inthandler26(void);
asmlinkage void user_inthandler27(void);
asmlinkage void user_inthandler28(void);
asmlinkage void user_inthandler29(void);
asmlinkage void user_inthandler30(void);
asmlinkage void user_inthandler31(void);
asmlinkage void user_inthandler32(void);
asmlinkage void user_inthandler33(void);
asmlinkage void user_inthandler34(void);
asmlinkage void user_inthandler35(void);
asmlinkage void user_inthandler36(void);
asmlinkage void user_inthandler37(void);
asmlinkage void user_inthandler38(void);
asmlinkage void user_inthandler39(void);
asmlinkage void user_inthandler40(void);
asmlinkage void user_inthandler41(void);
asmlinkage void user_inthandler42(void);
asmlinkage void user_inthandler43(void);
asmlinkage void user_inthandler44(void);
asmlinkage void user_inthandler45(void);
asmlinkage void user_inthandler46(void);
asmlinkage void user_inthandler47(void);
asmlinkage void user_inthandler48(void);
asmlinkage void user_inthandler49(void);
asmlinkage void user_inthandler50(void);
asmlinkage void user_inthandler51(void);
asmlinkage void user_inthandler52(void);
asmlinkage void user_inthandler53(void);
asmlinkage void user_inthandler54(void);
asmlinkage void user_inthandler55(void);
asmlinkage void user_inthandler56(void);
asmlinkage void user_inthandler57(void);
asmlinkage void user_inthandler58(void);
asmlinkage void user_inthandler59(void);
asmlinkage void user_inthandler60(void);
asmlinkage void user_inthandler61(void);
asmlinkage void user_inthandler62(void);
asmlinkage void user_inthandler63(void);
asmlinkage void user_inthandler64(void);
asmlinkage void user_inthandler65(void);
asmlinkage void user_inthandler66(void);
asmlinkage void user_inthandler67(void);
asmlinkage void user_inthandler68(void);
asmlinkage void user_inthandler69(void);
asmlinkage void user_inthandler70(void);
asmlinkage void user_inthandler71(void);
asmlinkage void user_inthandler72(void);
asmlinkage void user_inthandler73(void);
asmlinkage void user_inthandler74(void);
asmlinkage void user_inthandler75(void);
asmlinkage void user_inthandler76(void);
asmlinkage void user_inthandler77(void);
asmlinkage void user_inthandler78(void);
asmlinkage void user_inthandler79(void);
asmlinkage void user_inthandler80(void);
asmlinkage void user_inthandler81(void);
asmlinkage void user_inthandler82(void);
asmlinkage void user_inthandler83(void);
asmlinkage void user_inthandler84(void);
asmlinkage void user_inthandler85(void);
asmlinkage void user_inthandler86(void);
asmlinkage void user_inthandler87(void);
asmlinkage void user_inthandler88(void);
asmlinkage void user_inthandler89(void);
asmlinkage void user_inthandler90(void);
asmlinkage void user_inthandler91(void);
asmlinkage void user_inthandler92(void);
asmlinkage void user_inthandler93(void);
asmlinkage void user_inthandler94(void);
asmlinkage void user_inthandler95(void);
asmlinkage void user_inthandler96(void);
asmlinkage void user_inthandler97(void);
asmlinkage void user_inthandler98(void);
asmlinkage void user_inthandler99(void);
asmlinkage void user_inthandler100(void);
asmlinkage void user_inthandler101(void);
asmlinkage void user_inthandler102(void);
asmlinkage void user_inthandler103(void);
asmlinkage void user_inthandler104(void);
asmlinkage void user_inthandler105(void);
asmlinkage void user_inthandler106(void);
asmlinkage void user_inthandler107(void);
asmlinkage void user_inthandler108(void);
asmlinkage void user_inthandler109(void);
asmlinkage void user_inthandler110(void);
asmlinkage void user_inthandler111(void);
asmlinkage void user_inthandler112(void);
asmlinkage void user_inthandler113(void);
asmlinkage void user_inthandler114(void);
asmlinkage void user_inthandler115(void);
asmlinkage void user_inthandler116(void);
asmlinkage void user_inthandler117(void);
asmlinkage void user_inthandler118(void);
asmlinkage void user_inthandler119(void);
asmlinkage void user_inthandler120(void);
asmlinkage void user_inthandler121(void);
asmlinkage void user_inthandler122(void);
asmlinkage void user_inthandler123(void);
asmlinkage void user_inthandler124(void);
asmlinkage void user_inthandler125(void);
asmlinkage void user_inthandler126(void);
asmlinkage void user_inthandler127(void);
asmlinkage void user_inthandler128(void);
asmlinkage void user_inthandler129(void);
asmlinkage void user_inthandler130(void);
asmlinkage void user_inthandler131(void);
asmlinkage void user_inthandler132(void);
asmlinkage void user_inthandler133(void);
asmlinkage void user_inthandler134(void);
asmlinkage void user_inthandler135(void);
asmlinkage void user_inthandler136(void);
asmlinkage void user_inthandler137(void);
asmlinkage void user_inthandler138(void);
asmlinkage void user_inthandler139(void);
asmlinkage void user_inthandler140(void);
asmlinkage void user_inthandler141(void);
asmlinkage void user_inthandler142(void);
asmlinkage void user_inthandler143(void);
asmlinkage void user_inthandler144(void);
asmlinkage void user_inthandler145(void);
asmlinkage void user_inthandler146(void);
asmlinkage void user_inthandler147(void);
asmlinkage void user_inthandler148(void);
asmlinkage void user_inthandler149(void);
asmlinkage void user_inthandler150(void);
asmlinkage void user_inthandler151(void);
asmlinkage void user_inthandler152(void);
asmlinkage void user_inthandler153(void);
asmlinkage void user_inthandler154(void);
asmlinkage void user_inthandler155(void);
asmlinkage void user_inthandler156(void);
asmlinkage void user_inthandler157(void);
asmlinkage void user_inthandler158(void);
asmlinkage void user_inthandler159(void);
asmlinkage void user_inthandler160(void);
asmlinkage void user_inthandler161(void);
asmlinkage void user_inthandler162(void);
asmlinkage void user_inthandler163(void);
asmlinkage void user_inthandler164(void);
asmlinkage void user_inthandler165(void);
asmlinkage void user_inthandler166(void);
asmlinkage void user_inthandler167(void);
asmlinkage void user_inthandler168(void);
asmlinkage void user_inthandler169(void);
asmlinkage void user_inthandler170(void);
asmlinkage void user_inthandler171(void);
asmlinkage void user_inthandler172(void);
asmlinkage void user_inthandler173(void);
asmlinkage void user_inthandler174(void);
asmlinkage void user_inthandler175(void);
asmlinkage void user_inthandler176(void);
asmlinkage void user_inthandler177(void);
asmlinkage void user_inthandler178(void);
asmlinkage void user_inthandler179(void);
asmlinkage void user_inthandler180(void);
asmlinkage void user_inthandler181(void);
asmlinkage void user_inthandler182(void);
asmlinkage void user_inthandler183(void);
asmlinkage void user_inthandler184(void);
asmlinkage void user_inthandler185(void);
asmlinkage void user_inthandler186(void);
asmlinkage void user_inthandler187(void);
asmlinkage void user_inthandler188(void);
asmlinkage void user_inthandler189(void);
asmlinkage void user_inthandler190(void);
asmlinkage void user_inthandler191(void);
asmlinkage void user_inthandler192(void);
asmlinkage void bad_inthandler(void);
#else /* 68010 or higher */
extern e_vector vectors[];
asmlinkage void auto_inthandler(void);
asmlinkage void user_inthandler(void);
asmlinkage void bad_inthandler(void);
#endif /* 68010 or higher */

#endif

#define VEC_RESETSP (0)
#define VEC_RESETPC (1)
#define VEC_BUSERR  (2)
#define VEC_ADDRERR (3)
#define VEC_ILLEGAL (4)
#define VEC_ZERODIV (5)
#define VEC_CHK     (6)
#define VEC_TRAP    (7)
#define VEC_PRIV    (8)
#define VEC_TRACE   (9)
#define VEC_LINE10  (10)
#define VEC_LINE11  (11)
#define VEC_RESV12  (12)
#define VEC_COPROC  (13)
#define VEC_FORMAT  (14)
#define VEC_UNINT   (15)
#define VEC_RESV16  (16)
#define VEC_RESV17  (17)
#define VEC_RESV18  (18)
#define VEC_RESV19  (19)
#define VEC_RESV20  (20)
#define VEC_RESV21  (21)
#define VEC_RESV22  (22)
#define VEC_RESV23  (23)
#define VEC_SPUR    (24)
#define VEC_INT1    (25)
#define VEC_INT2    (26)
#define VEC_INT3    (27)
#define VEC_INT4    (28)
#define VEC_INT5    (29)
#define VEC_INT6    (30)
#define VEC_INT7    (31)
#define VEC_SYS     (32)
#define VEC_TRAP1   (33)
#define VEC_TRAP2   (34)
#define VEC_TRAP3   (35)
#define VEC_TRAP4   (36)
#define VEC_TRAP5   (37)
#define VEC_TRAP6   (38)
#define VEC_TRAP7   (39)
#define VEC_TRAP8   (40)
#define VEC_TRAP9   (41)
#define VEC_TRAP10  (42)
#define VEC_TRAP11  (43)
#define VEC_TRAP12  (44)
#define VEC_TRAP13  (45)
#define VEC_TRAP14  (46)
#define VEC_TRAP15  (47)
#define VEC_FPBRUC  (48)
#define VEC_FPIR    (49)
#define VEC_FPDIVZ  (50)
#define VEC_FPUNDER (51)
#define VEC_FPOE    (52)
#define VEC_FPOVER  (53)
#define VEC_FPNAN   (54)
#define VEC_FPUNSUP (55)
#define VEC_MMUCFG  (56)
#define VEC_MMUILL  (57)
#define VEC_MMUACC  (58)
#define VEC_RESV59  (59)
#define	VEC_UNIMPEA (60)
#define	VEC_UNIMPII (61)
#define VEC_RESV62  (62)
#define VEC_RESV63  (63)
#define VEC_USER    (64)

#define VECOFF(vec) ((vec)<<2)

#ifndef __ASSEMBLY__

/* Status register bits */
#define PS_T  (0x8000)
#define PS_S  (0x2000)
#define PS_M  (0x1000)
#define PS_C  (0x0001)

/* bits for 68020/68030 special status word */

#define FC    (0x8000)
#define FB    (0x4000)
#define RC    (0x2000)
#define RB    (0x1000)
#define DF    (0x0100)
#define RM    (0x0080)
#define RW    (0x0040)
#define SZ    (0x0030)
#define DFC   (0x0007)

/* bits for 68030 MMU status register (mmusr,psr) */

#define MMU_B	     (0x8000)    /* bus error */
#define MMU_L	     (0x4000)    /* limit violation */
#define MMU_S	     (0x2000)    /* supervisor violation */
#define MMU_WP	     (0x0800)    /* write-protected */
#define MMU_I	     (0x0400)    /* invalid descriptor */
#define MMU_M	     (0x0200)    /* ATC entry modified */
#define MMU_T	     (0x0040)    /* transparent translation */
#define MMU_NUM      (0x0007)    /* number of levels traversed */


/* bits for 68040 special status word */
#define CP_040	(0x8000)
#define CU_040	(0x4000)
#define CT_040	(0x2000)
#define CM_040	(0x1000)
#define MA_040	(0x0800)
#define ATC_040 (0x0400)
#define LK_040	(0x0200)
#define RW_040	(0x0100)
#define SIZ_040 (0x0060)
#define TT_040	(0x0018)
#define TM_040	(0x0007)

/* bits for 68040 write back status word */
#define WBV_040   (0x80)
#define WBSIZ_040 (0x60)
#define WBBYT_040 (0x20)
#define WBWRD_040 (0x40)
#define WBLNG_040 (0x00)
#define WBTT_040  (0x18)
#define WBTM_040  (0x07)

/* bus access size codes */
#define BA_SIZE_BYTE    (0x20)
#define BA_SIZE_WORD    (0x40)
#define BA_SIZE_LONG    (0x00)
#define BA_SIZE_LINE    (0x60)

/* bus access transfer type codes */
#define BA_TT_MOVE16    (0x08)

/* bits for 68040 MMU status register (mmusr) */
#define MMU_B_040   (0x0800)
#define MMU_G_040   (0x0400)
#define MMU_S_040   (0x0080)
#define MMU_CM_040  (0x0060)
#define MMU_M_040   (0x0010)
#define MMU_WP_040  (0x0004)
#define MMU_T_040   (0x0002)
#define MMU_R_040   (0x0001)

/* bits in the 68060 fault status long word (FSLW) */
#define	MMU060_MA	(0x08000000)	/* misaligned */
#define	MMU060_LK	(0x02000000)	/* locked transfer */
#define	MMU060_RW	(0x01800000)	/* read/write */
# define MMU060_RW_W	(0x00800000)	/* write */
# define MMU060_RW_R	(0x01000000)	/* read */
# define MMU060_RW_RMW	(0x01800000)	/* read/modify/write */
# define MMU060_W	(0x00800000)	/* general write, includes rmw */
#define	MMU060_SIZ	(0x00600000)	/* transfer size */
#define	MMU060_TT	(0x00180000)	/* transfer type (TT) bits */
#define	MMU060_TM	(0x00070000)	/* transfer modifier (TM) bits */
#define	MMU060_IO	(0x00008000)	/* instruction or operand */
#define	MMU060_PBE	(0x00004000)	/* push buffer bus error */
#define	MMU060_SBE	(0x00002000)	/* store buffer bus error */
#define	MMU060_PTA	(0x00001000)	/* pointer A fault */
#define	MMU060_PTB	(0x00000800)	/* pointer B fault */
#define	MMU060_IL	(0x00000400)	/* double indirect descr fault */
#define	MMU060_PF	(0x00000200)	/* page fault (invalid descr) */
#define	MMU060_SP	(0x00000100)	/* supervisor protection */
#define	MMU060_WP	(0x00000080)	/* write protection */
#define	MMU060_TWE	(0x00000040)	/* bus error on table search */
#define	MMU060_RE	(0x00000020)	/* bus error on read */
#define	MMU060_WE	(0x00000010)	/* bus error on write */
#define	MMU060_TTR	(0x00000008)	/* error caused by TTR translation */
#define	MMU060_BPE	(0x00000004)	/* branch prediction error */
#define	MMU060_SEE	(0x00000001)	/* software emulated error */

/* cases of missing or invalid descriptors */
#define MMU060_DESC_ERR (MMU060_PTA | MMU060_PTB | \
			 MMU060_IL  | MMU060_PF)
/* bits that indicate real errors */
#define MMU060_ERR_BITS (MMU060_PBE | MMU060_SBE | MMU060_DESC_ERR | MMU060_SP | \
			 MMU060_WP  | MMU060_TWE | MMU060_RE       | MMU060_WE)

/* structure for stack frames */

struct frame {
    struct pt_regs ptregs;
    union {
	    struct {
		    unsigned long  iaddr;    /* instruction address */
	    } fmt2;
	    struct {
		    unsigned long  effaddr;  /* effective address */
	    } fmt3;
	    struct {
		    unsigned long  effaddr;  /* effective address */
		    unsigned long  pc;	     /* pc of faulted instr */
	    } fmt4;
	    struct {
		    unsigned long  effaddr;  /* effective address */
		    unsigned short ssw;      /* special status word */
		    unsigned short wb3s;     /* write back 3 status */
		    unsigned short wb2s;     /* write back 2 status */
		    unsigned short wb1s;     /* write back 1 status */
		    unsigned long  faddr;    /* fault address */
		    unsigned long  wb3a;     /* write back 3 address */
		    unsigned long  wb3d;     /* write back 3 data */
		    unsigned long  wb2a;     /* write back 2 address */
		    unsigned long  wb2d;     /* write back 2 data */
		    unsigned long  wb1a;     /* write back 1 address */
		    unsigned long  wb1dpd0;  /* write back 1 data/push data 0*/
		    unsigned long  pd1;      /* push data 1*/
		    unsigned long  pd2;      /* push data 2*/
		    unsigned long  pd3;      /* push data 3*/
	    } fmt7;
	    struct {
		    unsigned long  iaddr;    /* instruction address */
		    unsigned short int1[4];  /* internal registers */
	    } fmt9;
	    struct {
		    unsigned short int1;
		    unsigned short ssw;      /* special status word */
		    unsigned short isc;      /* instruction stage c */
		    unsigned short isb;      /* instruction stage b */
		    unsigned long  daddr;    /* data cycle fault address */
		    unsigned short int2[2];
		    unsigned long  dobuf;    /* data cycle output buffer */
		    unsigned short int3[2];
	    } fmta;
	    struct {
		    unsigned short int1;
		    unsigned short ssw;     /* special status word */
		    unsigned short isc;     /* instruction stage c */
		    unsigned short isb;     /* instruction stage b */
		    unsigned long  daddr;   /* data cycle fault address */
		    unsigned short int2[2];
		    unsigned long  dobuf;   /* data cycle output buffer */
		    unsigned short int3[4];
		    unsigned long  baddr;   /* stage B address */
		    unsigned short int4[2];
		    unsigned long  dibuf;   /* data cycle input buffer */
		    unsigned short int5[3];
		    unsigned	   ver : 4; /* stack frame version # */
		    unsigned	   int6:12;
		    unsigned short int7[18];
	    } fmtb;
    } un;
};

#endif /* __ASSEMBLY__ */

#endif /* _M68K_TRAPS_H */
