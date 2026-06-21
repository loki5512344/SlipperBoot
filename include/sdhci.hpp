#pragma once
#include <cstdint>

static const uint32_t SDHCI_DMA_ADDR    = 0x00;
static const uint32_t SDHCI_BLK_SIZE    = 0x04;
static const uint32_t SDHCI_BLK_CNT     = 0x06;
static const uint32_t SDHCI_ARGUMENT    = 0x08;
static const uint32_t SDHCI_TRANS_MODE  = 0x0C;
static const uint32_t SDHCI_COMMAND     = 0x0E;
static const uint32_t SDHCI_RESP0       = 0x10;
static const uint32_t SDHCI_RESP1       = 0x14;
static const uint32_t SDHCI_RESP2       = 0x18;
static const uint32_t SDHCI_RESP3       = 0x1C;
static const uint32_t SDHCI_DATA_PORT   = 0x20;
static const uint32_t SDHCI_PRES_STATE  = 0x24;
static const uint32_t SDHCI_PWR_CTRL    = 0x28;
static const uint32_t SDHCI_CLK_CTRL    = 0x2C;
static const uint32_t SDHCI_TIMEOUT_CTRL= 0x2E;
static const uint32_t SDHCI_SW_RESET    = 0x2F;
static const uint32_t SDHCI_INT_STAT    = 0x30;
static const uint32_t SDHCI_INT_EN      = 0x34;
static const uint32_t SDHCI_SIG_EN      = 0x38;
static const uint32_t SDHCI_CAPS        = 0x40;

static const uint32_t PRES_CMD_INHIBIT  = 1 << 0;
static const uint32_t PRES_DAT_INHIBIT  = 1 << 1;
static const uint32_t PRES_CARD_INSERT  = 1 << 16;
static const uint32_t PRES_CARD_STABLE  = 1 << 17;

static const uint32_t INT_CMD_COMPLETE  = 1 << 0;
static const uint32_t INT_XFER_COMPLETE = 1 << 1;
static const uint32_t INT_BUF_READ      = 1 << 4;
static const uint32_t INT_BUF_WRITE     = 1 << 5;
static const uint32_t INT_ERROR         = 1 << 15;

static const uint8_t SD_CMD0   = 0;
static const uint8_t SD_CMD2   = 2;
static const uint8_t SD_CMD3   = 3;
static const uint8_t SD_CMD7   = 7;
static const uint8_t SD_CMD8   = 8;
static const uint8_t SD_CMD9   = 9;
static const uint8_t SD_CMD12  = 12;
static const uint8_t SD_CMD16  = 16;
static const uint8_t SD_CMD17  = 17;
static const uint8_t SD_CMD55  = 55;
static const uint8_t SD_ACMD41 = 41;

struct SDHCI {
    volatile uint32_t* regs;
    uint16_t rca;
    bool sd_mode;

    SDHCI(uint64_t base)
        : regs((volatile uint32_t*)base)
        , rca(0) {}

    void wr(uint32_t off, uint32_t v) { regs[off / 4] = v; }
    uint32_t rd(uint32_t off) { return regs[off / 4]; }

    bool wait_bits(uint32_t mask, bool set, uint32_t timeout) {
        for (uint32_t i = 0; i < timeout; i++) {
            if (!!(rd(SDHCI_PRES_STATE) & mask) == set) return true;
        }
        return false;
    }

    bool wait_int(uint32_t want, uint32_t timeout) {
        for (uint32_t i = 0; i < timeout; i++) {
            uint32_t st = rd(SDHCI_INT_STAT);
            if (st & INT_ERROR) { wr(SDHCI_INT_STAT, st & INT_ERROR); return false; }
            if (st & want) { wr(SDHCI_INT_STAT, st & (want | INT_ERROR)); return true; }
        }
        return false;
    }

    bool send_cmd(uint8_t idx, uint32_t arg, int resp_type) {
        if (!wait_bits(PRES_CMD_INHIBIT, false, 100000))
            return false;

        uint16_t cmd = idx;
        cmd |= (uint16_t)(resp_type & 3) << 6;
        if (resp_type == 1 || resp_type == 3) cmd |= 1 << 8;
        if (resp_type == 3) cmd |= 1 << 9;
        if (resp_type < 0) { cmd |= 1 << 10; resp_type = -resp_type; }

        wr(SDHCI_ARGUMENT, arg);
        wr(SDHCI_COMMAND, cmd);

        if (resp_type != 0)
            return wait_int(INT_CMD_COMPLETE, 200000);
        for (volatile int i = 0; i < 500; i++);
        return true;
    }

    uint32_t resp(int n) { return rd(SDHCI_RESP0 + n * 4); }

    bool probe() {
        wr(SDHCI_SW_RESET, 7);
        for (int i = 0; i < 10000; i++)
            if (!(rd(SDHCI_SW_RESET) & 7)) break;

        wr(SDHCI_INT_EN, 0xFFFF);
        wr(SDHCI_SIG_EN, 0xFFFF);
        wr(SDHCI_INT_STAT, 0xFFFF);

        wr(SDHCI_PWR_CTRL, 0x0E);
        wr(SDHCI_CLK_CTRL, 0xFA);
        for (int i = 0; i < 10000; i++)
            if (rd(SDHCI_CLK_CTRL) & 2) break;

        wr(SDHCI_TIMEOUT_CTRL, 0x0F);

        if (!(rd(SDHCI_PRES_STATE) & PRES_CARD_INSERT))
            return false;

        // CMD0: go idle
        send_cmd(SD_CMD0, 0, 0);

        // CMD8: check SDHC
        if (!send_cmd(SD_CMD8, 0x1AA, 3))
            return false;
        if ((resp(0) & 0xFFF) != 0x1AA)
            return false;

        // ACMD41: init with voltage window + HCS
        int ok = 0;
        for (int i = 0; i < 1000; i++) {
            send_cmd(SD_CMD55, 0, 3);
            if (!send_cmd(SD_ACMD41, 0x40FF8000, 2))
                continue;
            if (resp(0) & (1 << 31)) { ok = 1; break; }
        }
        if (!ok) return false;
        sd_mode = (resp(0) & (1 << 30)) != 0;

        // CMD2: CID
        if (!send_cmd(SD_CMD2, 0, 1)) return false;
        // CMD3: RCA
        if (!send_cmd(SD_CMD3, 0, 3)) return false;
        rca = (uint16_t)(resp(0) >> 16);
        // CMD9: CSD
        if (!send_cmd(SD_CMD9, (uint32_t)rca << 16, 1)) return false;
        // CMD7: select
        if (!send_cmd(SD_CMD7, (uint32_t)rca << 16, 3)) return false;
        // CMD16: block size
        if (!send_cmd(SD_CMD16, 512, 3)) return false;

        return true;
    }

    bool read_sector(uint64_t lba, void* buf) {
        uint32_t arg = sd_mode ? (uint32_t)lba : (uint32_t)(lba * 512);

        wr(SDHCI_BLK_SIZE, 512);
        wr(SDHCI_BLK_CNT, 1);
        // read, block count enable, single
        wr(SDHCI_TRANS_MODE, (1 << 4) | (1 << 1));

        if (!send_cmd(SD_CMD17, arg, -3))
            return false;

        if (!wait_int(INT_BUF_READ, 500000))
            return false;

        uint32_t* dst = (uint32_t*)buf;
        for (int i = 0; i < 128; i++)
            dst[i] = rd(SDHCI_DATA_PORT);

        return wait_int(INT_XFER_COMPLETE, 2000000);
    }
};
