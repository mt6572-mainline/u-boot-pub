// SPDX-License-Identifier: GPL-2.0
#include <asm/global_data.h>
#include <dm.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_USB_GADGET_GENERIC, 0, &dev);
	if (ret) {
		printf("%s: failed to get USB: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

/* LK framebuffer */
phys_addr_t board_get_usable_ram_top(phys_addr_t total_size)
{
	return 0xbf400000;
}
