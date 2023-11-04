#include "SpriteLoader.h"

std::vector<Frame> SpriteLoader::LoadFile(std::string filePath, bool transparent)
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

	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.Version), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.Type), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.TextFormat), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.BoundingRadius), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.MaxWidth), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.MaxHeight), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.NumFrames), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.BeamLen), 4);
	sprFile->Read(reinterpret_cast<char*>(&SpriteHeader.SynchType), 4);

	// Palette length
	UINT16 u;
	sprFile->Read(reinterpret_cast<char*>(&u), 2);

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
		sprFile->Read(reinterpret_cast<char*>(&frameGroup), 4);
		sprFile->Read(reinterpret_cast<char*>(&frameOriginX), 4);
		sprFile->Read(reinterpret_cast<char*>(&frameOriginY), 4);
		sprFile->Read(reinterpret_cast<char*>(&frameWidth), 4);
		sprFile->Read(reinterpret_cast<char*>(&frameHeight), 4);

		// Get pixelsize
		UINT32 pixelSize = (UINT32)(frameWidth * frameHeight);

		indexesOfPixelPositions[i] = sprFile->TellI();
		pixelsLengths[i] = pixelSize;

		// Load all pixels from file to array
		UINT8 *pixels = new UINT8[pixelSize];
		sprFile->Read(pixels, pixelSize);

		// Build image from image data
#ifdef linux
		wxImage frameImage(frameWidth, frameHeight);
		UINT32 P = 0;
		for (UINT32 y = 0; y < (UINT32)frameHeight; y++)
		{
			for (UINT32 x = 0; x < (UINT32)frameWidth; x++)
			{
				frameImage.SetRGB(x, y, red[pixels[P]], green[pixels[P]], blue[pixels[P]]);
				P++;
			}
		}
#endif
#ifdef _WIN32
		wxBitmap *frameImage = new wxBitmap(frameWidth, frameHeight, 8);
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
				P++;
			}
		}
#endif
		// Insert new frame to frames list
		Frame frame;
		frame.OriginX = frameOriginX;
		frame.OriginY = frameOriginY;
#ifdef linux
		frame.Image = new wxBitmap(frameImage, 8);
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