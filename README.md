<p align="center">
  <img src="https://img.shields.io/badge/platform-RISC--V%2064--bit-green" alt="RISC-V 64">
  <img src="https://img.shields.io/badge/language-C%2B%2B20-blue" alt="C++20">
  <img src="https://img.shields.io/badge/size-%7E4KB-purple" alt="~4KB">
  <img src="https://img.shields.io/badge/license-GPL--3.0-red" alt="GPL-3.0">
</p>

<h1 align="center">SlipperBoot</h1>
<p align="center"><em>Minimalist RISC-V 64-bit bootloader written in freestanding C++</em></p>

---

**SlipperBoot** is a tiny, bare-metal bootloader for RISC-V 64-bit (rv64gc) systems. It discovers hardware through the Flattened Device Tree (FDT), reads a kernel image (`kernel.elf`) from VirtIO or SDHCI block devices, parses the ELF64 binary, and jumps to its entry point - all in ~3.3 KB of machine code.

It is the first-stage loader for the [SlipperOS](https://github.com/anomalyco/SlipperOS) kernel.

---

## Quick Start

```console
$ make
$ ls -lh bootloader.bin
-rw-r--r-- 1 user user 3.3K …
```

Place the binary at address `0x80000000` (QEMU `-kernel` or raw flash) and `kernel.elf` on a block device. The bootloader will discover devices via FDT, present a boot menu, load the ELF, verify integrity, and hand off control.

---

## Build Dependencies

| Tool                       | Purpose                  |
|----------------------------|--------------------------|
| `riscv64-unknown-elf-g++`  | Default cross-compiler   |
| `riscv64-unknown-elf-objcopy` | Binary extraction     |
| `make`                     | Build orchestration      |

Override the toolchain prefix via `CROSS`:

```console
make CROSS=riscv64-elf          # GNU MCU Eclipse variant
```

---

## Project Structure

| File                   | Role                                       |
|------------------------|--------------------------------------------|
| `src/boot_entry.cpp`   | Naked `_start`: BSS clear, stack setup, call `boot_main` |
| `src/boot_main.cpp`    | Main boot sequence (FDT, devices, ELF load) |
| `include/uart.hpp`     | NS16550A UART driver                        |
| `include/virtio.hpp`   | VirtIO v2 modern MMIO block driver          |
| `include/sdhci.hpp`    | SDHCI SD card driver                        |
| `include/bootmenu.hpp` | Interactive boot device menu                |
| `include/elf.hpp`      | ELF64 parser + integrity check              |
| `include/fdt.hpp`      | Flattened Device Tree core parser           |
| `include/fdt_dev.hpp`  | Device finders (UART, VirtIO, SDHCI)        |
| `dts/milkv-duos.dts`   | Device tree source for Milk-V Duo S         |
| `linker.ld`            | Linker script (`BASE_ADDRESS = 0x80000000`) |
| `Makefile`             | Cross-compilation rules                     |

---

## Technical Details

- **Freestanding** - no libc, no libstdc++, no global constructors, no static objects.
- **No assembly files** - all privileged code (BSS, stack, WFI) lives in `boot_entry.cpp` via inline `asm`.
- **FDT-based discovery** - scans for UART (`ns16550a`), VirtIO (`virtio,mmio`), and SDHCI (`snps,dw-mshc`) devices.
- **VirtIO v2 & SDHCI** - supports both VirtIO MMIO block devices and standard SDHCI controllers.
- **Boot menu** - interactive device selection over UART.
- **ELF integrity check** - detects segment overlap with bootloader memory.
- **Progress bar** - visual feedback during kernel loading.
- **FDT-driven** - DTB passed from previous stage via a1, no hardcoded targets.
- **ELF64 loader** - iterates `PT_LOAD` segments, copies to target addresses, calls `entry(hart_id=0, fdt_addr)`.
- **Stack** - 8 KB. **Binary budget** - 32 KB max. Current build ~3.3 KB.
- **License** - GPL-3.0.

---

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
