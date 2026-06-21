<p align="center">
  <img src="https://img.shields.io/badge/platform-RISC--V%2064--bit-green" alt="RISC-V 64">
  <img src="https://img.shields.io/badge/language-C%2B%2B20-blue" alt="C++20">
  <img src="https://img.shields.io/badge/size-%7E4KB-purple" alt="~4KB">
  <img src="https://img.shields.io/badge/license-GPL--3.0-red" alt="GPL-3.0">
</p>

<h1 align="center">SlipperBoot</h1>
<p align="center"><em>Минималистичный RISC-V 64-bit загрузчик на голом C++</em></p>

---

**SlipperBoot** - крошечный bare-metal загрузчик для RISC-V 64-bit (rv64gc).
Обнаруживает железо через Flattened Device Tree (FDT), читает образ ядра
(`kernel.elf`) с VirtIO v2 modern MMIO блочного устройства, парсит ELF64
и прыгает в точку входа - всё в ~4 КБ машинного кода.

Загрузчик первой ступени для ядра [SlipperOS](https://github.com/anomalyco/SlipperOS).

---

## Быстрый старт

```console
$ make
riscv64-unknown-elf-g++ -march=rv64gc -mabi=lp64d -mcmodel=medany -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -fno-rtti boot_entry.cpp boot_main.cpp -T linker.ld -nostdlib -o bootloader.elf
riscv64-unknown-elf-objcopy -O binary bootloader.elf bootloader.bin
$ ls -lh bootloader.bin
-rw-r--r-- 1 user user 4.0K ...
```

Положите `bootloader.bin` по адресу `0x80000000` (QEMU `-kernel` или raw flash),
а `kernel.elf` - на VirtIO блочное устройство. Загрузчик найдёт устройство
через FDT, загрузит ELF и передаст управление.

---

## Зависимости

| Инструмент                    | Назначение              |
|-------------------------------|-------------------------|
| `riscv64-unknown-elf-g++`     | Кросс-компилятор        |
| `riscv64-unknown-elf-objcopy` | Извлечение бинарника    |
| `make`                        | Сборка                  |

Префикс тулчейна меняется через `CROSS`:
```console
make CROSS=riscv64-elf
```

---

## Структура проекта

| Файл                   | Назначение                                        |
|------------------------|---------------------------------------------------|
| `src/boot_entry.cpp`   | Голая `_start`: обнуление BSS, стек, вызов `boot_main` |
| `src/boot_main.cpp`    | Основная последовательность загрузки (FDT, VirtIO, ELF) |
| `include/uart.hpp`     | Драйвер NS16550A UART                             |
| `include/virtio.hpp`   | Драйвер VirtIO v2 modern MMIO block               |
| `include/elf.hpp`      | Парсер ELF64 (копирование PT_LOAD)                |
| `include/fdt.hpp`      | Парсер Flattened Device Tree                      |
| `linker.ld`            | Скрипт линковки (`BASE_ADDRESS = 0x80000000`)     |
| `Makefile`             | Правила кросс-компиляции                          |

---

## Технические детали

- **Без стандартной библиотеки** - нет libc, libstdc++, глобальных конструкторов, статических объектов.
- **Нет файлов `.S`** - весь привилегированный код (BSS, стек, WFI) в `boot_entry.cpp` через inline `asm`.
- **FDT discovery** - сканирует device tree в поисках `"ns16550a"` (UART) и `"virtio,mmio"` (VirtIO).
- **VirtIO v2 только** - современный MMIO интерфейс, обязателен `VIRTIO_F_VERSION_1`.
- **ELF64 загрузчик** - итерирует `PT_LOAD` сегменты, копирует в целевую память, вызывает `entry(hart_id=0, fdt_addr)`.
- **Стек** - 4 КБ. **Лимит бинарника** - 32 КБ. Текущая сборка ~4 КБ.
- **Лицензия** - GPL-3.0.

---

## Лицензия

GPL-3.0-or-later. См. [LICENSE](LICENSE).
