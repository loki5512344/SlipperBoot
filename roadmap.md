# Roadmap

## v0.2 — Текущий костяк
- [x] UART драйвер (NS16550A, адрес из FDT)
- [x] VirtIO v2 MMIO block (probe, read_sector)
- [x] ELF64 парсер (header, program headers, load)
- [x] FDT парсер (memory, UART, VirtIO)
- [x] Точка входа _start (BSS, стек, прыжок)
- [ ] **Пофиксить vring адреса** — Used ring пишется в 0x090 вместо 0x0A0
- [ ] Проверить на QEMU: собрать дискету/образ с kernel.elf
- [ ] Проверить на OC2r/Sedna: FDT с реальными адресами

## v0.3 — Стабильность
- [ ] Boot menu через UART (выбор устройства/ядра)
- [ ] Поддержка нескольких VirtIO устройств (не только первое)
- [ ] Загрузка по LBA/имени файла (слабая ФС)
- [ ] Проверка целостности ELF (checksum)
- [ ] Прогресс-бар загрузки

## v0.4 — SlipperBoot standalone
- [ ] Свой образ диска (genimage или скрипт)
- [ ] Поддержка 9P (VirtIO filesystem) для отладки
- [ ] Поддержка network boot (VirtIO net) — опционально
- [ ] Debug UART (вывод FDT, регистров)
- [ ] CI сборка (GitHub Actions)

## v1.0 — Релиз
- [ ] Документация: все регистры, FDT, протокол
- [ ] Полная проверка на OC2r
- [ ] Интеграция с SlipperOS (передача FDT, BootInfo)
