// SPDX-License-Identifier: GPL-2.0
#include <clk.h>
#include <config.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

/* LK framebuffer */
phys_addr_t board_get_usable_ram_top(phys_addr_t total_size)
{
    return 0xbf400000;
}
