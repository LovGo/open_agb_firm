/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2024 profi200
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

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "arm11/config.h"
#include "inih/ini.h"
#include "util.h"
#include "fsutil.h"


#define INI_BUF_SIZE    (1024u)
#define DEFAULT_CONFIG  "[general]\n"             \
                        "backlight=64\n"          \
                        "backlightSteps=5\n"      \
                        "directBoot=false\n"      \
                        "useGbaDb=true\n"         \
                        "useSavesFolder=true\n\n" \
                                                  \
                        "[video]\n"               \
                        "scaler=matrix\n"         \
                        "colorProfile=none\n"     \
                        "contrast=1.0\n"          \
                        "brightness=0.0\n"        \
                        "saturation=1.0\n\n"      \
                                                  \
                        "[audio]\n"               \
                        "audioOut=auto\n"         \
                        "volume=127\n\n"          \
                                                  \
                        "[advanced]\n"            \
                        "saveOverride=false\n"    \
                        "defaultSave=sram_256k"



// Default config.
OafConfig g_oafConfig =
{
	// [general]
	64,    // backlight
	5,     // backlightSteps
	false, // directBoot
	true,  // useGbaDb
	true,  // useSavesFolder

	// [video]
	2,     // scaler
	0,     // colorProfile
	1.f,   // contrast
	0.f,   // brightness
	1.f,   // saturation

	// [audio]
	0,     // Automatic audio output.
	127,   // Control via volume slider.

	// [input]
	{      // buttonMaps
		0, // A
		0, // B
		0, // Select
		0, // Start
		0, // Right
		0, // Left
		0, // Up
		0, // Down
		0, // R
		0  // L
	},

	{      // buttonComboMap
		0, 0, 0, 0, 0, 0, 0, 0,  // A,B,SELECT,START,RIGHT,LEFT,UP,DOWN
		0, 0, 0, 0, 0, 0, 0, 0,  // R,L,X,Y,_,_,ZL,ZR
		0, 0, 0, 0, 0, 0, 0, 0,  // _,_,_,_,TOUCH,_,_,_
		0, 0, 0, 0, 0, 0, 0, 0   // CS_RIGHT,CS_LEFT,CS_UP,CS_DOWN,CP_RIGHT,CP_LEFT,CP_UP,CP_DOWN
	},

	// [game]
	0,     // saveSlot
	255,   // saveType

	// [advanced]
	false, // saveOverride
	14     // defaultSave
};


static int findButton(const char* button, const char *const buttonStrLut[32])
{
    if(button == NULL || button[0] == '\0') return 0;

    for(unsigned i = 0; i < 32; i++)
    {
        if(buttonStrLut[i][0] != '\0' && strcmp(buttonStrLut[i], button) == 0)
        {
            return 1u << i;
        }
    }
    return 0;
}


static u32 parseButtons(const char *str)
{
	if(str == NULL || *str == '\0') return 0;

	char buf[32]; // Should be enough for all useful mappings.
	buf[31] = '\0';
	strncpy(buf, str, 31);

	char *bufPtr = buf;
	static const char *const buttonStrLut[32] =
	{
		"A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN",
		"R", "L", "X", "Y", "", "", "ZL", "ZR",
		"", "", "", "", "TOUCH", "", "", "",
		"CS_RIGHT", "CS_LEFT", "CS_UP", "CS_DOWN", "CP_RIGHT", "CP_LEFT", "CP_UP", "CP_DOWN"
	};

	u32 map = 0;
    char *token = strtok(buf, "+,");
    while(token)
    {
        // Only process non-empty tokens
        if(*token != '\0') map |= findButton(token, buttonStrLut);
        token = strtok(NULL, "+,");
    }

	return map & ~(1u<<12);
}

static int cfgIniCallback(void *user, const char *section, const char *name, const char *value)
{
	OafConfig *const config = (OafConfig*)user;

	if(strcmp(section, "general") == 0)
	{
		if(strcmp(name, "backlight") == 0)
			config->backlight = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "backlightSteps") == 0)
			config->backlightSteps = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "directBoot") == 0)
			config->directBoot = (strcmp(value, "false") == 0 ? false : true);
		else if(strcmp(name, "useGbaDb") == 0)
			config->useGbaDb = (strcmp(value, "true") == 0 ? true : false);
		else if(strcmp(name, "useSavesFolder") == 0)
			config->useSavesFolder = (strcmp(value, "true") == 0 ? true : false);
	}
	else if(strcmp(section, "video") == 0)
	{
		if(strcmp(name, "scaler") == 0)
		{
			if(strcmp(value, "none") == 0)
				config->scaler = 0;
			else if(strcmp(value, "bilinear") == 0)
				config->scaler = 1;
			else if(strcmp(value, "matrix") == 0)
				config->scaler = 2;
		}
		else if(strcmp(name, "colorProfile") == 0)
		{
			if(strcmp(value, "none") == 0)
				config->colorProfile = 0;
			else if(strcmp(value, "gba") == 0)
				config->colorProfile = 1;
			else if(strcmp(value, "gb_micro") == 0)
				config->colorProfile = 2;
			else if(strcmp(value, "gba_sp101") == 0)
				config->colorProfile = 3;
			else if(strcmp(value, "nds") == 0)
				config->colorProfile = 4;
			else if(strcmp(value, "ds_lite") == 0)
				config->colorProfile = 5;
			else if(strcmp(value, "nso") == 0)
				config->colorProfile = 6;
			else if(strcmp(value, "vba") == 0)
				config->colorProfile = 7;
			else if(strcmp(value, "identity") == 0)
				config->colorProfile = 8;
			//else if(strcmp(value, "custom") == 0) // TODO: Implement user provided profile.
			//	config->colorProfile = 9;
		}
		else if(strcmp(name, "contrast") == 0)
			config->contrast = str2float(value);
		else if(strcmp(name, "brightness") == 0)
			config->brightness = str2float(value);
		else if(strcmp(name, "saturation") == 0)
			config->saturation = str2float(value);
	}
	else if(strcmp(section, "audio") == 0)
	{
		if(strcmp(name, "audioOut") == 0)
		{
			if(strcmp(value, "auto") == 0)
				config->audioOut = 0;
			else if(strcmp(value, "speakers") == 0)
				config->audioOut = 1;
			else if(strcmp(value, "headphones") == 0)
				config->audioOut = 2;
		}
		else if(strcmp(name, "volume") == 0)
			config->volume = (s8)strtol(value, NULL, 10);
	}
	else if(strcmp(section, "input") == 0)
	{
		const u32 map = parseButtons(value);  // Parse 3DS buttons (right side in config)
		if(map != 0)
		{
			const u32 buttons = parseButtons(name) & 0x3FFu;  // Parse GBA buttons & only allow GBA buttons (left side in config)
			if(buttons != 0)
		{
				// Combination mapping
				if(strchr(name, '+') != NULL)
				{
					for(int i = 0; i < 32; i++)
					{
						if(map & (1u << i)) config->buttonComboMap[i] = buttons;
					}
				}
				else
				{
					// Single button mapping
					const u32 shift = 31u - __builtin_clzl(buttons);
			config->buttonMaps[shift] = map;
				}
			}
		}
	}
	else if(strcmp(section, "game") == 0)
	{
		if(strcmp(name, "saveSlot") == 0)
			config->saveSlot = (u8)strtoul(value, NULL, 10);
		if(strcmp(name, "saveType") == 0)
		{
			if(strcmp(value, "eeprom_8k") == 0)
				config->saveType = 0;
			if(strcmp(value, "rom_256m_eeprom_8k") == 0)
				config->saveType = 1;
			if(strcmp(value, "eeprom_64k") == 0)
				config->saveType = 2;
			if(strcmp(value, "rom_256m_eeprom_64k") == 0)
				config->saveType = 3;
			if(strcmp(value, "flash_512k_atmel_rtc") == 0)
				config->saveType = 4;
			if(strcmp(value, "flash_512k_atmel") == 0)
				config->saveType = 5;
			if(strcmp(value, "flash_512k_sst_rtc") == 0)
				config->saveType = 6;
			if(strcmp(value, "flash_512k_sst") == 0)
				config->saveType = 7;
			if(strcmp(value, "flash_512k_panasonic_rtc") == 0)
				config->saveType = 8;
			if(strcmp(value, "flash_512k_panasonic") == 0)
				config->saveType = 9;
			if(strcmp(value, "flash_1m_macronix_rtc") == 0)
				config->saveType = 10;
			if(strcmp(value, "flash_1m_macronix") == 0)
				config->saveType = 11;
			if(strcmp(value, "flash_1m_sanyo_rtc") == 0)
				config->saveType = 12;
			if(strcmp(value, "flash_1m_sanyo") == 0)
				config->saveType = 13;
			if(strcmp(value, "sram_256k") == 0)
				config->saveType = 14;
			if(strcmp(value, "none") == 0)
				config->saveType = 15;
			if(strcmp(value, "auto") == 0)
				config->saveType = 255;
		}
	}
	else if(strcmp(section, "advanced") == 0)
	{
		if(strcmp(name, "saveOverride") == 0)
			config->saveOverride = (strcmp(value, "false") == 0 ? false : true);
		if(strcmp(name, "defaultSave") == 0)
		{
			if(strcmp(value, "eeprom_8k") == 0)
				config->defaultSave = 0;
			if(strcmp(value, "rom_256m_eeprom_8k") == 0)
				config->defaultSave = 1;
			if(strcmp(value, "eeprom_64k") == 0)
				config->defaultSave = 2;
			if(strcmp(value, "rom_256m_eeprom_64k") == 0)
				config->defaultSave = 3;
			if(strcmp(value, "flash_512k_atmel_rtc") == 0)
				config->defaultSave = 4;
			if(strcmp(value, "flash_512k_atmel") == 0)
				config->defaultSave = 5;
			if(strcmp(value, "flash_512k_sst_rtc") == 0)
				config->defaultSave = 6;
			if(strcmp(value, "flash_512k_sst") == 0)
				config->defaultSave = 7;
			if(strcmp(value, "flash_512k_panasonic_rtc") == 0)
				config->defaultSave = 8;
			if(strcmp(value, "flash_512k_panasonic") == 0)
				config->defaultSave = 9;
			if(strcmp(value, "flash_1m_macronix_rtc") == 0)
				config->defaultSave = 10;
			if(strcmp(value, "flash_1m_macronix") == 0)
				config->defaultSave = 11;
			if(strcmp(value, "flash_1m_sanyo_rtc") == 0)
				config->defaultSave = 12;
			if(strcmp(value, "flash_1m_sanyo") == 0)
				config->defaultSave = 13;
			if(strcmp(value, "sram_256k") == 0)
				config->defaultSave = 14;
			if(strcmp(value, "none") == 0)
				config->defaultSave = 15;
		}
	}
	else return 0; // Error.

	return 1; // 1 is no error? Really?
}

// TODO: Instead of writing a hardcoded string turn default config into a string.
Result parseOafConfig(const char *const path, OafConfig *cfg, const bool newCfgOnError)
{
	char *iniBuf = (char*)calloc(INI_BUF_SIZE, 1);
	if(iniBuf == NULL) return RES_OUT_OF_MEM;

	cfg = (cfg != NULL ? cfg : &g_oafConfig);
	Result res = fsQuickRead(path, iniBuf, INI_BUF_SIZE - 1);
	if(res == RES_OK) ini_parse_string(iniBuf, cfgIniCallback, cfg);
	else if(newCfgOnError)
	{
		const char *const defaultConfig = DEFAULT_CONFIG;
		res = fsQuickWrite(path, defaultConfig, strlen(defaultConfig));
	}

	free(iniBuf);

	return res;
}