// Thanks Moneyl for the BinaryTools repository
// [https://github.com/Moneyl/BinaryTools]
#include "binaryIO.h"

namespace BIN
{
	// BinaryReader
	UINT8 ReadUInt8(wxFileInputStream *stream) { UINT8 output; stream->Read(reinterpret_cast<char*>(&output), 1); return output; }
	UINT16 ReadUInt16(wxFileInputStream *stream) { UINT16 output; stream->Read(reinterpret_cast<char*>(&output), 2); return output; }
	UINT32 ReadUInt32(wxFileInputStream *stream) { UINT32 output; stream->Read(reinterpret_cast<char*>(&output), 4); return output; }
	
	INT8 ReadInt8(wxFileInputStream *stream) { INT8 output; stream->Read(reinterpret_cast<char*>(&output), 1); return output; }
	INT16 ReadInt16(wxFileInputStream *stream) { INT16 output; stream->Read(reinterpret_cast<char*>(&output), 2); return output; }
	INT32 ReadInt32(wxFileInputStream *stream) { INT32 output; stream->Read(reinterpret_cast<char*>(&output), 4); return output; }
	
	float ReadFloat(wxFileInputStream *stream) { float output; stream->Read(reinterpret_cast<char*>(&output), 4); return output; }
	
	// BinaryWriter
	void WriteUInt8(wxFileOutputStream *stream, UINT8 value) { stream->Write(reinterpret_cast<const char*>(&value), 1); }
	void WriteUInt16(wxFileOutputStream *stream, UINT16 value) { stream->Write(reinterpret_cast<const char*>(&value), 2); }
	void WriteUInt32(wxFileOutputStream *stream, UINT32 value) { stream->Write(reinterpret_cast<const char*>(&value), 4); }
	
	void WriteInt8(wxFileOutputStream *stream, INT8 value) { stream->Write(reinterpret_cast<const char*>(&value), 1); }
	void WriteInt16(wxFileOutputStream *stream, INT16 value) { stream->Write(reinterpret_cast<const char*>(&value), 2); }
	void WriteInt32(wxFileOutputStream *stream, INT32 value) { stream->Write(reinterpret_cast<const char*>(&value), 4); }

	// BitConverter
	INT32 ToInt32(UINT8 *data)
	{
		INT32 retValue = 0;
		
		if (IsLittleEndian())
			retValue = ((int)data[3] << 24) | ((int)data[2] << 16) | ((int)data[1] << 8) | ((int)data[0]);
		else
			retValue = ((int)data[0] << 24) | ((int)data[1] << 16) | ((int)data[2] << 8) | ((int)data[3]);

		return retValue;
	}
}
