// SPDX-License-Identifier: GPL-2.0

#include <config.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/arch/misc.h>
#include <asm/secure.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE, SZ_1G);
	return 0;
}

void reset_cpu(void)
{
	// wdt reset
	writel(0x1209, 0x10007000 + 0x14);
}

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT6572\n");

	return 0;
}


