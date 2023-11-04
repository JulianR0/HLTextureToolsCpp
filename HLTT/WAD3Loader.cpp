#include "WAD3Loader.h"

bool WAD3Loader::LoadFile(std::string filePath)
{
	Filename = filePath;

	// Reset previous loaded data
	LumpsInfo.clear();
	Close();

	header = EmptyWadHeader;
	wadFile = new wxFileInputStream(filePath);

	// Get file header
	char temp[5];
	wadFile->Read(temp, 4);
	temp[4] = '\0';
	header.Id.assign(temp);

	if (header.Id != WadHeaderId)
	{
		Close();
		wxMessageBox("Invalid or unsupported WAD File!", "Error", wxOK | wxICON_ERROR);
		return false;
	}

	wadFile->Read(reinterpret_cast<char*>(&header.LumpCount), 4);
	wadFile->Read(reinterpret_cast<char*>(&header.LumpOffset), 4);

	return true;
}

void WAD3Loader::LoadLumps()
{
	// Seek to first lump
	wadFile->SeekI(header.LumpOffset, wxFromStart);

	// Iterate through all lumps, add them to array
	for (UINT32 i = 0; i < header.LumpCount; i++)
	{
		WADLump lump;
		wadFile->Read(reinterpret_cast<char*>(&lump.Offset), 4); // ReadUInt32
		wadFile->Read(reinterpret_cast<char*>(&lump.CompressedLength), 4);
		wadFile->Read(reinterpret_cast<char*>(&lump.FullLength), 4);
		wadFile->Read(reinterpret_cast<char*>(&lump.Type), 1); // ReadByte
		wadFile->Read(reinterpret_cast<char*>(&lump.Compression), 1);

		// Padding, 2-bytes
		wadFile->SeekI(2, wxFromCurrent);

		// GetNullTerminatedString()
		//char temp[MaxNameLength]; // expression must have a constant value
		char *temp = new char[MaxNameLength];
		wadFile->Read(temp, MaxNameLength);
		std::string chars(temp);
		std::string sb = "";
		for (UINT32 j = 0; j < chars.length(); j++)
		{
			if (chars[j] == 0x0) break;

			sb += chars[j];
		}
		sb += '\0'; // just in case append the null terminator
		lump.Name = sb;

		LumpsInfo.push_back(lump);
		delete[] temp;
	}
}

wxBitmap *WAD3Loader::GetLumpImage(UINT32 index, bool transparent)
{
	if (index < LumpsInfo.size())
	{
		UINT8 type = LumpsInfo[index].Type;

		// Supported types:
		// 0x40 - tempdecal.wad
		// 0x42 - cached.wad
		// 0x43 - normal wads
		// 0x46 - fonts
		if (type == 0x40 || type == 0x42 || type == 0x43 || type == 0x46)
		{
			// Go to lump
			wadFile->SeekI(LumpsInfo[index].Offset, wxFromStart);

			if (type == 0x40 || type == 0x43)
			{
				// Skip lump name
				wadFile->SeekI(MaxNameLength, wxFromCurrent);
			}

			// Read texture size
			UINT32 width, height;
			wadFile->Read(reinterpret_cast<char*>(&width), 4);
			wadFile->Read(reinterpret_cast<char*>(&height), 4);

			if (width > MaxTextureWidth || height > MaxTextureHeight)
			{
				wxMessageBox("Texture width or height exceeds maximum size!", "Error", wxOK | wxICON_ERROR);
				return new wxBitmap();
			}

			if (width == 0 || height == 0)
			{
				wxMessageBox("Texture width and height must be larger than 0!", "Error", wxOK | wxICON_ERROR);
				return new wxBitmap();
			}
#if 0
			// If QFont
			if (type == 0x46)
			{
				width = 256;
				UINT32 RowCount, RowHeight;
				wadFile->Read(reinterpret_cast<char*>(&RowCount), 4);
				wadFile->Read(reinterpret_cast<char*>(&RowHeight), 4);
			}
#endif
			// Read pixel offset, skip MIPMAPS offsets
			if (type == 0x40 || type == 0x43)
			{
				// Not used, but needed
				UINT32 pixelOffset;
				wadFile->Read(reinterpret_cast<char*>(&pixelOffset), 4);

				// Skip MIPMAPS offsets, not needed
				wadFile->SeekI(12, wxFromCurrent);
			}

			// Read RAW pixels
			UINT32 pixelSize = width * height;
			pixelsBlockPos = wadFile->TellI();
			lastImageSize = pixelSize;
			lastImageWidth = width;
			UINT8 *pixels = new UINT8[pixelSize];
			wadFile->Read(pixels, pixelSize);

			// Read MIPMAPS
			if (type == 0x40 || type == 0x43)
			{
				// unused
				UINT8 *Mipmap1 = new UINT8[(width / 2) * (height / 2)];
				UINT8 *Mipmap2 = new UINT8[(width / 4) * (height / 4)];
				UINT8 *Mipmap3 = new UINT8[(width / 8) * (height / 8)];
				wadFile->Read(Mipmap1, (width / 2) * (height / 2));
				wadFile->Read(Mipmap2, (width / 4) * (height / 4));
				wadFile->Read(Mipmap3, (width / 8) * (height / 8));
				delete[] Mipmap1;
				delete[] Mipmap2;
				delete[] Mipmap3;
			}

			// Padding 2-bytes
			wadFile->SeekI(2, wxFromCurrent);

			// Prepare new palette for bitmap
			paletteBlockPos = wadFile->TellI();

			// Read palette bytes from file into array
			UINT8 *palBytes = new UINT8[MaxPaletteColors * 3];
			wadFile->Read(palBytes, MaxPaletteColors * 3);

			UINT8 *red = new UINT8[MaxPaletteColors];
			UINT8 *green = new UINT8[MaxPaletteColors];
			UINT8 *blue = new UINT8[MaxPaletteColors];

			for (int i = 0, j = 0; i < MaxPaletteColors; i++)
			{
				if (type == 0x40) // tempdecal.wad
				{
					red[i] = green[i] = blue[i] = i;
				}
				else
				{
					// Read palette entry RGB
					red[i] = palBytes[j];
					green[i] = palBytes[j + 1];
					blue[i] = palBytes[j + 2];
				}
				j += 3;
			}
			
			// Build image from pixel data
#ifdef linux
			wxImage lumpImage(width, height);
			UINT32 P = 0;
			for (UINT32 y = 0; y < height; y++)
			{
				for (UINT32 x = 0; x < width; x++)
				{
					lumpImage.SetRGB(x, y, red[pixels[P]], green[pixels[P]], blue[pixels[P]]);
					P++;
				}
			}
#endif
#ifdef _WIN32
			wxBitmap *lumpImage = new wxBitmap(width, height, 8);
			PixelData bmdata(*lumpImage);
			PixelData::Iterator IBMD(bmdata); IBMD.Reset(bmdata);

			UINT32 P = 0;
			for (UINT32 y = 0; y < height; y++)
			{
				for (UINT32 x = 0; x < width; x++)
				{
					IBMD.MoveTo(bmdata, x, y);
					IBMD.Red() = red[pixels[P]];
					IBMD.Green() = green[pixels[P]];
					IBMD.Blue() = blue[pixels[P]];
					P++;
				}
			}
#endif

			delete[] pixels;
			delete[] palBytes;
			delete[] red;
			delete[] green;
			delete[] blue;

#ifdef linux
			return new wxBitmap(lumpImage, 8);
#endif
#ifdef _WIN32
			return lumpImage;
#endif
		}
	}

	return new wxBitmap();
}

void WAD3Loader::Close()
{
	if (wadFile != nullptr)
	{
		if (wadFile->GetFile()->IsOpened())
		{
			wadFile->GetFile()->Close();
		}

		delete wadFile;
		wadFile = nullptr;
	}
}