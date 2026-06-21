#include <cstdint>
#include "uart.hpp"
#include "virtio.hpp"
#include "sdhci.hpp"
#include "elf.hpp"
#include "fdt_dev.hpp"
#include "bootmenu.hpp"

extern uint8_t _sbss[];
extern uint8_t _stack_end[];

extern "C" void boot_main(uint64_t fdt_addr) {
    uart_info ui = fdt_find_uart((const void*)fdt_addr);
    UART uart(ui.base);
    uart.init();
    uart.puts("SlipperBoot v0.3\n");

    BootDevice devs[16];
    int ndevs = 0;

    // find VirtIO devices from FDT
    mmio_dev virtio_devs[8];
    int nv = fdt_find_virtio((const void*)fdt_addr, virtio_devs, 8);
    for (int i = 0; i < nv; i++) {
        devs[ndevs].type = DEV_VIRTIO;
        devs[ndevs].base = virtio_devs[i].base;
        devs[ndevs].irq = virtio_devs[i].irq;
        devs[ndevs].avail = true;
        ndevs++;
    }

    // find SDHCI devices from FDT
    mmio_dev sdhci_devs[8];
    int ns = fdt_find_sdhci((const void*)fdt_addr, sdhci_devs, 8);
    for (int i = 0; i < ns; i++) {
        devs[ndevs].type = DEV_SDHCI;
        devs[ndevs].base = sdhci_devs[i].base;
        devs[ndevs].irq = sdhci_devs[i].irq;
        devs[ndevs].avail = true;
        ndevs++;
    }

    if (ndevs < 1) {
        uart.puts("no boot device\n");
        while (1) ;
    }

    int sel = boot_menu(uart, devs, ndevs);
    if (sel < 0 || sel >= ndevs || !devs[sel].avail) {
        uart.puts("invalid device\n");
        while (1) ;
    }

    static uint8_t kernel_buf[2 * 1024 * 1024] __attribute__((aligned(512)));

    uart.puts("loading kernel.elf [");
    bool ok = true;
    if (devs[sel].type == DEV_VIRTIO) {
        VirtIOBlock disk(devs[sel].base);
        if (!disk.probe()) ok = false;
        for (int i = 0; i < 4096 && ok; i++) {
            if (!disk.read_sector(i, &kernel_buf[i * 512])) ok = false;
            if ((i & 127) == 0) uart.putchar('#');
        }
    } else {
        SDHCI mmc(devs[sel].base);
        if (!mmc.probe()) ok = false;
        for (int i = 0; i < 4096 && ok; i++) {
            if (!mmc.read_sector(i, &kernel_buf[i * 512])) ok = false;
            if ((i & 127) == 0) uart.putchar('#');
        }
    }
    uart.puts("]\n");
    if (!ok) {
        uart.puts("read error\n");
        while (1) ;
    }

    ELF64 elf(kernel_buf);
    if (!elf.valid()) {
        uart.puts("bad ELF\n");
        while (1) ;
    }

    uint64_t boot_start = (uint64_t)_sbss;
    uint64_t boot_end = (uint64_t)_stack_end;
    if (!elf.check_safe(boot_start, boot_end)) {
        uart.puts("ELF overlaps bootloader\n");
        while (1) ;
    }

    elf.load_all();

    uart.puts("jumping to kernel\n");

    auto entry = elf.entry();
    void (*kernel_entry)(uint64_t, uint64_t) =
        reinterpret_cast<decltype(kernel_entry)>(entry);
    kernel_entry(0, fdt_addr);
}
