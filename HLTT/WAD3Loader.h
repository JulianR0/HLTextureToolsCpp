#pragma once

#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/palette.h>
#include <wx/rawbmp.h>

#include "binaryIO.h"

#ifdef linux
#include "linux.h"
#endif

typedef wxAlphaPixelData PixelData;

struct WADHeader
{
	std::string Id;
	UINT32 LumpCount;
	UINT32 LumpOffset;
};
static const struct WADHeader EmptyWadHeader = {};

struct WADLump
{
	UINT32 Offset;
	UINT32 CompressedLength;
	UINT32 FullLength;
	UINT8 Type;
	UINT8 Compression;
	std::string Name;
};

struct IncludedBSPTexture
{
	UINT32 Offset;
	UINT32 Size;
	std::string Name;
};

class WAD3Loader
{
private:
	const int MaxPaletteColors = 256;
	const int MaxNameLength = 16;
	const int LumpSize = 32;
	const UINT32 MaxTextureWidth = 4096;
	const UINT32 MaxTextureHeight = 4096;
	const int QCharWidth = 16;
	const int QNumbOfGlyphs = 256;
	const std::string WadHeaderId = "WAD3";

	long paletteBlockPos = 0;
	long pixelsBlockPos = 0;
	long lastImageSize = 0;
	UINT32 lastImageWidth = 0;

	WADHeader header;
	wxFileInputStream *wadFile;

public:
	std::vector<WADLump> LumpsInfo;
	std::string Filename;

	bool LoadFile(std::string filePath);
	void Close();

	void LoadLumps();
	wxBitmap *GetLumpImage(UINT32 index, bool transparent);
	int ExtractWADFromBSP(std::string inputFile, std::string outputFile);
};
