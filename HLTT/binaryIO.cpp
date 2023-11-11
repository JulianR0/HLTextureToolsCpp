// Thanks Moneyl for the BinaryTools repository
// [https://github.com/Moneyl/BinaryTools]
#include "binaryIO.h"

namespace BIN
{
	// BinaryReader
	uint8_t ReadUInt8(wxFileInputStream *stream) { uint8_t output; stream->Read(reinterpret_cast<char*>(&output), 1); return output; }
	uint16_t ReadUInt16(wxFileInputStream *stream) { uint16_t output; stream->Read(reinterpret_cast<char*>(&output), 2); return output; }
	uint32_t ReadUInt32(wxFileInputStream *stream) { uint32_t output; stream->Read(reinterpret_cast<char*>(&output), 4); return output; }
	
	int8_t ReadInt8(wxFileInputStream *stream) { int8_t output; stream->Read(reinterpret_cast<char*>(&output), 1); return output; }
	int16_t ReadInt16(wxFileInputStream *stream) { int16_t output; stream->Read(reinterpret_cast<char*>(&output), 2); return output; }
	int32_t ReadInt32(wxFileInputStream *stream) { int32_t output; stream->Read(reinterpret_cast<char*>(&output), 4); return output; }
	
	float ReadFloat(wxFileInputStream *stream) { float output; stream->Read(reinterpret_cast<char*>(&output), 4); return output; }
	
	// BinaryWriter
	void WriteUInt8(wxFileOutputStream *stream, uint8_t value) { stream->Write(reinterpret_cast<const char*>(&value), 1); }
	void WriteUInt16(wxFileOutputStream *stream, uint16_t value) { stream->Write(reinterpret_cast<const char*>(&value), 2); }
	void WriteUInt32(wxFileOutputStream *stream, uint32_t value) { stream->Write(reinterpret_cast<const char*>(&value), 4); }
	
	void WriteInt8(wxFileOutputStream *stream, int8_t value) { stream->Write(reinterpret_cast<const char*>(&value), 1); }
	void WriteInt16(wxFileOutputStream *stream, int16_t value) { stream->Write(reinterpret_cast<const char*>(&value), 2); }
	void WriteInt32(wxFileOutputStream *stream, int32_t value) { stream->Write(reinterpret_cast<const char*>(&value), 4); }
}
