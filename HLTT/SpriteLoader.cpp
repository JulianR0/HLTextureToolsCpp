#include "SpriteLoader.h"

std::vector<Frame> SpriteLoader::LoadFile(std::string filePath, bool transparent = false)
{
	Filename = filePath;

	// Reset previous loaded data
	std::vector<Frame> frames;
	Close();

	SpriteHeader = EmptySprHeader;
	sprFile = new wxFileInputStream(filePath);

	// Get file header
	char temp[5];
	sprFile->Read(temp, 4);
	temp[4] = '\0';
	SpriteHeader.Id.assign(temp);

	if (SpriteHeader.Id != SpriteHeaderId)
	{
		Close();
		wxMessageBox("Invalid or unsupported SPR File!", "Error", wxOK | wxICON_ERROR);
		return frames;
	}

	SpriteHeader.Version = BIN::ReadInt32(sprFile);
	SpriteHeader.Type = (SprType)BIN::ReadInt32(sprFile);
	SpriteHeader.TextFormat = (SprTextFormat)BIN::ReadInt32(sprFile);
	SpriteHeader.BoundingRadius = BIN::ReadFloat(sprFile);
	SpriteHeader.MaxWidth = BIN::ReadInt32(sprFile);
	SpriteHeader.MaxHeight = BIN::ReadInt32(sprFile);
	SpriteHeader.NumFrames = BIN::ReadInt32(sprFile);
	SpriteHeader.BeamLen = BIN::ReadFloat(sprFile);
	SpriteHeader.SynchType = (SprSynchType)BIN::ReadInt32(sprFile);
	
	// Palette length
	UINT16 u = BIN::ReadUInt16(sprFile);

	// Create new palette for bitmap
	UINT8 *palBytes = new UINT8[u * 3];
	sprFile->Read(palBytes, u * 3);

	UINT8 *red = new UINT8[u];
	UINT8 *green = new UINT8[u];
	UINT8 *blue = new UINT8[u];

	for (int i = 0, j = 0; i < u; i++)
	{
		// Load RGB from file
		red[i] = palBytes[j];
		green[i] = palBytes[j + 1];
		blue[i] = palBytes[j + 2];
		
		j += 3;
	}

	indexesOfPixelPositions = new long[SpriteHeader.NumFrames];
	pixelsLengths = new UINT32[SpriteHeader.NumFrames];

	// Load frames
	for (int i = 0; i < SpriteHeader.NumFrames; i++)
	{
		int frameGroup, frameOriginX, frameOriginY, frameWidth, frameHeight;
		frameGroup = BIN::ReadInt32(sprFile);
		frameOriginX = BIN::ReadInt32(sprFile);
		frameOriginY = BIN::ReadInt32(sprFile);
		frameWidth = BIN::ReadInt32(sprFile);
		frameHeight = BIN::ReadInt32(sprFile);
		
		// Get pixelsize
		UINT32 pixelSize = (UINT32)(frameWidth * frameHeight);

		indexesOfPixelPositions[i] = sprFile->TellI();
		pixelsLengths[i] = pixelSize;

		// Load all pixels from file to array
		UINT8 *pixels = new UINT8[pixelSize];
		sprFile->Read(pixels, pixelSize);

		// Build image from image data
#ifdef linux
		wxImage frameImage(frameWidth, frameHeight); frameImage.InitAlpha();
		UINT32 P = 0;
		for (UINT32 y = 0; y < (UINT32)frameHeight; y++)
		{
			for (UINT32 x = 0; x < (UINT32)frameWidth; x++)
			{
				frameImage.SetRGB(x, y, red[pixels[P]], green[pixels[P]], blue[pixels[P]]);
				frameImage.SetAlpha(x, y, (transparent && pixels[P] == u - 1 && SpriteHeader.TextFormat == SPR_ALPHATEST) ? 0 : 255);
				P++;
			}
		}
#endif
#ifdef _WIN32
		wxBitmap *frameImage = new wxBitmap(frameWidth, frameHeight, 32);
		PixelData bmdata(*frameImage);
		PixelData::Iterator IBMD(bmdata); IBMD.Reset(bmdata);

		UINT32 P = 0;
		for (UINT32 y = 0; y < (UINT32)frameHeight; y++)
		{
			for (UINT32 x = 0; x < (UINT32)frameWidth; x++)
			{
				IBMD.MoveTo(bmdata, x, y);
				IBMD.Red() = red[pixels[P]];
				IBMD.Green() = green[pixels[P]];
				IBMD.Blue() = blue[pixels[P]];
				IBMD.Alpha() = (transparent && pixels[P] == u - 1 && SpriteHeader.TextFormat == SPR_ALPHATEST) ? 0 : 255;
				P++;
			}
		}
#endif
		// Insert new frame to frames list
		Frame frame;
		frame.OriginX = frameOriginX;
		frame.OriginY = frameOriginY;
#ifdef linux
		frame.Image = new wxBitmap(frameImage, 32);
#endif
#ifdef _WIN32
		frame.Image = frameImage;
#endif
		frames.push_back(frame);

		delete[] pixels;
	}

	delete[] palBytes;
	delete[] red;
	delete[] green;
	delete[] blue;

	return frames;
}

void SpriteLoader::Close()
{
	if (sprFile != nullptr)
	{
		if (sprFile->GetFile()->IsOpened())
		{
			sprFile->GetFile()->Close();
		}

		delete sprFile;
		sprFile = nullptr;
	}
}
