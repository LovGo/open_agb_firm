#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2021 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types.h"


// Possible error codes for most of the functions below.
enum
{
	SDMMC_ERR_NONE           =  0u, // No error.
	SDMMC_ERR_INVAL_PARAM    =  1u, // Invalid parameter.
	SDMMC_ERR_INITIALIZED    =  2u, // The device is already initialized.
	SDMMC_ERR_GO_IDLE_STATE  =  3u, // GO_IDLE_STATE CMD error.
	SDMMC_ERR_SEND_IF_COND   =  4u, // SEND_IF_COND CMD error.
	SDMMC_ERR_IF_COND_RESP   =  5u, // IF_COND response pattern mismatch or unsupported voltage.
	SDMMC_ERR_SEND_OP_COND   =  6u, // SEND_OP_COND CMD error.
	SDMMC_ERR_OP_COND_TMOUT  =  7u, // Card initialization timeout.
	SDMMC_ERR_VOLT_SUPPORT   =  8u, // Voltage not supported.
	SDMMC_ERR_ALL_SEND_CID   =  9u, // ALL_SEND_CID CMD error.
	SDMMC_ERR_SET_SEND_RCA   = 10u, // SET/SEND_RELATIVE_ADDR CMD error.
	SDMMC_ERR_SEND_CSD       = 11u, // SEND_CSD CMD error.
	SDMMC_ERR_SELECT_CARD    = 12u, // SELECT_CARD CMD error.
	SDMMC_ERR_LOCKED         = 13u, // Card is locked with a password.
    SDMMC_ERR_SEND_EXT_CSD   = 14u, // SEND_EXT_CSD CMD error.
    SDMMC_ERR_SWITCH_HS      = 15u, // Error on switching to high speed mode.
	SDMMC_ERR_SET_CLR_CD     = 16u, // SET_CLR_CARD_DETECT CMD error.
	SDMMC_ERR_SET_BUS_WIDTH  = 17u, // Error on switching to a different bus width.
	SDMMC_ERR_SEND_STATUS    = 18u, // SEND_STATUS CMD error.
	SDMMC_ERR_CARD_STATUS    = 19u, // The card returned an error via its status.
	SDMMC_ERR_NO_CARD        = 20u, // Card unitialized or not inserted.
	SDMMC_ERR_SECT_RW        = 21u, // Sector read/write error.
	SDMMC_ERR_WRITE_PROT     = 22u  // The card is write protected.
};

// (e)MMC/SD device numbers.
enum
{
	SDMMC_DEV_CARD = 0u, // SD card/MMC.
	SDMMC_DEV_eMMC = 1u, // Builtin eMMC.

    // Alias for internal use only.
    SDMMC_MAX_DEV_NUM = SDMMC_DEV_eMMC
};

// Bit definition for SdmmcInfo.wrProt and SDMMC_getWriteProtBits().
// Each bit 1 = protected.
#define SDMMC_WR_PROT_SLIDER  (1u)    // SD card write protection slider.
#define SDMMC_WR_PROT_TEMP    (1u<<1) // Temporary write protection (CSD).
#define SDMMC_WR_PROT_PERM    (1u<<2) // Permanent write protection (CSD).

typedef struct
{
	u8 type;     // 0 = none, 1 = (e)MMC, 2 = High capacity (e)MMC, 3 = SDSC, 4 = SDHC/SDXC, 5 = SDUC.
	u8 wrProt;   // See SDMMC_WR_PROT_ defines above for details.
	u16 rca;     // Relative Card Address (RCA).
	u32 sectors; // Size in 512 byte units.
	u32 clock;   // The current clock frequency in Hz.
	u32 cid[4];  // Raw CID without the CRC.
	u16 ccc;     // (e)MMC/SD command class support from CSD. One per bit starting at 0.
	u8 busWidth; // The current bus width used to talk to the card.
} SdmmcInfo;



/**
 * @brief      Initializes a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device to initialize.
 *
 * @return     Returns SDMMC_ERR_NONE on success or
 *             one of the errors listed above on failure.
 */
u32 SDMMC_init(const u8 devNum);

/**
 * @brief      Deinitializes a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device to deinitialize.
 *
 * @return     Returns SDMMC_ERR_NONE on success or SDMMC_ERR_INVAL_PARAM on failure.
 */
u32 SDMMC_deinit(const u8 devNum);

/**
 * @brief      Outputs infos about a (e)MMC/SD card device.
 *
 * @param[in]  devNum   The device.
 * @param      infoOut  A pointer to a SdmmcInfo struct.
 *
 * @return     Returns SDMMC_ERR_NONE on success or SDMMC_ERR_INVAL_PARAM on failure.
 */
u32 SDMMC_getDevInfo(const u8 devNum, SdmmcInfo *const infoOut);

/**
 * @brief      Outputs the CID of a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device.
 * @param      cidOut  A u32[4] pointer for storing the CID.
 *
 * @return     Returns SDMMC_ERR_NONE on success or SDMMC_ERR_INVAL_PARAM on failure.
 */
u32 SDMMC_getCid(const u8 devNum, u32 cidOut[4]);

/**
 * @brief      Returns the write protection bits of a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device.
 *
 * @return     Returns the write protection bits or 0xFF on failure.
 *             See SDMMC_WR_PROT_ defines above for details.
 */
u8 SDMMC_getWriteProtBits(const u8 devNum);

/**
 * @brief      Outputs the number of sectors for a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device.
 *
 * @return     Returns the number of sectors or 0 on failure.
 */
u32 SDMMC_getSectors(const u8 devNum);

/**
 * @brief      Reads one or more sectors from a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device.
 * @param[in]  sect    The start sector.
 * @param      buf     The output buffer pointer. NULL for DMA.
 * @param[in]  count   The number of sectors to read.
 *
 * @return     Returns SDMMC_ERR_NONE on success or
 *             one of the errors listed above on failure.
 */
u32 SDMMC_readSectors(const u8 devNum, u32 sect, u32 *const buf, const u16 count);

/**
 * @brief      Writes one or more sectors to a (e)MMC/SD card device.
 *
 * @param[in]  devNum  The device.
 * @param[in]  sect    The start sector.
 * @param[in]  buf     The input buffer pointer. NULL for DMA.
 * @param[in]  count   The count
 *
 * @return     Returns SDMMC_ERR_NONE on success or
 *             one of the errors listed above on failure.
 */
u32 SDMMC_writeSectors(const u8 devNum, u32 sect, const u32 *const buf, const u16 count);

// TODO: TRIM/erase support.
