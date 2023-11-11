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

enum SprType
{
	VP_PARALLEL_UPRIGHT,
	VP_FACING_UPRIGHT,
	VP_PARALLEL,
	ORIENTED,
	VP_PARALLEL_ORIENTED
};

enum SprTextFormat
{
	SPR_NORMAL,
	SPR_ADDITIVE,
	SPR_INDEXALPHA,
	SPR_ALPHATEST
};

enum SprSynchType
{
	Synchronized,
	Random
};

struct SprHeader
{
	std::string Id;
	int Version;
	SprType Type;
	SprTextFormat TextFormat;
	float BoundingRadius;
	int MaxWidth;
	int MaxHeight;
	int NumFrames;
	float BeamLen;
	SprSynchType SynchType;
};
static const struct SprHeader EmptySprHeader = {};

struct Frame
{
	int OriginX;
	int OriginY;
	wxBitmap *Image;
};

class SpriteLoader
{
private:
	const std::string SpriteHeaderId = "IDSP";
	const int MaxPaletteColors = 256;
	wxFileInputStream *sprFile;
	long *indexesOfPixelPositions;
	UINT32 *pixelsLengths;

public:
	SprHeader SpriteHeader;
	std::string Filename;

	std::vector<Frame> LoadFile(std::string filePath, bool transparent);
	void Close();
};
