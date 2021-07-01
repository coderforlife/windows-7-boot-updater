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

#include "Zip.h"

#include "Utilities.h"

#include "Files.h"

using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Files;
using namespace Win7BootUpdater::Compression;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::IO::Compression;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#include <PshPack1.h>

//////////////////////////////////////////////////////////////////////////////
///// Zip Version Flags and Structure (commented ones are not supported)
//////////////////////////////////////////////////////////////////////////////
//#define VERSION_OS_FAT		0x00
//#define VERSION_OS_AMIGA		0x01
//#define VERSION_OS_OPENVMS	0x02
//#define VERSION_OS_UNIX		0x03
//#define VERSION_OS_VM_CMS		0x04
//#define VERSION_OS_ATARI_ST	0x05
//#define VERSION_OS_HPFS		0x06
//#define VERSION_OS_MAC		0x07
//#define VERSION_OS_Z_SYSTEM	0x08
//#define VERSION_OS_CP_M		0x09
//#define VERSION_OS_NTFS		0x0A
//#define VERSION_OS_MVS		0x0B
//#define VERSION_OS_VSE		0x0C
//#define VERSION_OS_ACORN_RISC	0x0D
//#define VERSION_OS_VFAT		0x0E
//#define VERSION_OS_ALT_MVS	0x0F
//#define VERSION_OS_BEOS		0x10
//#define VERSION_OS_TANDEM		0x11
//#define VERSION_OS_OS_400		0x12
//#define VERSION_OS_OS_X		0x13

#define ZIP_VERSION_SPEC(maj, min)	maj*10+min
#define ZIP_VERSION_SPEC_MAX					ZIP_VERSION_SPEC(2, 0)

#define ZIP_VERSION_SPEC_DEFAULT				ZIP_VERSION_SPEC(1, 0)
//#define ZIP_VERSION_SPEC_VOLUME_LABEL			ZIP_VERSION_SPEC(1, 1) // File is a volume label
//#define ZIP_VERSION_SPEC_FOLDER				ZIP_VERSION_SPEC(2, 0) // File is a folder (directory)
#define ZIP_VERSION_SPEC_DEFLATE_COMPRESSED		ZIP_VERSION_SPEC(2, 0) // File is compressed using Deflate compression
//#define ZIP_VERSION_SPEC_TRADITIONAL_ENCRYPTED	ZIP_VERSION_SPEC(2, 0) // File is encrypted using traditional PKWARE encryption
//#define ZIP_VERSION_SPEC_DEFLATE64_COMPRESSED	ZIP_VERSION_SPEC(2, 1) // File is compressed using Deflate64(tm)
//#define ZIP_VERSION_SPEC_IMPLODE_COMPRESSED	ZIP_VERSION_SPEC(2, 5) // File is compressed using PKWARE DCL Implode
//#define ZIP_VERSION_SPEC_PATCH_DATA_SET		ZIP_VERSION_SPEC(2, 7) // File is a patch data set
//#define ZIP_VERSION_SPEC_ZIP64_FORMAT			ZIP_VERSION_SPEC(4, 5) // File uses ZIP64 format extensions
//#define ZIP_VERSION_SPEC_BZIP2_COMPRESSED		ZIP_VERSION_SPEC(4, 6) // File is compressed using BZIP2 compression* (sometimes 5.0)
//#define ZIP_VERSION_SPEC_DES_ENCRYPTED		ZIP_VERSION_SPEC(5, 0) // File is encrypted using DES
//#define ZIP_VERSION_SPEC_3DES_ENCRYPTED		ZIP_VERSION_SPEC(5, 0) // File is encrypted using 3DES
//#define ZIP_VERSION_SPEC_RC2_ORIG_ENCRYPTED	ZIP_VERSION_SPEC(5, 0) // File is encrypted using original RC2 encryption
//#define ZIP_VERSION_SPEC_RC4_ENCRYPTED		ZIP_VERSION_SPEC(5, 0) // File is encrypted using RC4 encryption
//#define ZIP_VERSION_SPEC_AES_ENCRYPTED		ZIP_VERSION_SPEC(5, 1) // File is encrypted using AES encryption
//#define ZIP_VERSION_SPEC_RC2_ENCRYPTED		ZIP_VERSION_SPEC(5, 1) // File is encrypted using corrected RC2 encryption**
//#define ZIP_VERSION_SPEC_RC2_64_ENCRYPTED		ZIP_VERSION_SPEC(5, 2) // File is encrypted using corrected RC2-64 encryption**
//#define ZIP_VERSION_SPEC_NON_OAEP_ENCRYPTED	ZIP_VERSION_SPEC(6, 1) // File is encrypted using non-OAEP key wrapping***
//#define ZIP_VERSION_SPEC_CENTRAL_DIR_ENCRYPTED	ZIP_VERSION_SPEC(6, 2) // Central directory encryption
//#define ZIP_VERSION_SPEC_LZMA_COMPRESSED		ZIP_VERSION_SPEC(6, 3) // File is compressed using LZMA
//#define ZIP_VERSION_SPEC_PPMD_COMPRESSED		ZIP_VERSION_SPEC(6, 3) // File is compressed using PPMd+
//#define ZIP_VERSION_SPEC_BLOWFISH_ENCRYPTED	ZIP_VERSION_SPEC(6, 3) // File is encrypted using Blowfish
//#define ZIP_VERSION_SPEC_TWOFISH_ENCRYPTED	ZIP_VERSION_SPEC(6, 3) // File is encrypted using Twofish

typedef union _ZipVersion {
	uint16 Version;
	struct {
		byte OS;
		byte Spec;
	};
} ZipVersion;


//////////////////////////////////////////////////////////////////////////////
///// Zip Flags (commented ones are not supported)
//////////////////////////////////////////////////////////////////////////////
//#define ZIP_FLAG_ENCRYPTED			0x0001
#define ZIP_FLAG_METHOD_DEPENDENT		0x0006 // bits 1 and 2 are used for different compression methods
#define ZIP_FLAG_DATA_DESC_AFTER_COMP	0x0008 // Crc32/CompressedSize/UncompressedSize are 0 in header and instead are after the compressed data
											   // in <=2.04g only use this for DEFLATE, otherwise for any compression method
//#define ZIP_FLAG_DEFLATING_RESERVED	0x0010
//#define ZIP_FLAG_PATCHED_DATA			0x0020 // >=2.7
//#define ZIP_FLAG_STRONG_ENCRYPTION	0x0040 // >=5.0, must also set ZIP_FLAG_ENCRYPTED
//#define ZIP_FLAG_UNUSED_1				0x0080
//#define ZIP_FLAG_UNUSED_2				0x0100
//#define ZIP_FLAG_UNUSED_3				0x0200
//#define ZIP_FLAG_UNUSED_4				0x0400
#define ZIP_FLAG_UTF8_TEXT				0x0800 // >=6.3, filename and comment are UTF-8 encoded
//#define ZIP_FLAG_RESERVED_1			0x1000
//#define ZIP_FLAG_ENCRYPTED_MASK		0x2000
//#define ZIP_FLAG_RESERVED_2			0x4000
//#define ZIP_FLAG_RESERVED_3			0x8000

#define ZIP_FLAG_MASK					(ZIP_FLAG_METHOD_DEPENDENT | ZIP_FLAG_DATA_DESC_AFTER_COMP | ZIP_FLAG_UTF8_TEXT)


//#define ZIP_FLAG_IMPLODING_4K_DICT	0x0000
//#define ZIP_FLAG_IMPLODING_8K_DICT	0x0002
//#define ZIP_FLAG_IMPLODING_2_TREES	0x0000
//#define ZIP_FLAG_IMPLODING_3_TREES	0x0004

#define ZIP_FLAG_DEFLATING_NORMAL		0x0000
#define ZIP_FLAG_DEFLATING_MAXIMUM		0x0002
#define ZIP_FLAG_DEFLATING_FAST			0x0004
#define ZIP_FLAG_DEFLATING_SUPER_FAST	0x0006

//#define ZIP_FLAG_LZMA_EOS_BYTE_USED	0x0002


//////////////////////////////////////////////////////////////////////////////
///// Zip Compression Method (commented ones are not supported)
//////////////////////////////////////////////////////////////////////////////
#define ZIP_COMPRESSION_NONE			0x0000
//#define ZIP_COMPRESSION_SHRINK		0x0001
//#define ZIP_COMPRESSION_REDUCE_1		0x0002
//#define ZIP_COMPRESSION_REDUCE_2		0x0003
//#define ZIP_COMPRESSION_REDUCE_3		0x0004
//#define ZIP_COMPRESSION_REDUCE_4		0x0005
//#define ZIP_COMPRESSION_IMPLODE		0x0006
//#define ZIP_COMPRESSION_TOKENIZING	0x0007 // reserved, not implemented
#define ZIP_COMPRESSION_DEFLATE			0x0008
//#define ZIP_COMPRESSION_DEFLATE64		0x0009
//#define ZIP_COMPRESSION_PKWARE_IMPLODE	0x000A // old IBM TERSE
//#define ZIP_COMPRESSION_RESERVED_1	0x000B
//#define ZIP_COMPRESSION_BZIP2			0x000C
//#define ZIP_COMPRESSION_RESERVED_2	0x000D
//#define ZIP_COMPRESSION_LZMA			0x000E
//#define ZIP_COMPRESSION_RESERVED_3	0x000F
//#define ZIP_COMPRESSION_RESERVED_4	0x0010
//#define ZIP_COMPRESSION_RESERVED_5	0x0011
//#define ZIP_COMPRESSION_IBM_TERSE		0x0012
//#define ZIP_COMPRESSION_LZ77_PFS		0x0013
//#define ZIP_COMPRESSION_WAVPACK		0x0061
//#define ZIP_COMPRESSION_PPMD			0x0062

static const uint16 ZipCompressionMethods[] = { ZIP_COMPRESSION_NONE, ZIP_COMPRESSION_DEFLATE };


//////////////////////////////////////////////////////////////////////////////
///// Zip File Attributes (commented ones are not supported)
//////////////////////////////////////////////////////////////////////////////
#define ZIP_FILE_ATTRIBUTE_TEXT			0x0001
//#define ZIP_FILE_LENGTH_CONTROL_FIELD	0x0002 // ????


//////////////////////////////////////////////////////////////////////////////
///// Zip Extra Header IDs (commented ones are not supported)
//////////////////////////////////////////////////////////////////////////////
//#define ZIP_EXTRA_ID_ZIP64				0x0001
//#define ZIP_EXTRA_ID_AV_INFO				0x0007
//#define ZIP_EXTRA_ID_LANG_RESERVED		0x0008
//#define ZIP_EXTRA_ID_OS_2					0x0009
//#define ZIP_EXTRA_ID_NTFS					0x000a
//#define ZIP_EXTRA_ID_OPEN_VMS				0x000c
//#define ZIP_EXTRA_ID_UNIX					0x000d
//#define ZIP_EXTRA_ID_FORK_RESERVED		0x000e
//#define ZIP_EXTRA_ID_PATCH_DESCRIPTOR		0x000f
//#define ZIP_EXTRA_ID_PKCS7_X509_CERT		0x0014
//#define ZIP_EXTRA_ID_X509_CERT_FOR_FILE	0x0015
//#define ZIP_EXTRA_ID_X509_CERT_FOR_CD		0x0016
//#define ZIP_EXTRA_ID_STRONG_ENCRYPT		0x0017
//#define ZIP_EXTRA_ID_RECORD_MNGMNT		0x0018
//#define ZIP_EXTRA_ID_PKCS7_ENCRYPT_LIST	0x0019
//#define ZIP_EXTRA_ID_Z390_I400			0x0065
//#define ZIP_EXTRA_ID_Z390_I400_RESERVED	0x0066
//#define ZIP_EXTRA_ID_POSZIP_4690			0x4690

// Third party mappings
//#define ZIP_EXTRA_ID_MAC					0x07c8
//#define ZIP_EXTRA_ID_MAC_ZIPIT			0x2605
//#define ZIP_EXTRA_ID_MAC_ZIPIT_135_1		0x2705
//#define ZIP_EXTRA_ID_MAC_ZIPIT_135_2		0x2805
//#define ZIP_EXTRA_ID_MAC_INFO_ZIP			0x334d
//#define ZIP_EXTRA_ID_ACORN_SPARKFS		0x4341
//#define ZIP_EXTRA_ID_WINDOWS_NT_SACL		0x4453
//#define ZIP_EXTRA_ID_VM_CMS				0x4704
//#define ZIP_EXTRA_ID_MVS					0x470f
//#define ZIP_EXTRA_ID_FWKCS_MD5			0x4b46
//#define ZIP_EXTRA_ID_OS_2_ACL				0x4c41
//#define ZIP_EXTRA_ID_OPEN_VMS_INFO_ZIP	0x4d49
//#define ZIP_EXTRA_ID_XCEED_ORIG_LOC		0x4f4c
//#define ZIP_EXTRA_ID_AOS_VS_ACL			0x5356
//#define ZIP_EXTRA_ID_TIMESTAMP			0x5455
//#define ZIP_EXTRA_ID_XCEED_UNICODE		0x554e
//#define ZIP_EXTRA_ID_INFO_ZIP				0x5855
//#define ZIP_EXTRA_ID_COMMENT_INFO_ZIP		0x6375 // Unicode
//#define ZIP_EXTRA_ID_BEOS_BEBOX			0x6542
//#define ZIP_EXTRA_ID_PATH_INFO_ZIP		0x7075 // Unicode
//#define ZIP_EXTRA_ID_ASI_UNIX				0x756e
//#define ZIP_EXTRA_ID_UNIX_INFO_ZIP		0x7855
//#define ZIP_EXTRA_ID_MS_OPEN_PACKAGING	0xa220
//#define ZIP_EXTRA_ID_SMS_QDOS				0xfd4a
				  

//////////////////////////////////////////////////////////////////////////////
///// Zip Structures
//////////////////////////////////////////////////////////////////////////////
#define ZipFileSignature		0x04034b50
#define ZIP_FILE_FILENAME(f)	((byte*)((f)+1))
#define ZIP_FILE_EXTRA(f)		(ZIP_FILE_FILENAME(f)+(f)->FileNameLength)
#define ZIP_FILE_DATA(f)		(ZIP_FILE_EXTRA(f)+(f)->ExtraLength)
#define ZIP_FILE_SIZE(f)		(sizeof(ZipFile)+(f)->FileNameLength+(f)->ExtraLength)
typedef struct _ZipFile {
	uint32		Signature;
	ZipVersion	VersionNeededToExtract;
	uint16		Flags;
	uint16		CompressionMethod;
	uint16		LastModificationTime;
	uint16		LastModificationDate;
	uint32		Crc32;
	uint32		CompressedSize;
	uint32		UncompressedSize;
	uint16		FileNameLength;
	uint16		ExtraLength;
	//byte		Name[1];		// use '/' for separator
	//byte		Extra[1];		// cannot access this way
} ZipFile;


/*#define ZipDataSignature		0x08074b50
typedef struct _ZipData {
	uint32		Signature;
	uint32		Crc32;
	uint32		CompressedSize;
	uint32		UncompressedSize;
} ZipData;*/


/*typedef struct _ZipExtraData {
	uint16		HeaderID;
	uint16		DataSize;
} ZipExtraData;*/


/*#define ZipExtraSignature		0x08064b50
typedef struct _ZipExtra {
	uint32		Signature;
	uint32		Length;
	//byte		Data[1];
} ZipExtra;*/


#define ZipCentralDirectoryFileSignature	0x02014b50
#define ZIP_CDF_FILENAME(f)					((byte*)((f)+1))
#define ZIP_CDF_EXTRA(f)					(ZIP_CDF_FILENAME(f)+(f)->FileNameLength)
#define ZIP_CDF_COMMENT(f)					(ZIP_CDF_EXTRA(f)+(f)->ExtraLength)
#define ZIP_CDF_NEXT_ENTRY(f)				((ZipCentralDirectoryFile*)(ZIP_CDF_COMMENT(f)+(f)->CommentLength))
#define ZIP_CDF_SIZE(f)						(sizeof(ZipCentralDirectoryFile)+(f)->FileNameLength+(f)->ExtraLength+(f)->CommentLength)
typedef struct _ZipCentralDirectoryFile {
	uint32		Signature;
	ZipVersion	VersionMadeBy;
	ZipVersion	VersionNeededToExtract;
	uint16		Flags;
	uint16		CompressionMethod;
	uint16		LastModificationTime;
	uint16		LastModificationDate;
	uint32		Crc32;
	uint32		CompressedSize;
	uint32		UncompressedSize;
	uint16		FileNameLength;
	uint16		ExtraLength;
	uint16		CommentLength;
	uint16		DiskStart;
	uint16		InternalFileAttr;
	uint32		ExternalFileAttr;
	uint32		DiskOffset;
	//byte		FileName[1];	// use '/' for separator
	//byte		Extra[1];		// cannot access this way
	//byte		Comment[1];		// cannot access this way
} ZipCentralDirectoryFile;


#define ZipCentralDirectoryEndSignature		0x06054b50
#define ZIP_CDE_COMMENT(f)					((byte*)((f)+1))
#define ZIP_CDE_SIZE(f)						(sizeof(ZipCentralDirectoryEnd)+(f)->CommentLength)
typedef struct _ZipCentralDirectoryEnd {
	uint32		Signature;
	uint16		DiskNumber;
	uint16		CentralDirectoryDiskStart;
	uint16		CentralDirectoryEntriesOnDisk;
	uint16		CentralDirectoryEntries;
	uint32		CentralDirectorySize;
	uint32		CentralDirectoryOffset;
	uint16		CommentLength;
	//byte		Comment[1];
} ZipCentralDirectoryEnd;

#include <PopPack.h>


//////////////////////////////////////////////////////////////////////////////
///// Zip Functions
//////////////////////////////////////////////////////////////////////////////
#pragma unmanaged
static bool IsSupportedZipEntry(ZipVersion v, uint32 compressed, uint32 uncompressed, uint16 flags, uint16 method) {
	if (v.Spec > ZIP_VERSION_SPEC_MAX || compressed == INVALID_FILE_SIZE || uncompressed == INVALID_FILE_SIZE || (flags | ZIP_FLAG_MASK) != ZIP_FLAG_MASK) return false;
	for (int i = 0; i < ARRAYSIZE(ZipCompressionMethods); ++i)
		if (method == ZipCompressionMethods[i])
			return true;
	return false;
}
inline static bool IsSupportedZipEntry(ZipFile *f)					{ return f->Signature == ZipFileSignature                 && IsSupportedZipEntry(f->VersionNeededToExtract, f->CompressedSize, f->UncompressedSize, f->Flags, f->CompressionMethod); }
inline static bool IsSupportedZipEntry(ZipCentralDirectoryFile *f)	{ return f->Signature == ZipCentralDirectoryFileSignature && IsSupportedZipEntry(f->VersionNeededToExtract, f->CompressedSize, f->UncompressedSize, f->Flags, f->CompressionMethod); }

static void *read(HANDLE h, uint32 off, uint32 sz) {
	void *x = malloc(sz);
	if (!x || !ReadAt(h, x, sz, off))			{ free(x); return NULL; }
	return x;
}
/*static void *read_more(HANDLE h, void *x, uint32 orig_len, uint32 extra_len) {
	if (extra_len) {
		byte *b = (byte*)realloc(x, orig_len + extra_len);
		if (!b)									{ free(x); return NULL; }
		if (!Read(h, b+orig_len, extra_len))	{ free(b); return NULL; }
		x = b;
	}
	return x;
}*/

static ZipCentralDirectoryFile *GetCentralDirectory(HANDLE h, ZipCentralDirectoryEnd **_cde) {
	BYTE buf[256]; // read 256 bytes at a time
	uint32 file_size = GetFileSize(h, NULL);
	if (file_size == INVALID_FILE_SIZE || file_size<sizeof(ZipCentralDirectoryEnd))	{ return NULL; }
	uint32 search_at = file_size - sizeof(ZipCentralDirectoryEnd), read_size = sizeof(ZipCentralDirectoryEnd), cde_off = INVALID_FILE_SIZE;

	// Search near the end for the CDE block
	do {
		if (!ReadAt(h, buf, read_size, search_at))									{ return NULL; }
		for (uint32 i = read_size-sizeof(uint32)-1; i >= 0; --i) {
			if (*(uint32*)(buf+i) == ZipCentralDirectoryEndSignature) {
				cde_off = search_at + i;
				break;
			}
		}
		if (cde_off != INVALID_FILE_SIZE) break;
		if (search_at < sizeof(buf) - sizeof(uint32)) {
			read_size = search_at + sizeof(uint32);
			search_at = 0;
		} else {
			read_size = sizeof(buf);
			search_at -= sizeof(buf) - sizeof(uint32);
		}
	} while (read_size > sizeof(uint32));
	if (read_size <= sizeof(uint32))												{ return NULL; }

	// Read just the CDE block (and the comment if it exists)
	ZipCentralDirectoryEnd *cde = (ZipCentralDirectoryEnd*)read(h, cde_off, sizeof(ZipCentralDirectoryEnd));
	if (!cde)																		{ return NULL; }
//	cde = (ZipCentralDirectoryEnd*)read_more(h, cde, sizeof(ZipCentralDirectoryEnd), cde->CommentLength);
//	if (!cde)																		{ return NULL; }

	// Check that the file is not split and that the structure makes sense
	uint32 end = cde->CentralDirectoryOffset + cde->CentralDirectorySize;
	if (cde->DiskNumber != 0 || cde->CentralDirectoryDiskStart != 0 || cde->CentralDirectoryEntries != cde->CentralDirectoryEntriesOnDisk ||
		end < MAX(cde->CentralDirectoryOffset, cde->CentralDirectorySize) || end > cde_off) {
		free(cde);
		return NULL;
	}

	// Get the central directory
	ZipCentralDirectoryFile *cd = (ZipCentralDirectoryFile*)read(h, cde->CentralDirectoryOffset, cde->CentralDirectorySize);
	if (!cd || cd->Signature != ZipCentralDirectoryFileSignature)					{ free(cde); free(cd); return NULL; }

	*_cde = cde;
	return cd;
}

static HANDLE OpenZipFile(LPCWSTR file, ZipCentralDirectoryEnd **cde, ZipCentralDirectoryFile **cd) {
	HANDLE h = CreateFile(file, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)						{ return INVALID_HANDLE_VALUE; }
	if ((*cd = GetCentralDirectory(h, cde)) == NULL)	{ CloseHandle(h); return INVALID_HANDLE_VALUE; }
	return h;
}

static BYTE *GetFileCompressedData(HANDLE h, ZipCentralDirectoryFile *entry) {
	ZipFile *f = (ZipFile*)read(h, entry->DiskOffset, sizeof(ZipFile));
	if (!f)		{ return NULL; }
//	f = (ZipFile*)read_more(h, f, sizeof(ZipFile), f->FileNameLength + f->ExtraLength);
//	if (!f)		{ return NULL; }
	BYTE *b = IsSupportedZipEntry(f) ? (BYTE*)read(h, entry->DiskOffset + ZIP_FILE_SIZE(f), entry->CompressedSize) : NULL;
	free(f);
	return b;
}

#pragma managed
inline static string GetZipString(byte *b, int len, bool utf8) { return (utf8 ? Encoding::UTF8 : Encoding::ASCII)->GetString(Utilities::GetManagedArray(b, len)); }
//static string GetZipString(ZipFile *f) { return GetZipString(ZIP_FILE_NAME(f), f->FileNameLength, (f->Flags & ZIP_FLAG_UTF8_TEXT) ? true : false); }
inline static string GetZipString(ZipCentralDirectoryFile *f) { return GetZipString(ZIP_CDF_FILENAME(f), f->FileNameLength, (f->Flags & ZIP_FLAG_UTF8_TEXT) ? true : false); }

array<string> ^Zip::GetFileNames(string file) {
	ZipCentralDirectoryEnd *cde = NULL;
	ZipCentralDirectoryFile *cd = NULL;
	HANDLE h = OpenZipFile(as_native(file), &cde, &cd);
	if (h == INVALID_HANDLE_VALUE)	{ return nullptr; }
	CloseHandle(h);

	List<string> ^names = gcnew List<string>(cde->CentralDirectoryEntries);
	ZipCentralDirectoryFile *entry = cd;
	for (int i = 0; i < cde->CentralDirectoryEntries; ++i, entry = ZIP_CDF_NEXT_ENTRY(entry))
		if (IsSupportedZipEntry(entry))
			names->Add(GetZipString(entry));

	free(cde);
	free(cd);

	return names->ToArray();
}

Stream ^Zip::GetStream(string file, string name) {
	ZipCentralDirectoryEnd *cde = NULL;
	ZipCentralDirectoryFile *cd = NULL;
	HANDLE h = OpenZipFile(as_native(file), &cde, &cd);
	if (h == INVALID_HANDLE_VALUE)	{ return nullptr; }

	Stream ^s = nullptr;
	byte *b = NULL;
	
	ZipCentralDirectoryFile *entry = cd;
	for (int i = 0; i < cde->CentralDirectoryEntries; ++i, entry = ZIP_CDF_NEXT_ENTRY(entry)) {
		if (IsSupportedZipEntry(entry) && GetZipString(entry) == name) {
			b = GetFileCompressedData(h, entry);
			break;
		}
	}
	
	CloseHandle(h);

	if (b) {
		s = gcnew MemoryStream(Utilities::GetManagedArray(b, entry->CompressedSize));
		free(b);
		switch (entry->CompressionMethod) {
		case ZIP_COMPRESSION_NONE:		break; // s is not compressed, direct access
		case ZIP_COMPRESSION_DEFLATE:	s = gcnew DeflateStream(s, CompressionMode::Decompress); break;
		}
	}

	free(cde);
	free(cd);

	return s;
}
