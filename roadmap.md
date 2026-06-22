# Roadmap

## v0.2 - OC2r (VirtIO)
- [x] UART драйвер (NS16550A, reg-shift из FDT)
- [x] VirtIO v1+v2 MMIO block (probe, read_sector)
- [x] ELF64 парсер (header, program headers, load, check_safe)
- [x] FDT парсер (memory, UART, VirtIO, SDHCI, model)
- [x] Точка входа _start (BSS, стек, прыжок, hartid из a0)
- [x] Проверено на QEMU riscv64 virt

## v0.3 - Milk-V Duo S (SDHCI)
- [x] SDHCI драйвер
- [x] DTB от предыдущей стадии (a1), единая сборка
- [x] Загрузка kernel.elf с блочного устройства
- [ ] Проверить на реальной Milk-V Duo S (нет железа)

## v0.4 - Стабильность
- [x] Boot menu с timeout auto-select
- [x] Поддержка нескольких устройств
- [x] PHDRS в linker.ld (без RWX warning)
- [x] S-mode совместимость (a0 вместо csrr mhartid)
- [x] UART reg-shift из FDT
- [x] Универсальный бинарник (без compile-time platform)
- [x] Убран 2MB static buffer из BSS
- [x] nsectors ограничен по DRAM size

## v0.5 - Инфраструктура
- [x] genimage + mkimage
- [x] 9P документация (комментарий в run_qemu.sh)
- [x] Debug UART (debug.hpp, guarded by -DDEBUG)
- [x] CI (GitHub Actions, .github/workflows/test.yml)
- [x] Документация (README)

## v1.0 - Релиз
- [ ] Проверка на реальном Milk-V Duo S
- [ ] Стабильная интеграция с SlipperOS
- [ ] Полное покрытие тестами
