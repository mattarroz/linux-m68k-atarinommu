/*
 *  Copyright (C) 2007 Geert Uytterhoeven
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/platform_device.h>
#include <linux/mm.h>			// FIXME show_mem()

#include <asm/amigahw.h>


static struct resource amiga_serial_resources[] = {
	{
		/*
		 *  We request SERDAT and SERPER only, because the serial
		 *  registers are too spread over the custom register space
		 */
		.start	= CUSTOM_PHYSADDR+0x30,
		.end	= CUSTOM_PHYSADDR+0x33,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device amiga_serial = {
	.name		= "amiga-serial",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(amiga_serial_resources),
	.resource	= amiga_serial_resources,
};

static int __init amiga_init_devices(void)
{
	if (AMIGAHW_PRESENT(AMI_FLOPPY))
		platform_device_register_simple("amiga-floppy", -1, NULL, 0);

	if (AMIGAHW_PRESENT(AMI_SERIAL))
		platform_device_register(&amiga_serial);

#if 0
    /* video hardware */
[ ] AMIGAHW_PRESENT(AMI_VIDEO);		/* Amiga Video */
[ ] AMIGAHW_PRESENT(AMI_BLITTER);	/* Amiga Blitter */
[ ] AMIGAHW_PRESENT(AMBER_FF);		/* Amber Flicker Fixer */
    /* sound hardware */
[ ] AMIGAHW_PRESENT(AMI_AUDIO);		/* Amiga Audio */
    /* disk storage interfaces */
[ ] AMIGAHW_PRESENT(AMI_FLOPPY);	/* Amiga Floppy */
[ ] AMIGAHW_PRESENT(A3000_SCSI);	/* SCSI (wd33c93, A3000 alike) */
[ ] AMIGAHW_PRESENT(A4000_SCSI);	/* SCSI (ncr53c710, A4000T alike) */
[ ] AMIGAHW_PRESENT(A1200_IDE);		/* IDE (A1200 alike) */
[ ] AMIGAHW_PRESENT(A4000_IDE);		/* IDE (A4000 alike) */
[ ] AMIGAHW_PRESENT(CD_ROM);		/* CD ROM drive */
    /* other I/O hardware */
[ ] AMIGAHW_PRESENT(AMI_KEYBOARD);	/* Amiga Keyboard */
[ ] AMIGAHW_PRESENT(AMI_MOUSE);		/* Amiga Mouse */
[ ] AMIGAHW_PRESENT(AMI_SERIAL);	/* Amiga Serial */
[ ] AMIGAHW_PRESENT(AMI_PARALLEL);	/* Amiga Parallel */
    /* real time clocks */
[ ] AMIGAHW_PRESENT(A2000_CLK);		/* Hardware Clock (A2000 alike) */
[ ] AMIGAHW_PRESENT(A3000_CLK);		/* Hardware Clock (A3000 alike) */
    /* supporting hardware */
[ ] AMIGAHW_PRESENT(CHIP_RAM);		/* Chip RAM */
[ ] AMIGAHW_PRESENT(PAULA);		/* Paula (8364) */
[ ] AMIGAHW_PRESENT(DENISE);		/* Denise (8362) */
[ ] AMIGAHW_PRESENT(DENISE_HR);		/* Denise (8373) */
[ ] AMIGAHW_PRESENT(LISA);		/* Lisa (8375) */
[ ] AMIGAHW_PRESENT(AGNUS_PAL);		/* Normal/Fat PAL Agnus (8367/8371) */
[ ] AMIGAHW_PRESENT(AGNUS_NTSC);	/* Normal/Fat NTSC Agnus (8361/8370) */
[ ] AMIGAHW_PRESENT(AGNUS_HR_PAL);	/* Fat Hires PAL Agnus (8372) */
[ ] AMIGAHW_PRESENT(AGNUS_HR_NTSC);	/* Fat Hires NTSC Agnus (8372) */
[ ] AMIGAHW_PRESENT(ALICE_PAL);		/* PAL Alice (8374) */
[ ] AMIGAHW_PRESENT(ALICE_NTSC);	/* NTSC Alice (8374) */
[ ] AMIGAHW_PRESENT(MAGIC_REKICK);	/* A3000 Magic Hard Rekick */
[ ] AMIGAHW_PRESENT(PCMCIA);		/* PCMCIA Slot */
[ ] AMIGAHW_PRESENT(GG2_ISA);		/* GG2 Zorro2ISA Bridge */
[ ] AMIGAHW_PRESENT(ZORRO);		/* Zorro AutoConfig */
[ ] AMIGAHW_PRESENT(ZORRO3);		/* Zorro III */
#endif

	return 0;
}

device_initcall(amiga_init_devices);

