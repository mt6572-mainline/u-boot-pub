// SPDX-License-Identifier: GPL-2.0-only

#include <asm/io.h>
#include <dm.h>
#include <dt-bindings/clock/mediatek,mt6572-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define CLK_CFG_0		0x00

#define ARMPLL_OFFSET		0x100
#define MAINPLL_OFFSET		0x120
#define UNIVPLL_OFFSET		0x140
#define WHPLL_OFFSET		0x240

#define WHPLL_PATHSEL_CON 	0x254
#define RSV_RW0_CON1		0xf04

#define REG_CON0		0x0
#define REG_CON1		0x4
#define REG_PWR_CON0		0x10

#define CON0_RST_BAR		BIT(27)
#define PLL_AO			BIT(1)

enum {
	CLK_PAD_CLK26M,
	CLK_PAD_CLK32K,
};

static const ulong ext_clock_rates[] = {
	[CLK_PAD_CLK26M] = 26 * MHZ,
	[CLK_PAD_CLK32K] = 32000,
};

#define PLL(_id, _name, _base, _en_mask, _rst_bar_mask, _flags, _pcwbits, \
	    _fmin, _fmax)                                                 \
{                                                                         \
	.id = _id,                                                        \
	.reg = (_base) + REG_CON0,                                        \
	.pwr_reg = (_base) + REG_PWR_CON0,                                \
	.en_mask = _en_mask,                                              \
	.rst_bar_mask = _rst_bar_mask,                                    \
	.pd_reg = (_base) + REG_CON1,                                     \
	.pd_shift = 24,                                                   \
	.pcw_reg = (_base) + REG_CON1,                                    \
	.pcw_chg_reg = (_base) + REG_CON1,                                \
	.pcwbits = _pcwbits,                                              \
	.flags = _flags,                                                  \
	.fmin = _fmin,                                                    \
	.fmax = _fmax,                                                    \
}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, "armpll", ARMPLL_OFFSET, 0x00000011, 0, PLL_AO,
	    21, 1001 * MHZ, 1989 * MHZ),

	PLL(CLK_APMIXED_MAINPLL, "mainpll", MAINPLL_OFFSET, 0x00000011,
	    CON0_RST_BAR, PLL_AO | CLK_PLL_HAVE_RST_BAR, 21, 1000 * MHZ,
	    1989 * MHZ),

	PLL(CLK_APMIXED_UNIVPLL, "univpll", UNIVPLL_OFFSET, 0x30000011,
	    CON0_RST_BAR, CLK_PLL_HAVE_RST_BAR, 7, 1248 * MHZ, 1248 * MHZ),

	PLL(CLK_APMIXED_WHPLL, "whpll", WHPLL_OFFSET, 0x00000011, 0, 0, 21,
	    1001 * MHZ, 1989 * MHZ),
};

static const struct mtk_gate_regs univpll_cg_regs = {
	.set_ofs = UNIVPLL_OFFSET,
	.clr_ofs = UNIVPLL_OFFSET,
	.sta_ofs = UNIVPLL_OFFSET,
};

#define GATE_UNIVPLL(_id, _name, _parent, _shift)          \
	GATE_FLAGS(_id, _parent, &univpll_cg_regs, _shift, \
		   CLK_GATE_NO_SETCLR | CLK_PARENT_TOPCKGEN)

static const struct mtk_gate apmixed_gates[] = {
	GATE_UNIVPLL(CLK_APMIXED_USB48M, "univpll_usb48m", CLK_TOP_UNIVPLL_D26, 26),
};

static const struct mtk_clk_tree mt6572_apmixedsys_clk_tree = {
	.pll_parent = EXT_PARENT(CLK_PAD_CLK26M),
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.gates_offs = CLK_APMIXED_USB48M,
	.plls = apmixed_plls,
	.gates = apmixed_gates,
	.num_plls = ARRAY_SIZE(apmixed_plls),
	.num_gates = ARRAY_SIZE(apmixed_gates),
};

#define FACTOR_PLL(_id, _name, _parent, _mult, _div)	\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR_PLL(CLK_TOP_MAINPLL_D2, "mainpll_d2", CLK_APMIXED_MAINPLL, 1, 2),
	FACTOR_PLL(CLK_TOP_MAINPLL_D3, "mainpll_d3", CLK_APMIXED_MAINPLL, 1, 3),
	FACTOR_PLL(CLK_TOP_MAINPLL_D4, "mainpll_d4", CLK_APMIXED_MAINPLL, 1, 4),
	FACTOR_PLL(CLK_TOP_MAINPLL_D5, "mainpll_d5", CLK_APMIXED_MAINPLL, 1, 5),
	FACTOR_PLL(CLK_TOP_MAINPLL_D6, "mainpll_d6", CLK_APMIXED_MAINPLL, 1, 6),
	FACTOR_PLL(CLK_TOP_MAINPLL_D7, "mainpll_d7", CLK_APMIXED_MAINPLL, 1, 7),
	FACTOR_PLL(CLK_TOP_MAINPLL_D8, "mainpll_d8", CLK_APMIXED_MAINPLL, 1, 8),
	FACTOR_PLL(CLK_TOP_MAINPLL_D10, "mainpll_d10", CLK_APMIXED_MAINPLL, 1, 10),
	FACTOR_PLL(CLK_TOP_MAINPLL_D12, "mainpll_d12", CLK_APMIXED_MAINPLL, 1, 12),
	FACTOR_PLL(CLK_TOP_MAINPLL_D20, "mainpll_d20", CLK_APMIXED_MAINPLL, 1, 20),
	FACTOR_PLL(CLK_TOP_MAINPLL_D24, "mainpll_d24", CLK_APMIXED_MAINPLL, 1, 24),
	
	FACTOR_PLL(CLK_TOP_UNIVPLL_D2, "univpll_d2", CLK_APMIXED_UNIVPLL, 1, 2),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D3, "univpll_d3", CLK_APMIXED_UNIVPLL, 1, 3),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D4, "univpll_d4", CLK_APMIXED_UNIVPLL, 1, 4),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D5, "univpll_d5", CLK_APMIXED_UNIVPLL, 1, 5),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D6, "univpll_d6", CLK_APMIXED_UNIVPLL, 1, 6),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D7, "univpll_d7", CLK_APMIXED_UNIVPLL, 1, 7),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D8, "univpll_d8", CLK_APMIXED_UNIVPLL, 1, 8),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D10, "univpll_d10", CLK_APMIXED_UNIVPLL, 1, 10),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D12, "univpll_d12", CLK_APMIXED_UNIVPLL, 1, 12),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D16, "univpll_d16", CLK_APMIXED_UNIVPLL, 1, 16),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D20, "univpll_d20", CLK_APMIXED_UNIVPLL, 1, 20),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D24, "univpll_d24", CLK_APMIXED_UNIVPLL, 1, 24),
	FACTOR_PLL(CLK_TOP_UNIVPLL_D26, "univpll_d26", CLK_APMIXED_UNIVPLL, 1, 26),
};

static const struct mtk_parent uart_sel_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL_D24),
};

static const struct mtk_parent emi2x_sel_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D3),
	TOP_PARENT(CLK_TOP_MAINPLL_D4),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D2),
};

static const struct mtk_parent axi_sel_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D10),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D12)
};

static const struct mtk_parent mfg_mux_parents[] = {
	TOP_PARENT(CLK_TOP_MFG_PRE_491M),
	TOP_PARENT(CLK_TOP_MFG_PRE_500M),
	TOP_PARENT(CLK_TOP_MAINPLL_D3),
	TOP_PARENT(CLK_TOP_UNIVPLL_D2),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D2),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D2),
};

static const struct mtk_parent mfg_pre_parents[] = {
	TOP_PARENT(CLK_TOP_UNIVPLL_D3),
	TOP_PARENT(CLK_TOP_MFG_SEL),
};

static const struct mtk_parent msdc_sel_parents[] = {
	TOP_PARENT(CLK_TOP_MAINPLL_D12),
	TOP_PARENT(CLK_TOP_MAINPLL_D10),
	TOP_PARENT(CLK_TOP_MAINPLL_D8),
	TOP_PARENT(CLK_TOP_UNIVPLL_D7),
	TOP_PARENT(CLK_TOP_MAINPLL_D7),
	TOP_PARENT(CLK_TOP_MAINPLL_D8),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL_D6),
};

static const struct mtk_parent spi_nand_sel_parents[] = {
	TOP_PARENT(CLK_TOP_MAINPLL_D24),
	TOP_PARENT(CLK_TOP_MAINPLL_D20),
	TOP_PARENT(CLK_TOP_UNIVPLL_D20),
	TOP_PARENT(CLK_TOP_UNIVPLL_D16),
	TOP_PARENT(CLK_TOP_UNIVPLL_D12),
	TOP_PARENT(CLK_TOP_UNIVPLL_D10),
	TOP_PARENT(CLK_TOP_MAINPLL_D12),
	TOP_PARENT(CLK_TOP_MAINPLL_D10),
};

static const struct mtk_parent cam_sel_parents[] = {
	TOP_PARENT(CLK_TOP_UNIVPLL_D26),
	TOP_PARENT(CLK_TOP_UNIVPLL_D6),
};

static const struct mtk_parent pwm_mm_sel_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL_D12),
};

static const struct mtk_parent spm_52m_sel_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL_D24),
};

static const struct mtk_parent pmic_spi_sel_ddr2_parents[] = {
	TOP_PARENT(CLK_TOP_MAINPLL_D24),
	TOP_PARENT(CLK_TOP_UNIVPLL_D26),
	TOP_PARENT(CLK_TOP_UNIVPLL_D16),
	EXT_PARENT(CLK_PAD_CLK26M)
};

static const struct mtk_parent pmic_spi_sel_ddr3_parents[] = {
	TOP_PARENT(CLK_TOP_MAINPLL_D20),
	TOP_PARENT(CLK_TOP_UNIVPLL_D26),
	TOP_PARENT(CLK_TOP_UNIVPLL_D16),
	EXT_PARENT(CLK_PAD_CLK26M)
};

static const struct mtk_parent aud_intbus_sel_ddr2_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D24),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D12)
};

static const struct mtk_parent aud_intbus_sel_ddr3_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D20),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAINPLL_D10)
};

static const struct mtk_parent spinfi_pre_sel_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SPINFI_SEL)
};

static const struct mtk_composite top_muxes[] = {
	MUX(CLK_TOP_UART0_SEL, uart_sel_parents, CLK_CFG_0, 0, 1),
	MUX(CLK_TOP_EMI2X_SEL, emi2x_sel_parents, CLK_CFG_0, 1, 4),
	MUX(CLK_TOP_AXI_SEL, axi_sel_parents, CLK_CFG_0, 5, 3),
	MUX(CLK_TOP_MFG_SEL, mfg_mux_parents, CLK_CFG_0, 8, 3),
	MUX(CLK_TOP_MSDC0_SEL, msdc_sel_parents, CLK_CFG_0, 11, 3),
	MUX(CLK_TOP_SPINFI_SEL, spi_nand_sel_parents, CLK_CFG_0, 14, 3),
	MUX(CLK_TOP_CAM_SEL, cam_sel_parents, CLK_CFG_0, 17, 1),
	MUX(CLK_TOP_PWM_MM_SEL, pwm_mm_sel_parents, CLK_CFG_0, 18, 1),
	MUX(CLK_TOP_UART1_SEL, uart_sel_parents, CLK_CFG_0, 19, 1),
	MUX(CLK_TOP_MSDC1_SEL, msdc_sel_parents, CLK_CFG_0, 20, 3),
	MUX(CLK_TOP_SPM_52M_SEL, spm_52m_sel_parents, CLK_CFG_0, 23, 1),
	/* We need some way to switch between DDR2 and DDR3... */
	MUX(CLK_TOP_PMIC_SPI_SEL, pmic_spi_sel_ddr2_parents, CLK_CFG_0, 24, 2),
	MUX(CLK_TOP_AUD_INTBUS_SEL, aud_intbus_sel_ddr2_parents, CLK_CFG_0, 27, 3),
	MUX(CLK_TOP_SPINFI_PRE_SEL, spinfi_pre_sel_parents, CLK_CFG_0, 30, 1),
	MUX(CLK_TOP_MFG_PRE_SEL, mfg_pre_parents, CLK_CFG_0, 31, 1),
};

static const struct mtk_gate_regs top0_cg_regs = {
	.sta_ofs = 0x20,
	.set_ofs = 0x50,
	.clr_ofs = 0x80,
};

static const struct mtk_gate_regs top1_cg_regs = {
	.sta_ofs = 0x24,
	.set_ofs = 0x54,
	.clr_ofs = 0x84,
};

#define GATE_TOP0(_id, _name, _parent, _shift)          \
	GATE_FLAGS(_id, _parent, &top0_cg_regs, _shift, \
		   CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_TOP0_INV_PLL(_id, _name, _parent, _shift)  \
	GATE_FLAGS(_id, _parent, &top0_cg_regs, _shift, \
		   CLK_GATE_SETCLR_INV | CLK_PARENT_APMIXED)

#define GATE_TOP0_INV_EXT(_id, _name, _parent, _shift)  \
	GATE_FLAGS(_id, _parent, &top0_cg_regs, _shift, \
		   CLK_GATE_SETCLR_INV | CLK_PARENT_EXT)

#define GATE_TOP1_TOP(_id, _name, _parent, _shift)      \
	GATE_FLAGS(_id, _parent, &top1_cg_regs, _shift, \
		   CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_TOP1_EXT(_id, _name, _parent, _shift)      \
	GATE_FLAGS(_id, _parent, &top1_cg_regs, _shift, \
		   CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

static const struct mtk_gate top_gates[] = {
	GATE_TOP0(CLK_TOP_PWM_MM, "pwm_mm", CLK_TOP_PWM_MM_SEL, 0),
	GATE_TOP0(CLK_TOP_CAM_MM, "cam_mm", CLK_TOP_CAM_SEL, 1),
	GATE_TOP0(CLK_TOP_MFG_MM, "mfg_mm", CLK_TOP_MFG_SEL, 2),
	GATE_TOP0(CLK_TOP_SPM_52M, "spm_52m", CLK_TOP_SPM_52M_SEL, 3),
	GATE_TOP0_INV_EXT(CLK_TOP_MIPI_26M_DBG, "mipi_26m_dbg", CLK_PAD_CLK26M, 4),
	/* ! DDR2 ! */
	GATE_TOP0(CLK_TOP_DBI_BCLK, "dbi_bclk", CLK_TOP_MAINPLL_D12, 5),
	GATE_TOP0_INV_EXT(CLK_TOP_SC_26M, "sc_26m", CLK_PAD_CLK26M, 6),
	GATE_TOP0_INV_EXT(CLK_TOP_SC_MEM, "sc_mem", CLK_PAD_CLK26M, 7),
	GATE_TOP0(CLK_TOP_DBI_PAD0, "dbi_pad0", CLK_TOP_DBI_BCLK, 16),
	GATE_TOP0(CLK_TOP_DBI_PAD1, "dbi_pad1", CLK_TOP_DBI_BCLK, 17),
	GATE_TOP0(CLK_TOP_DBI_PAD2, "dbi_pad2", CLK_TOP_DBI_BCLK, 18),
	GATE_TOP0(CLK_TOP_DBI_PAD3, "dbi_pad3", CLK_TOP_DBI_BCLK, 19),
	GATE_TOP0_INV_PLL(CLK_TOP_MFG_PRE_491M, "mfg_pre_whpll_491m", CLK_APMIXED_WHPLL, 20),
	GATE_TOP0_INV_PLL(CLK_TOP_MFG_PRE_500M, "mfg_pre_whpll_500m", CLK_APMIXED_WHPLL, 21),
	GATE_TOP0_INV_EXT(CLK_TOP_ARMDCM, "armdcm", CLK_PAD_CLK26M, 31),

	GATE_TOP1_EXT(CLK_TOP_EFUSE, "efuse", CLK_PAD_CLK26M, 0),
	GATE_TOP1_EXT(CLK_TOP_THERMAL, "thermal", CLK_PAD_CLK26M, 1),
	GATE_TOP1_TOP(CLK_TOP_APDMA, "apdma", CLK_TOP_AXI_SEL, 2),
	GATE_TOP1_TOP(CLK_TOP_I2C0, "i2c0", CLK_TOP_AXI_SEL, 3),
	GATE_TOP1_TOP(CLK_TOP_I2C1, "i2c1", CLK_TOP_AXI_SEL, 4),
	GATE_TOP1_TOP(CLK_TOP_NFI, "nfi", CLK_TOP_AXI_SEL, 6),
	GATE_TOP1_TOP(CLK_TOP_NFI_ECC, "nfi_ecc", CLK_TOP_AXI_SEL, 7),
	GATE_TOP1_TOP(CLK_TOP_DEBUGSYS, "debugsys", CLK_TOP_AXI_SEL, 8),
	GATE_TOP1_TOP(CLK_TOP_PWM, "pwm", CLK_TOP_AXI_SEL, 9),
	GATE_TOP1_TOP(CLK_TOP_UART0, "uart0", CLK_TOP_UART0_SEL, 10),
	GATE_TOP1_TOP(CLK_TOP_UART1, "uart1", CLK_TOP_UART1_SEL, 11),
	GATE_TOP1_TOP(CLK_TOP_BTIF, "btif", CLK_TOP_AXI_SEL, 12),
	GATE_TOP1_TOP(CLK_TOP_USB, "usb", CLK_TOP_AXI_SEL, 13),
	GATE_TOP1_EXT(CLK_TOP_FHCTL, "fhctl", CLK_PAD_CLK26M, 14),
	GATE_TOP1_TOP(CLK_TOP_SPINFI, "spinfi", CLK_TOP_SPINFI_SEL, 16),
	GATE_TOP1_TOP(CLK_TOP_MSDC0, "msdc0", CLK_TOP_MSDC0_SEL, 17),
	GATE_TOP1_TOP(CLK_TOP_MSDC1, "msdc1", CLK_TOP_MSDC1_SEL, 18),
	GATE_TOP1_TOP(CLK_TOP_PMIC_SPI, "pmic_spi", CLK_TOP_PMIC_SPI_SEL, 20),
	GATE_TOP1_EXT(CLK_TOP_SEJ, "sej", CLK_PAD_CLK26M, 21),
	GATE_TOP1_EXT(CLK_TOP_MEMSLP_DLYER, "memslp_dlyer", CLK_PAD_CLK26M, 22),
	GATE_TOP1_EXT(CLK_TOP_APXGPT, "apxgpt", CLK_PAD_CLK26M, 24),
	GATE_TOP1_TOP(CLK_TOP_AUD, "aud", CLK_TOP_AUD_INTBUS_SEL, 25),
	GATE_TOP1_EXT(CLK_TOP_SPM, "spm", CLK_PAD_CLK26M, 26),
	GATE_TOP1_EXT(CLK_TOP_PMIC_26M, "pmic_26m", CLK_PAD_CLK26M, 29),
	GATE_TOP1_EXT(CLK_TOP_AUXADC, "auxadc", CLK_PAD_CLK26M, 30),
};

static const struct mtk_clk_tree mt6572_topckgen_clk_tree = {
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.fdivs_offs = CLK_TOP_MAINPLL_D2,
	.muxes_offs = CLK_TOP_UART0_SEL,
	.gates_offs = CLK_TOP_PWM_MM,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
	.gates = top_gates,
	.num_fdivs = ARRAY_SIZE(top_fixed_divs),
	.num_muxes = ARRAY_SIZE(top_muxes),
	.num_gates = ARRAY_SIZE(top_gates),
};

static const struct mtk_parent cpu_mux_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	APMIXED_PARENT(CLK_APMIXED_ARMPLL),
	APMIXED_PARENT(CLK_APMIXED_UNIVPLL),
	TOP_PARENT(CLK_TOP_MAINPLL_D2),
};

static const struct mtk_composite infra_muxes[] = {
	MUX(CLK_INFRA_CPUSEL, cpu_mux_parents, 0x0, 2, 2),
};

static const struct mtk_clk_tree mt6572_infracfg_ao_clk_tree = {
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.muxes_offs = CLK_INFRA_CPUSEL,
	.muxes = infra_muxes,
	.num_muxes = ARRAY_SIZE(infra_muxes),
};

static int mt6572_apmixedsys_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt6572_apmixedsys_clk_tree);
}

static int mt6572_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt6572_topckgen_clk_tree);
}

static int mt6572_infracfg_ao_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt6572_infracfg_ao_clk_tree);
}

static const struct udevice_id mt6572_apmixed[] = {
	{ .compatible = "mediatek,mt6572-apmixedsys", },
	{ }
};

static const struct udevice_id mt6572_topckgen_compat[] = {
	{ .compatible = "mediatek,mt6572-topckgen", },
	{ }
};

static const struct udevice_id mt6572_infracfg_ao_compat[] = {
	{ .compatible = "mediatek,mt6572-infracfg_ao", },
	{ }
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt6572-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt6572_apmixed,
	.probe = mt6572_apmixedsys_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt6572-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt6572_topckgen_compat,
	.probe = mt6572_topckgen_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg_ao) = {
	.name = "mt6572-infracfg_ao",
	.id = UCLASS_CLK,
	.of_match = mt6572_infracfg_ao_compat,
	.probe = mt6572_infracfg_ao_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_infrasys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
