#pragma once
#include <windows.h>

// 如果未定义 IO_REPARSE_TAG_SYMLINK，先定义它
#ifndef IO_REPARSE_TAG_SYMLINK
	#define IO_REPARSE_TAG_SYMLINK (0xA000000CL)
#endif

// 手动定义 REPARSE_DATA_BUFFER 结构体
// 注意：为了简化，这里只包含处理符号链接所需的部分
typedef struct REPARSE_DATA_BUFFER
{
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;

	union
	{
		struct
		{
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;

		// 如果你需要处理目录交接点 (Junctions)，也可以添加 MountPointReparseBuffer
	} DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

// 定义符号链接标志
#ifndef SYMLINK_FLAG_RELATIVE
	#define SYMLINK_FLAG_RELATIVE 0x00000001
#endif
