/*	$OpenBSD: cpu.h,v 1.91 2012/12/02 07:03:31 guenther Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	Copyright (C) 1989 Digital Equipment Corporation.
 *	Permission to use, copy, modify, and distribute this software and
 *	its documentation for any purpose and without fee is hereby granted,
 *	provided that the above copyright notice appears in all copies.
 *	Digital Equipment Corporation makes no representations about the
 *	suitability of this software for any purpose.  It is provided "as is"
 *	without express or implied warranty.
 *
 *	from: @(#)cpu.h	8.4 (Berkeley) 1/4/94
 */

#ifndef _MIPS64_CPU_H_
#define	_MIPS64_CPU_H_

#ifndef _LOCORE

/*
 * MIPS32-style segment definitions.
 * They only cover the first 512MB of physical addresses.
 */
#define	CKSEG0_BASE		0xffffffff80000000UL
#define	CKSEG1_BASE		0xffffffffa0000000UL
#define	CKSSEG_BASE		0xffffffffc0000000UL
#define	CKSEG3_BASE		0xffffffffe0000000UL
#define	CKSEG_SIZE		0x0000000020000000UL

#define	CKSEG0_TO_PHYS(x)	((u_long)(x) & (CKSEG_SIZE - 1))
#define	CKSEG1_TO_PHYS(x)	((u_long)(x) & (CKSEG_SIZE - 1))
#define	PHYS_TO_CKSEG0(x)	((u_long)(x) | CKSEG0_BASE)
#define	PHYS_TO_CKSEG1(x)	((u_long)(x) | CKSEG1_BASE)

/*
 * MIPS64-style segment definitions.
 * These allow for 36 bits of addressable physical memory, thus 64GB.
 */

/*
 * Cache Coherency Attributes.
 */
/* r8k only */
#define	CCA_NC_COPROCESSOR	0UL	/* uncached, coprocessor ordered */
/* common to r4, r5k, r8k and r1xk */
#define	CCA_NC			2UL	/* uncached, write-around */
#define	CCA_NONCOHERENT		3UL	/* cached, non-coherent, write-back */
/* r8k, r1xk only */
#define	CCA_COHERENT_EXCL	4UL	/* cached, coherent, exclusive */
#define	CCA_COHERENT_EXCLWRITE	5UL	/* cached, coherent, exclusive write */
/* r4k only */
#define	CCA_COHERENT_UPDWRITE	6UL	/* cached, coherent, update on write */
/* r1xk only */
#define	CCA_NC_ACCELERATED	7UL	/* uncached accelerated */

#ifdef TGT_COHERENT
#define	CCA_CACHED		CCA_COHERENT_EXCLWRITE
#else
#define	CCA_CACHED		CCA_NONCOHERENT
#endif

/*
 * Uncached spaces.
 * R1x000 processors use bits 58:57 of uncached virtual addresses (CCA_NC)
 * to select different spaces. Unfortunately, other processors need these
 * bits to be zero, so uncached address have to be decided at runtime.
 */
#define	SP_HUB			0UL	/* Hub space */
#define	SP_IO			1UL	/* I/O space */
#define	SP_SPECIAL		2UL	/* Memory Special space */
#define	SP_NC			3UL	/* Memory Uncached space */

#define	XKSSSEG_BASE		0x4000000000000000UL
#define	XKPHYS_BASE		0x8000000000000000UL
#define	XKSSEG_BASE		0xc000000000000000UL

#define	XKPHYS_TO_PHYS(x)	((paddr_t)(x) & 0x0000000fffffffffUL)
#define	PHYS_TO_XKPHYS(x,c)	((paddr_t)(x) | XKPHYS_BASE | ((c) << 59))
#define	PHYS_TO_XKPHYS_UNCACHED(x,s) \
	(PHYS_TO_XKPHYS(x, CCA_NC) | ((s) << 57))
#define	IS_XKPHYS(va)		(((va) >> 62) == 2)
#define	XKPHYS_TO_CCA(x)	(((x) >> 59) & 0x07)
#define	XKPHYS_TO_SP(x)		(((x) >> 57) & 0x03)

#endif	/* _LOCORE */

/*
 * Exported definitions unique to mips cpu support.
 */

#if defined(_KERNEL) && !defined(_LOCORE)

#include <sys/device.h>
#include <sys/lock.h>
#include <machine/intr.h>
#include <sys/sched.h>

struct cpu_hwinfo {
	uint32_t	c0prid;
	uint32_t	c1prid;
	uint32_t	clock;	/* Hz */
	uint32_t	tlbsize;
	uint		type;
	uint32_t	l2size;
};

struct cpu_info {
	struct device	*ci_dev;	/* our device */
	struct cpu_info	*ci_self;	/* pointer to this structure */
	struct cpu_info	*ci_next;	/* next cpu */
	struct proc	*ci_curproc;
	struct user	*ci_curprocpaddr;
	struct proc	*ci_fpuproc;	/* pointer to last proc to use FP */
	uint32_t	 ci_delayconst;
	struct cpu_hwinfo
			ci_hw;

	/* cache information */
	uint		ci_cacheconfiguration;
	uint		ci_cacheways;
	uint		ci_l1instcachesize;
	uint		ci_l1instcacheline;
	uint		ci_l1instcacheset;
	uint		ci_l1datacachesize;
	uint		ci_l1datacacheline;
	uint		ci_l1datacacheset;
	uint		ci_l2size;
	uint		ci_l2line;
	uint		ci_l3size;

	/* function pointers for the cache handling routines */
	void		(*ci_SyncCache)(struct cpu_info *);
	void		(*ci_InvalidateICache)(struct cpu_info *, vaddr_t,
			    size_t);
	void		(*ci_SyncDCachePage)(struct cpu_info *, vaddr_t,
			    paddr_t);
	void		(*ci_HitSyncDCache)(struct cpu_info *, vaddr_t, size_t);
	void		(*ci_HitInvalidateDCache)(struct cpu_info *, vaddr_t,
			    size_t);
	void		(*ci_IOSyncDCache)(struct cpu_info *, vaddr_t, size_t,
			    int);

	struct schedstate_percpu
			ci_schedstate;
	int		ci_want_resched;	/* need_resched() invoked */
	cpuid_t		ci_cpuid;		/* our CPU ID */
	uint32_t	ci_randseed;		/* per cpu random seed */
	int		ci_ipl;			/* software IPL */
	uint32_t	ci_softpending;		/* pending soft interrupts */
	int		ci_clock_started;
	u_int32_t	ci_cpu_counter_last;	/* last compare value loaded */
	u_int32_t	ci_cpu_counter_interval; /* # of counter ticks/tick */

	u_int32_t	ci_pendingticks;
	struct pmap	*ci_curpmap;
	uint		ci_intrdepth;		/* interrupt depth */
#ifdef MULTIPROCESSOR
	u_long		ci_flags;		/* flags; see below */
	struct intrhand	ci_ipiih;
#endif
	volatile int    ci_ddb;
#define	CI_DDB_RUNNING		0
#define	CI_DDB_SHOULDSTOP	1
#define	CI_DDB_STOPPED		2
#define	CI_DDB_ENTERDDB		3
#define	CI_DDB_INDDB		4

#ifdef DIAGNOSTIC
	int	ci_mutex_level;
#endif
};

#define	CPUF_PRIMARY	0x01		/* CPU is primary CPU */
#define	CPUF_PRESENT	0x02		/* CPU is present */
#define	CPUF_RUNNING	0x04		/* CPU is running */

extern struct cpu_info cpu_info_primary;
extern struct cpu_info *cpu_info_list;
#define CPU_INFO_ITERATOR		int
#define	CPU_INFO_FOREACH(cii, ci)	for (cii = 0, ci = cpu_info_list; \
					    ci != NULL; ci = ci->ci_next)

#define CPU_INFO_UNIT(ci)               ((ci)->ci_dev ? (ci)->ci_dev->dv_unit : 0)

#ifdef MULTIPROCESSOR
#define MAXCPUS				4
#define getcurcpu()			hw_getcurcpu()
#define setcurcpu(ci)			hw_setcurcpu(ci)
extern struct cpu_info *get_cpu_info(int);
#define curcpu() getcurcpu()
#define	CPU_IS_PRIMARY(ci)		((ci)->ci_flags & CPUF_PRIMARY)
#define cpu_number()			(curcpu()->ci_cpuid)

extern struct cpuset cpus_running;
void cpu_unidle(struct cpu_info *);
void cpu_boot_secondary_processors(void);
#define cpu_boot_secondary(ci)          hw_cpu_boot_secondary(ci)
#define cpu_hatch(ci)                   hw_cpu_hatch(ci)

vaddr_t alloc_contiguous_pages(size_t);

#define MIPS64_IPI_NOP		0x00000001
#define MIPS64_IPI_RENDEZVOUS	0x00000002
#define MIPS64_IPI_DDB		0x00000004
#define MIPS64_NIPIS		3	/* must not exceed 32 */

void	mips64_ipi_init(void);
void	mips64_send_ipi(unsigned int, unsigned int);
void	smp_rendezvous_cpus(unsigned long, void (*)(void *), void *arg);

#include <sys/mplock.h>
#else
#define MAXCPUS				1
#define curcpu()			(&cpu_info_primary)
#define	CPU_IS_PRIMARY(ci)		1
#define cpu_number()			0
#define cpu_unidle(ci)
#define get_cpu_info(i)			(&cpu_info_primary)
#endif

extern void (*md_startclock)(struct cpu_info *);
void	cp0_calibrate(struct cpu_info *);

#include <machine/frame.h>

/*
 * Arguments to hardclock encapsulate the previous machine state in
 * an opaque clockframe.
 */
#define	clockframe trap_frame	/* Use normal trap frame */

#define	SR_KSU_USER		0x00000010
#define	CLKF_USERMODE(framep)	((framep)->sr & SR_KSU_USER)
#define	CLKF_PC(framep)		((framep)->pc)
#define	CLKF_INTR(framep)	(curcpu()->ci_intrdepth > 1)	/* XXX */

/*
 * This is used during profiling to integrate system time.
 */
#define	PROC_PC(p)	((p)->p_md.md_regs->pc)
#define	PROC_STACK(p)	((p)->p_md.md_regs->sp)

/*
 * Preempt the current process if in interrupt from user mode,
 * or after the current trap/syscall if in system mode.
 */
#define	need_resched(ci) \
	do { \
		(ci)->ci_want_resched = 1; \
		if ((ci)->ci_curproc != NULL) \
			aston((ci)->ci_curproc); \
	} while(0)
#define	clear_resched(ci) 	(ci)->ci_want_resched = 0

/*
 * Give a profiling tick to the current process when the user profiling
 * buffer pages are invalid.  On the PICA, request an ast to send us
 * through trap, marking the proc as needing a profiling tick.
 */
#define	need_proftick(p)	aston(p)

/*
 * Notify the current process (p) that it has a signal pending,
 * process as soon as possible.
 */
#ifdef MULTIPROCESSOR
#define	signotify(p)		(aston(p), cpu_unidle(p->p_cpu))
#else
#define	signotify(p)		aston(p)
#endif

#define	aston(p)		p->p_md.md_astpending = 1

#ifdef CPU_R8000
#define	mips_sync()		__asm__ __volatile__ ("lw $0, 0(%0)" :: \
				    "r" (PHYS_TO_XKPHYS(0, CCA_NC)) : "memory")
#else
#define	mips_sync()		__asm__ __volatile__ ("sync" ::: "memory")
#endif

#endif /* _KERNEL && !_LOCORE */

#ifdef _KERNEL
/*
 * Values for the code field in a break instruction.
 */
#define	BREAK_INSTR		0x0000000d
#define	BREAK_VAL_MASK		0x03ff0000
#define	BREAK_VAL_SHIFT		16
#define	BREAK_KDB_VAL		512
#define	BREAK_SSTEP_VAL		513
#define	BREAK_BRKPT_VAL		514
#define	BREAK_SOVER_VAL		515
#define	BREAK_DDB_VAL		516
#define	BREAK_FPUEMUL_VAL	517
#define	BREAK_KDB	(BREAK_INSTR | (BREAK_KDB_VAL << BREAK_VAL_SHIFT))
#define	BREAK_SSTEP	(BREAK_INSTR | (BREAK_SSTEP_VAL << BREAK_VAL_SHIFT))
#define	BREAK_BRKPT	(BREAK_INSTR | (BREAK_BRKPT_VAL << BREAK_VAL_SHIFT))
#define	BREAK_SOVER	(BREAK_INSTR | (BREAK_SOVER_VAL << BREAK_VAL_SHIFT))
#define	BREAK_DDB	(BREAK_INSTR | (BREAK_DDB_VAL << BREAK_VAL_SHIFT))
#define	BREAK_FPUEMUL	(BREAK_INSTR | (BREAK_FPUEMUL_VAL << BREAK_VAL_SHIFT))

#endif /* _KERNEL */

/*
 * CTL_MACHDEP definitions.
 */
#define	CPU_ALLOWAPERTURE	1	/* allow mmap of /dev/xf86 */
		/*		2	   formerly: keyboard reset */
#define	CPU_MAXID		3	/* number of valid machdep ids */

#define	CTL_MACHDEP_NAMES {			\
	{ 0, 0 },				\
	{ "allowaperture", CTLTYPE_INT },	\
	{ 0, 0 },				\
}

/*
 * MIPS CPU types (cp_imp).
 */
#define	MIPS_R2000	0x01	/* MIPS R2000 CPU		ISA I   */
#define	MIPS_R3000	0x02	/* MIPS R3000 CPU		ISA I   */
#define	MIPS_R6000	0x03	/* MIPS R6000 CPU		ISA II	*/
#define	MIPS_R4000	0x04	/* MIPS R4000/4400 CPU		ISA III	*/
#define	MIPS_R3LSI	0x05	/* LSI Logic R3000 derivate	ISA I	*/
#define	MIPS_R6000A	0x06	/* MIPS R6000A CPU		ISA II	*/
#define MIPS_OCTEON	0x06	/* Cavium OCTEON		MIPS64R2*/
#define	MIPS_R3IDT	0x07	/* IDT R3000 derivate		ISA I	*/
#define	MIPS_R10000	0x09	/* MIPS R10000/T5 CPU		ISA IV  */
#define	MIPS_R4200	0x0a	/* MIPS R4200 CPU (ICE)		ISA III */
#define	MIPS_R4300	0x0b	/* NEC VR4300 CPU		ISA III */
#define	MIPS_R4100	0x0c	/* NEC VR41xx CPU MIPS-16	ISA III */
#define	MIPS_R12000	0x0e	/* MIPS R12000			ISA IV  */
#define	MIPS_R14000	0x0f	/* MIPS R14000			ISA IV  */
#define	MIPS_R8000	0x10	/* MIPS R8000 Blackbird/TFP	ISA IV  */
#define	MIPS_R4600	0x20	/* PMCS R4600 Orion		ISA III */
#define	MIPS_R4700	0x21	/* PMCS R4700 Orion		ISA III */
#define	MIPS_R3TOSH	0x22	/* Toshiba R3000 based CPU	ISA I	*/
#define	MIPS_R5000	0x23	/* MIPS R5000 CPU		ISA IV  */
#define	MIPS_RM7000	0x27	/* PMCS RM7000 CPU		ISA IV  */
#define	MIPS_RM52X0	0x28	/* PMCS RM52X0 CPU		ISA IV  */
#define	MIPS_RM9000	0x34	/* PMCS RM9000 CPU		ISA IV  */
#define	MIPS_LOONGSON	0x42	/* STC LoongSon CPU		ISA III */
#define	MIPS_VR5400	0x54	/* NEC Vr5400 CPU		ISA IV+ */
#define	MIPS_LOONGSON2	0x63	/* STC LoongSon2/3 CPU		ISA III+ */

/*
 * MIPS FPU types. Only soft, rest is the same as cpu type.
 */
#define	MIPS_SOFT	0x00	/* Software emulation		ISA I   */


#if defined(_KERNEL) && !defined(_LOCORE)

extern register_t protosr;

struct exec_package;
struct user;

void	tlb_asid_wrap(struct cpu_info *);
void	tlb_flush(int);
void	tlb_flush_addr(vaddr_t);
void	tlb_init(unsigned int);
void	tlb_set_gbase(vaddr_t, vsize_t);
void	tlb_set_page_mask(uint32_t);
void	tlb_set_pid(u_int);
void	tlb_set_wired(uint32_t);
int	tlb_update(vaddr_t, register_t);

void	build_trampoline(vaddr_t, vaddr_t);
void	cpu_switchto_asm(struct proc *, struct proc *);
int	exec_md_map(struct proc *, struct exec_package *);
void	savectx(struct user *, int);

void	enable_fpu(struct proc *);
void	save_fpu(void);
int	fpe_branch_emulate(struct proc *, struct trap_frame *, uint32_t,
	    vaddr_t);
void	MipsSaveCurFPState(struct proc *);
void	MipsSaveCurFPState16(struct proc *);
void	MipsSwitchFPState(struct proc *, struct trap_frame *);
void	MipsSwitchFPState16(struct proc *, struct trap_frame *);

int	guarded_read_1(paddr_t, uint8_t *);
int	guarded_read_2(paddr_t, uint16_t *);
int	guarded_read_4(paddr_t, uint32_t *);
int	guarded_write_4(paddr_t, uint32_t);

void	MipsFPTrap(struct trap_frame *);
register_t MipsEmulateBranch(struct trap_frame *, vaddr_t, uint32_t, uint32_t);

/*
 * Low level access routines to CPU registers
 */

void	setsoftintr0(void);
void	clearsoftintr0(void);
void	setsoftintr1(void);
void	clearsoftintr1(void);
register_t enableintr(void);
register_t disableintr(void);
register_t getsr(void);
register_t setsr(register_t);

u_int	cp0_get_count(void);
register_t cp0_get_config(void);
uint32_t cp0_get_config_1(void);
uint32_t cp0_get_config_2(void);
uint32_t cp0_get_config_3(void);
register_t cp0_get_prid(void);
void	cp0_reset_cause(register_t);
void	cp0_set_compare(u_int);
void	cp0_set_config(register_t);
void	cp0_set_trapbase(register_t);
u_int	cp1_get_prid(void);

/*
 * Cache routines (may be overriden)
 */

#ifndef	Mips_SyncCache
#define	Mips_SyncCache(ci) \
	((ci)->ci_SyncCache)(ci)
#endif
#ifndef	Mips_InvalidateICache
#define	Mips_InvalidateICache(ci, va, l) \
	((ci)->ci_InvalidateICache)(ci, va, l)
#endif
#ifndef	Mips_SyncDCachePage
#define	Mips_SyncDCachePage(ci, va, pa) \
	((ci)->ci_SyncDCachePage)(ci, va, pa)
#endif
#ifndef	Mips_HitSyncDCache
#define	Mips_HitSyncDCache(ci, va, l) \
	((ci)->ci_HitSyncDCache)(ci, va, l)
#endif
#ifndef	Mips_HitInvalidateDCache
#define	Mips_HitInvalidateDCache(ci, va, l) \
	((ci)->ci_HitInvalidateDCache)(ci, va, l)
#endif
#ifndef	Mips_IOSyncDCache
#define	Mips_IOSyncDCache(ci, va, l, h) \
	((ci)->ci_IOSyncDCache)(ci, va, l, h)
#endif
 
#endif /* _KERNEL && !_LOCORE */
#endif /* !_MIPS64_CPU_H_ */
