#include "regs.h"
#include "io.h"
#include "mailbox.h"
#include <string.h>

#define SD_TM_BLKCNT_ENABLE				(1 << 1)
#define SD_TM_AUTO_CMD_CMD12			(0b10 << 2)
#define SD_TM_DIR_CARD_TO_HOST			(1 << 4)
#define SD_TM_MULTI_BLOCK				(1 << 5)
#define SD_CMD_CRC_CHECK_ENABLE			(1 << 19)
#define SD_CMD_ISDATA					(1 << 21)
#define SD_CMD_RESPONSE_TYPE_136BIT		(0b01 << 16)
#define SD_CMD_RESPONSE_TYPE_48BIT		(0b10 << 16)
#define SD_CMD_RESPONSE_TYPE_48BIT_BUSY	(0b11 << 16)
#define SD_CMD_INDEX_GO_IDLE_STATE		0
#define SD_CMD_INDEX_SEND_IF_COND		(8 << 24)
#define SD_CMD_INDEX_READ_SINGLE_BLOCK	(17 << 24)
#define SD_CMD_INDEX_READ_MULTI_BLOCK	(18 << 24)
#define SD_CMD_INDEX_WRITE_SINGLE_BLOCK	(24 << 24)
#define SD_CMD_INDEX_WRITE_MULTI_BLOCK	(25 << 24)
#define SD_CMD_INDEX_APP_CMD			(55 << 24)
#define SD_CMD_INDEX_ACMD41				(41 << 24)
#define SD_CMD_INDEX_SEND_RELATIVE_ADDR	(3 << 24)
#define SD_CMD_INDEX_SELECT_CARD		(7 << 24)
#define SD_CMD_INDEX_ALL_SEND_CID		(2 << 24)
#define SD_CMD_INDEX_STOP_TRANSMISSION	(12 << 24)

#define SD_CMD_GO_IDLE_STATE			SD_CMD_INDEX_GO_IDLE_STATE
#define SD_CMD_SEND_IF_COND				(SD_CMD_INDEX_SEND_IF_COND | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_48BIT)
#define SD_CMD_APP_CMD					(SD_CMD_INDEX_APP_CMD | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_48BIT)
#define SD_CMD_SEND_RELATIVE_ADDR		(SD_CMD_INDEX_SEND_RELATIVE_ADDR | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_48BIT)
#define SD_CMD_SELECT_CARD				(SD_CMD_INDEX_SELECT_CARD | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_48BIT_BUSY)
#define SD_CMD_ALL_SEND_CID				(SD_CMD_INDEX_ALL_SEND_CID | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_136BIT)
#define SD_CMD_READ_MULTI_BLOCK			(SD_CMD_INDEX_READ_MULTI_BLOCK | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_48BIT | SD_CMD_ISDATA | SD_TM_BLKCNT_ENABLE /*| SD_TM_AUTO_CMD_CMD12*/ | SD_TM_DIR_CARD_TO_HOST | SD_TM_MULTI_BLOCK)
#define SD_CMD_STOP_TRANSMISSION		(SD_CMD_INDEX_STOP_TRANSMISSION | SD_CMD_CRC_CHECK_ENABLE | SD_CMD_RESPONSE_TYPE_48BIT_BUSY)

#define SD_ACMD41						(SD_CMD_INDEX_ACMD41 | SD_CMD_RESPONSE_TYPE_48BIT)

#define SD_INTERRUPT_CMD_DONE			(1 << 0)
#define SD_INTERRUPT_DATA_DONE			(1 << 1)
#define SD_INTERRUPT_READ_READY			(1 << 5)
#define SD_INTERRUPT_ERROR				(1 << 15)

#define SD_STATUS_DAT3					(1 << 23)
#define SD_STATUS_CMD_INHIBIT			(1 << 0)
#define SD_STATUS_DAT_INHIBIT			(1 << 1)
#define SD_STATUS_READ_TRANSFER			(1 << 9)

#define SD_CONTROL1_INTERNAL_CLOCK_ENABLE	(1 << 0)
#define SD_CONTROL1_CLOCK_STABLE			(1 << 1)
#define SD_CONTROL1_CLOCK_ENABLE			(1 << 2)
#define SD_CONTROL1_RESET_HOST_CIRCUIT		(1 << 24)

static u16	_rca;
static bool	_high_capacity;

// TODO: should probably round the integer division up
static void usleep(u32 microseconds) {
	// 0x100000 ticks -> one second
	const u32 end_time = SYS_TIMER->clo + ((0x100000 / 1000000) * microseconds);
	while (SYS_TIMER->clo < end_time) {
	}
}

#define TIMEOUT	0x80000;
static bool wait_flag_set(volatile u32* reg, u32 flag) {
	const u32 timeout = SYS_TIMER->clo + TIMEOUT;
	while (SYS_TIMER->clo < timeout) {
		if ((*reg & flag) == flag) {
			return true;
		}
	}
	return false;
}

static bool wait_flag_clear(volatile u32* reg, u32 flag) {
	const u32 timeout = SYS_TIMER->clo + TIMEOUT;
	while (SYS_TIMER->clo < timeout) {
		if (!(*reg & flag)) {
			return true;
		}
	}
	return false;
}

static void emmc_dump_regs(void) {
	print("======= EMMC REGISTERS =======\n");
	printf("arg2: %\n", (u64)EMMC->arg2);
	printf("blk_size_cnt: %\n", (u64)EMMC->blk_size_cnt);
	printf("arg1: %\n", (u64)EMMC->arg1);
	printf("cmd_tm: %\n", (u64)EMMC->cmd_tm);
	printf("resp0: %\n", (u64)EMMC->resp0);
	printf("resp1: %\n", (u64)EMMC->resp1);
	printf("resp2: %\n", (u64)EMMC->resp2);
	printf("resp3: %\n", (u64)EMMC->resp3);
	printf("status: %\n", (u64)EMMC->status);
	printf("control0: %\n", (u64)EMMC->control0);
	printf("control1: %\n", (u64)EMMC->control1);
	printf("interrupt: %\n", (u64)EMMC->interrupt);
	printf("irpt_mask: %\n", (u64)EMMC->irpt_mask);
	printf("irpt_en: %\n", (u64)EMMC->irpt_en);
	printf("control2: %\n", (u64)EMMC->control2);
}

// NOTE: block commands will require blksizecnt to be configured beforehand
static bool sd_send_cmd(u32 cmd_tm, u32 arg, u8* response) {
	/*
	u32 inhibits = SD_STATUS_CMD_INHIBIT;
	if (((cmd_tm & SD_CMD_RESPONSE_TYPE_48BIT_BUSY) == SD_CMD_RESPONSE_TYPE_48BIT_BUSY) || (cmd_tm & SD_CMD_ISDATA)) {
		inhibits |= SD_STATUS_DAT_INHIBIT;
	}
	if (!wait_flag_clear(&EMMC->status, inhibits)) {
		print("[sd_send_cmd] timeout while waiting for inhibits\n");
		emmc_dump_regs();
		return false;
	}
	*/

	if (EMMC->interrupt & SD_INTERRUPT_ERROR) {
		print("[sd_send_cmd] INTERRUPT register indicates an error occurred\n");
		emmc_dump_regs();
		return false;
	}

	EMMC->interrupt = ~0;
	EMMC->arg1 = arg;
	EMMC->cmd_tm = cmd_tm;

	if (!wait_flag_set(&EMMC->interrupt, SD_INTERRUPT_CMD_DONE)) {
		print("[sd_send_cmd] timeout while waiting for CMD_DONE\n");
		emmc_dump_regs();
		return false;
	}

	if (EMMC->interrupt & SD_INTERRUPT_ERROR) {
		print("[sd_send_cmd] INTERRUPT register indicates an error occurred\n");
		emmc_dump_regs();
		return false;
	}

	if (response == NULL) {
		return true;
	}

	if (cmd_tm & SD_CMD_RESPONSE_TYPE_48BIT) {
		*((u32*)response) = EMMC->resp0;
		*((u16*)response + 2) = (u16)(EMMC->resp1 & 0xffff);
	}

	return true;
}

static bool sd_send_acmd(u32 cmd_tm, u32 arg, u8* response) {
	if (!sd_send_cmd(SD_CMD_APP_CMD, 0, NULL)) {
		return false;
	}
	return sd_send_cmd(cmd_tm, arg, response);
}

// NOTE: for now we will always have block_size = 512

// TODO: Try a multiple block read

// NOTE: block_count can be zero for single block transfers
static void set_blk_size_cnt(u16 block_size, u16 block_count) {
	EMMC->blk_size_cnt = (block_size & 0x3ff) | block_count << 16;
}

// NOTE: when running this command we expect to be in the TRANSFER state
bool sd_read_sectors(void* dest, u32 sector_index, u16 sector_count) {
	if (!_high_capacity) {
		sector_index *= 512;
	}
	if ((u64)dest & 0b11) {
		print("[sd_read_sectors] destination buffer is not 32-bit aligned\n");
		return false;
	}

	set_blk_size_cnt(512, sector_count);
	if (!sd_send_cmd(SD_CMD_READ_MULTI_BLOCK, sector_index, NULL)) {
		return false;
	}

	for (u32 i = 0; i < sector_count; ++i) {
		for (u32 j = 0; j < 512 / 4; ++j) {
			if (!wait_flag_set(&EMMC->interrupt, SD_INTERRUPT_READ_READY)) {
				print("[sd_read_sectors] incomplete read (timeout on READ_READY)\n");
				return false;
			}
			((u32*)(dest + 512 * i))[j] = EMMC->data;
		}
	}

	if (!sd_send_cmd(SD_CMD_STOP_TRANSMISSION, 0, NULL)) {
		return false;
	}

	if (!wait_flag_set(&EMMC->interrupt, SD_INTERRUPT_DATA_DONE)) {
		print("[sd_read_sectors] warning: timeout on DATA_DONE\n");
		return false;
	}

	return true;
}

static bool sd_set_clock_rate(u32 desired_rate) {
	EMMC->control1 &= ~(SD_CONTROL1_INTERNAL_CLOCK_ENABLE | SD_CONTROL1_CLOCK_ENABLE | (0xf << 16) | (0x3ff << 6));
	usleep(2000);

	EMMC->control1 = SD_CONTROL1_INTERNAL_CLOCK_ENABLE | (0xe << 16);
	//EMMC->control1 = SD_CONTROL1_INTERNAL_CLOCK_ENABLE | (0xf << 16);

	if (!wait_flag_set(&EMMC->control1, SD_CONTROL1_CLOCK_STABLE)) {
		print("[sd_init] timeout while waiting for clock to stabilize\n");
		return false;
	}

	u32 base_rate = mailbox_get_clock_rate(MAILBOX_CLOCK_ID_EMMC);
	printf("[sd_init] EMMC base clock rate is % Hz\n", (u64)base_rate);
	u32 divider = base_rate / desired_rate;
	printf("[sd_init] clock divider is %\n", (u64)divider);

	EMMC->control1 |= SD_CONTROL1_CLOCK_ENABLE | ((divider & 0x300) >> 2) | ((divider & 0xff) << 8);
	if (!wait_flag_set(&EMMC->control1, SD_CONTROL1_CLOCK_STABLE)) {
		print("[sd_init] timeout while waiting for clock to stabilize\n");
		return false;
	}

	return true;
}

bool sd_init(void) {
	EMMC->control0 = 0;
	EMMC->control1 = SD_CONTROL1_RESET_HOST_CIRCUIT;
	if (!wait_flag_clear(&EMMC->control1, SD_CONTROL1_RESET_HOST_CIRCUIT)) {
		print("[sd_init] EMMC reset timed out\n");
		return false;
	}

	// Check if Card Detect is held high
	// NOTE: it has not been tested whether this correctly detects when no card is inserted
	if (!(EMMC->status & SD_STATUS_DAT3)) {
		print("[sd_init] SD card not detected\n");
		return false;	
	}

	if (!sd_set_clock_rate(400000)) {
		return false;
	}

	// Enable which flags are to be set when events happen
	EMMC->irpt_mask = ~0;

	EMMC->interrupt = ~0;
	if (EMMC->interrupt & SD_INTERRUPT_ERROR) {
		print("[sd_init] INTERRUPT register indicates an error occurred\n");
		emmc_dump_regs();
		return false;
	}

	if (!sd_send_cmd(SD_CMD_GO_IDLE_STATE, 0, NULL)) {
		return false;
	}

	u8 response[6] = {};
	if (!sd_send_cmd(SD_CMD_SEND_IF_COND, 0x1aa, response)) {
		print("[sd_init] SD card version < 2.0 not supported\n");
		return false;
	}

	if (response[0] != 0xaa || !(response[1] & 1)) {
		print("[sd_init] invalid response for SEND_IF_COND\n");
		printf("[sd_init] response[0]: %\n", (u64)response[0]);
		printf("[sd_init] response[1]: %\n", (u64)response[1]);
		return false;
	}

	if (!sd_send_acmd(SD_ACMD41, 0, NULL)) {
		print("[sd_init] ACMD41 failed\n");
		return false;
	}

	// TODO: SD_OCR_ macros
	u32 ocr;
	const u32 tries = 4000;
	u32 i;
	for (i = 0; i < tries; ++i) {
		memset(response, 0, 6);

		// HCS (bit 30) = 1 indicates that we support high capacity cards
		if (!sd_send_acmd(SD_ACMD41, 0x00ff8000 | (1 << 30), response)) {
			print("[sd_init] ACMD41 failed\n");
			return false;
		}

		//ocr = response[1] | (response[2] << 8) | (response[3] << 16) | (response[4] << 24);
		ocr = *((u32*)response);
		// Check if card is busy
		if (ocr & (1 << 31)) {
			break;
		}
		usleep(2000);
	}
	if (i == tries) {
		print("[sd_init] timeout while waiting for card\n");
		return false;
	}

	_high_capacity = !!(ocr & (1 << 30));

	if (!sd_set_clock_rate(25000000)) {
		return false;
	}

	if (!sd_send_cmd(SD_CMD_ALL_SEND_CID, 0, NULL)) {
		print("[sd_init] ALL_SEND_CID failed\n");
		return false;
	}

	if (!sd_send_cmd(SD_CMD_SEND_RELATIVE_ADDR, 0, response)) {
		print("[sd_init] SEND_RELATIVE_ADDR failed\n");
		return false;
	}
	_rca = *((u16*)response + 1);
	printf("[sd_init] RCA = %\n", (u64)_rca);

	if (!sd_send_cmd(SD_CMD_SELECT_CARD, (u32)_rca << 16, NULL)) {
		print("[sd_init] SELECT_CARD failed\n");
		return false;
	}

	return true;
}
