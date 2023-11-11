// Thanks Moneyl for the BinaryTools repository
// [https://github.com/Moneyl/BinaryTools]
#pragma once

#include <wx/wfstream.h>

namespace BIN
{
	// BinaryReader
	uint8_t ReadUInt8(wxFileInputStream *stream);
	uint16_t ReadUInt16(wxFileInputStream *stream);
	uint32_t ReadUInt32(wxFileInputStream *stream);
	
	int8_t ReadInt8(wxFileInputStream *stream);
	int16_t ReadInt16(wxFileInputStream *stream);
	int32_t ReadInt32(wxFileInputStream *stream);
	
	float ReadFloat(wxFileInputStream *stream);
	
	// BinaryWriter
	void WriteUInt8(wxFileOutputStream *stream, uint8_t value);
	void WriteUInt16(wxFileOutputStream *stream, uint16_t value);
	void WriteUInt32(wxFileOutputStream *stream, uint32_t value);
	
	void WriteInt8(wxFileOutputStream *stream, int8_t value);
	void WriteInt16(wxFileOutputStream *stream, int16_t value);
	void WriteInt32(wxFileOutputStream *stream, int32_t value);
}
