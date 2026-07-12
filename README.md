TODO:
- poweroff
- PSCI
- charger, fuel gauge
- replace LK
- ...


status:
- clocks: ok
- pinctrl: ok
- eMMC: ok
- microSD: ok
- USB: ok
- input keys (power + volume): ok


U-Boot changes made by Dr. clk120m:
- MUSB speed improvements (4.6s -> 0.88s for 30 MB file transfer)
- MediaTek keypad driver
- doesn't build without sudo


how to boot:
- `make ARCH=arm CROSS_COMPILE=arm-none-eabi- -j16 jty_d101_defconfig` (or lenovo_a369i_defconfig)
- `make ARCH=arm CROSS_COMPILE=arm-none-eabi- -j16`
- `sudo -E ./target/release/da-boot --lk ../mtkclient/lk-a369.img --kernel ../mt6572/u-boot-new/u-boot.bin -p ../mtkclient/preloader-a369i.img --dram-size-per-rank 0x20000000 --dram-ranks 1 lk`
- **!! for 1gb devices use --dram-ranks 2 !!**
- flashing not tested


booting kernel:
- enable fastboot mode (either select in bootmenu or `fastboot usb 0`)
- `fastboot stage kernel.bin` (kernel.bin is a FIT image)
- `fastboot oem run:"bootm 0x90000000"`
