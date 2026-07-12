// SPDX-License-Identifier: GPL-2.0+

#include <blk.h>
#include <command.h>
#include <memalign.h>
#include <vsprintf.h>
#include <asm/unaligned.h>
#include <linux/compiler.h>
#include <part.h>

#define PMT_MAGIC_PTV1	0x50547631 /* PTv1 */
#define PMT_MAGIC_MPT1	0x4D505431 /* MPT1 */

#define PMT_MAX_PART	128
#define PMT_NAME_LEN	64
#define PMT_SECTORS	25

struct mtk_pmt_entry {
	char name[PMT_NAME_LEN];
	u64 size;
	u64 offset;
	u32 part_id;
	u32 mask_flags;
} __packed;

static lbaint_t find_pmt_lba(struct blk_desc *desc)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, desc->blksz);
	const int pmt_offsets[] = { 2048, 2040 };
	lbaint_t target_lba;
	u32 magic;


	for (int i = 0; i < 2; i++) {
		target_lba = desc->lba - pmt_offsets[i];

		if (blk_dread(desc, target_lba, 1, (void *)buffer) != 1)
			continue;

		magic = le32_to_cpu(*(u32 *)buffer);

		if (magic == PMT_MAGIC_PTV1 || magic == PMT_MAGIC_MPT1) {
			return target_lba;
		}
	}

	return 0;
}

static int part_test_pmt(struct blk_desc *desc)
{
	lbaint_t pmt_lba = find_pmt_lba(desc);

	if (pmt_lba) {
		printf("PMT: Found valid MediaTek partition table layout.\n");
		return 0;
	}

	return -1;
}

static int part_get_info_pmt(struct blk_desc *desc, int part,
			     struct disk_partition *info)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer,
				 desc->blksz *PMT_SECTORS);
	lbaint_t pmt_lba = find_pmt_lba(desc);
	struct mtk_pmt_entry *entries;
	u32 name_len;

	if (!pmt_lba)
		return -1;

	if (part < 0 || part >= PMT_MAX_PART)
		return -1;

	if (blk_dread(desc, pmt_lba, PMT_SECTORS, buffer) != PMT_SECTORS)
		return -1;

	/* Skip 8-byte table header */
	entries = (struct mtk_pmt_entry *)(buffer + 8);

	if (entries[part].name[0] == '\0' || entries[part].size == 0)
		return -1;

	info->start = entries[part].offset / desc->blksz;
	info->size = entries[part].size / desc->blksz;

	name_len = min((u32)PMT_NAME_LEN, (u32)sizeof(info->name));
	strncpy((char *)info->name, entries[part].name, name_len);
	info->name[name_len - 1] = '\0';

	strcpy((char *)info->type, "U-Boot");
	info->bootable = 0;
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	info->uuid[0] = 0;
#endif

	return 0;
}

static void __maybe_unused part_print_pmt(struct blk_desc *desc)
{
	struct disk_partition info;
	int i;

	printf("Part\tStart Sector\tNum Sectors\tName\n");

	for (i = 0; i < PMT_MAX_PART; i++) {
		if (part_get_info_pmt(desc, i, &info))
			break;

		printf("%4d\t%12llu\t%12llu\t%s\n", i + 1,
		       (unsigned long long)info.start,
		       (unsigned long long)info.size, info.name);
	}
}

U_BOOT_PART_TYPE(pmt) = {
	.name = "PMT",
	.part_type = PART_TYPE_PMT,
	.max_entries = PMT_MAX_PART,
	.get_info = part_get_info_ptr(part_get_info_pmt),
	.print = part_print_ptr(part_print_pmt),
	.test = part_test_pmt,
};
