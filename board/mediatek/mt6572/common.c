// SPDX-License-Identifier: GPL-2.0
#include <asm/global_data.h>
#include <dm.h>
#include <init.h>
#include <fdt_support.h>

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
	const char *compatible;
	int len;

	compatible = fdt_getprop(gd->fdt_blob, 0, "compatible", &len);
	if (compatible && len) {
		debug("Compatible: %s\n", compatible);

		if (!strcmp(compatible, "jty,d101"))
			return 0xbf400000;
		else if (!strcmp(compatible, "lenovo,a369i"))
			return 0x9fa00000;
		else
			panic("invalid compatible: %s", compatible);
	}

	panic("can't get compatible property: %d", len);
}
