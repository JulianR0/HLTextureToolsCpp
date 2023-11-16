// Thanks Moneyl for the BinaryTools repository
// [https://github.com/Moneyl/BinaryTools]
#pragma once

#include <wx/wfstream.h>

#ifdef _WIN32
#include <basetsd.h>
#endif
#ifdef linux
#include "linux.h"
#endif

namespace BIN
{
	// BinaryReader
	UINT8 ReadUInt8(wxFileInputStream *stream);
	UINT16 ReadUInt16(wxFileInputStream *stream);
	UINT32 ReadUInt32(wxFileInputStream *stream);
	
	INT8 ReadInt8(wxFileInputStream *stream);
	INT16 ReadInt16(wxFileInputStream *stream);
	INT32 ReadInt32(wxFileInputStream *stream);
	
	float ReadFloat(wxFileInputStream *stream);
	
	// BinaryWriter
	void WriteUInt8(wxFileOutputStream *stream, UINT8 value);
	void WriteUInt16(wxFileOutputStream *stream, UINT16 value);
	void WriteUInt32(wxFileOutputStream *stream, UINT32 value);
	
	void WriteInt8(wxFileOutputStream *stream, INT8 value);
	void WriteInt16(wxFileOutputStream *stream, INT16 value);
	void WriteInt32(wxFileOutputStream *stream, INT32 value);

	// BitConverter
	INT32 ToInt32(UINT8 *data);

	inline bool IsLittleEndian()
	{
		volatile unsigned int i = 0x01234567;
		return (*((unsigned char*)(&i))) == 0x67;
	}
}
