/*
 * Windows 7 Boot Updater (github.com/coderforlife/windows-7-boot-updater)
 * Copyright (C) 2021  Jeffrey Bush - Coder for Life
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

// The generic version of a few errors, not used publicly
#define GEN_ERR_CREATE_BACKUP		0x01
#define GEN_ERR_PEFILE_LOAD			0x02
#define GEN_ERR_PEFILE_INVALID_RES	0x03
#define GEN_ERR_PEFILE_INVALID_VER	0x04
#define GEN_ERR_PEFILE_MOD_RES		0x05
#define GEN_ERR_PEFILE_CLEAR_CERT	0x06
#define GEN_ERR_PEFILE_SAVE			0x07
#define GEN_ERR_DEACTIVE_SEC		0x08
#define GEN_ERR_RESTORE_SEC			0x09

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS				0x00
#endif

// Bootres errors
#define ERROR_BOOTRES_BASE			0x10
#define ERROR_BOOTRES_BACKUP		GEN_ERR_CREATE_BACKUP		+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_LOAD			GEN_ERR_PEFILE_LOAD			+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_INVALID_RES	GEN_ERR_PEFILE_INVALID_RES	+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_INVALID_VER	GEN_ERR_PEFILE_INVALID_VER	+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_ADD_WIM		GEN_ERR_PEFILE_MOD_RES		+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_CLEAR_CERT	GEN_ERR_PEFILE_CLEAR_CERT	+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_SAVE			GEN_ERR_PEFILE_SAVE			+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_DEACTIVE_SEC	GEN_ERR_DEACTIVE_SEC		+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_RESTORE_SEC	GEN_ERR_RESTORE_SEC			+ ERROR_BOOTRES_BASE
#define ERROR_BOOTRES_ANIM_SAVE		0x1A
#define ERROR_BOOTRES_WIM_CAPTURE	0x1B
#define ERROR_BOOTRES_WIM_READ		0x1C

// Winload errors
#define ERROR_WINLOAD_BASE			0x30
#define ERROR_WINLOAD_BACKUP		GEN_ERR_CREATE_BACKUP		+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_LOAD			GEN_ERR_PEFILE_LOAD			+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_INVALID_RES	GEN_ERR_PEFILE_INVALID_RES	+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_INVALID_VER	GEN_ERR_PEFILE_INVALID_VER	+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_MOD_RES		GEN_ERR_PEFILE_MOD_RES		+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_CLEAR_CERT	GEN_ERR_PEFILE_CLEAR_CERT	+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_SAVE			GEN_ERR_PEFILE_SAVE			+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_DEACTIVE_SEC	GEN_ERR_DEACTIVE_SEC		+ ERROR_WINLOAD_BASE
#define ERROR_WINLOAD_RESTORE_SEC	GEN_ERR_RESTORE_SEC			+ ERROR_WINLOAD_BASE
#define	ERROR_WINLOAD_IS_MUI		0x3A
#define	ERROR_WINLOAD_COPYRIGHT		0x3B
#define ERROR_WINLOAD_MSG_TBL		0x3C
#define ERROR_WINLOAD_XSL			0x3D
#define ERROR_WINLOAD_PROP			0x3E
#define ERROR_WINLOAD_HACK			0x3F
#define ERROR_WINLOAD_FONT			0x40

// Winload MUI errors
#define ERROR_WINLOAD_MUI_BASE			0x50
#define ERROR_WINLOAD_MUI_BACKUP		GEN_ERR_CREATE_BACKUP		+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_LOAD			GEN_ERR_PEFILE_LOAD			+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_INVALID_RES	GEN_ERR_PEFILE_INVALID_RES	+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_INVALID_VER	GEN_ERR_PEFILE_INVALID_VER	+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_MOD_RES		GEN_ERR_PEFILE_MOD_RES		+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_CLEAR_CERT	GEN_ERR_PEFILE_CLEAR_CERT	+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_SAVE			GEN_ERR_PEFILE_SAVE			+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_DEACTIVE_SEC	GEN_ERR_DEACTIVE_SEC		+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_RESTORE_SEC	GEN_ERR_RESTORE_SEC			+ ERROR_WINLOAD_MUI_BASE
#define ERROR_WINLOAD_MUI_IS_NOT_MUI	0x5A
#define ERROR_WINLOAD_MUI_MSG_TBL		0x5C

// Winresume errors
#define ERROR_WINRESUME_BASE			0x70
#define ERROR_WINRESUME_BACKUP			GEN_ERR_CREATE_BACKUP		+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_LOAD			GEN_ERR_PEFILE_LOAD			+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_INVALID_RES		GEN_ERR_PEFILE_INVALID_RES	+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_INVALID_VER		GEN_ERR_PEFILE_INVALID_VER	+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_MOD_RES			GEN_ERR_PEFILE_MOD_RES		+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_CLEAR_CERT		GEN_ERR_PEFILE_CLEAR_CERT	+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_SAVE			GEN_ERR_PEFILE_SAVE			+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_DEACTIVE_SEC	GEN_ERR_DEACTIVE_SEC		+ ERROR_WINRESUME_BASE
#define ERROR_WINRESUME_RESTORE_SEC		GEN_ERR_RESTORE_SEC			+ ERROR_WINRESUME_BASE
#define	ERROR_WINRESUME_IS_MUI			0x7A
#define	ERROR_WINRESUME_COPYRIGHT		0x7B
#define ERROR_WINRESUME_MSG_TBL			0x7C
#define ERROR_WINRESUME_XSL				0x7D
#define ERROR_WINRESUME_PROP			0x7E
#define ERROR_WINRESUME_HACK			0x7F
#define ERROR_WINRESUME_FONT			0x80

// Winresume MUI errors
#define ERROR_WINRESUME_MUI_BASE			0x90
#define ERROR_WINRESUME_MUI_BACKUP			GEN_ERR_CREATE_BACKUP		+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_LOAD			GEN_ERR_PEFILE_LOAD			+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_INVALID_RES		GEN_ERR_PEFILE_INVALID_RES	+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_INVALID_VER		GEN_ERR_PEFILE_INVALID_VER	+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_MOD_RES			GEN_ERR_PEFILE_MOD_RES		+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_CLEAR_CERT		GEN_ERR_PEFILE_CLEAR_CERT	+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_SAVE			GEN_ERR_PEFILE_SAVE			+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_DEACTIVE_SEC	GEN_ERR_DEACTIVE_SEC		+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_RESTORE_SEC		GEN_ERR_RESTORE_SEC			+ ERROR_WINRESUME_MUI_BASE
#define ERROR_WINRESUME_MUI_IS_NOT_MUI		0x9A
#define ERROR_WINRESUME_MUI_MSG_TBL			0x9C

// Bootmgr errors
#define	ERROR_BOOTMGR_BASE			0xB0
#define ERROR_BOOTMGR_BACKUP		GEN_ERR_CREATE_BACKUP		+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_LOAD			GEN_ERR_PEFILE_LOAD			+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_INVALID_RES	GEN_ERR_PEFILE_INVALID_RES	+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_INVALID_VER	GEN_ERR_PEFILE_INVALID_VER	+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_MOD_RES		GEN_ERR_PEFILE_MOD_RES		+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_CLEAR_CERT	GEN_ERR_PEFILE_CLEAR_CERT	+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_SAVE			GEN_ERR_PEFILE_SAVE			+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_DEACTIVE_SEC	GEN_ERR_DEACTIVE_SEC		+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_RESTORE_SEC	GEN_ERR_RESTORE_SEC			+ ERROR_BOOTMGR_BASE
#define ERROR_BOOTMGR_HACK			0xBA
#define ERROR_BOOTMGR_COMPRESS		0xBB

// File system errors
#define ERROR_GET_TEMP_FILE			0xD1
#define ERROR_GET_TEMP_DIR			0xD2
#define ERROR_DEL_TEMP_FILE			0xD3

#define ERROR_THROWN				0xFFFF
