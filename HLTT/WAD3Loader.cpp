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

	header.LumpCount = BIN::ReadUInt32(wadFile);
	header.LumpOffset = BIN::ReadUInt32(wadFile);

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
		lump.Offset = BIN::ReadUInt32(wadFile);
		lump.CompressedLength = BIN::ReadUInt32(wadFile);
		lump.FullLength = BIN::ReadUInt32(wadFile);
		lump.Type = BIN::ReadUInt8(wadFile); // ReadByte()
		lump.Compression = BIN::ReadUInt8(wadFile); // ReadByte()

		// Padding, 2-bytes
		wadFile->SeekI(2, wxFromCurrent);

		// GetNullTerminatedString()
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

wxBitmap *WAD3Loader::GetLumpImage(UINT32 index, bool transparent = false)
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
			width = BIN::ReadUInt32(wadFile);
			height = BIN::ReadUInt32(wadFile);

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
				UINT32 pixelOffset = BIN::ReadUInt32(wadFile);

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
			wxImage lumpImage(width, height); lumpImage.InitAlpha();
			UINT32 P = 0;
			for (UINT32 y = 0; y < height; y++)
			{
				for (UINT32 x = 0; x < width; x++)
				{
					lumpImage.SetRGB(x, y, red[pixels[P]], green[pixels[P]], blue[pixels[P]]);
					lumpImage.SetAlpha(x, y, (transparent && LumpsInfo[index].Name[0] == '{' && pixels[P] == MaxPaletteColors - 1) ? 0 : 255);
					P++;
				}
			}
#endif
#ifdef _WIN32
			wxBitmap *lumpImage = new wxBitmap(width, height, 32);
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
					IBMD.Alpha() = (transparent && LumpsInfo[index].Name[0] == '{' && pixels[P] == MaxPaletteColors - 1) ? 0 : 255;
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
			return new wxBitmap(lumpImage, 32);
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

int WAD3Loader::ExtractWADFromBSP(std::string inputFile, std::string outputFile)
{
	bool foundEOF = false;
	std::vector<IncludedBSPTexture> includedTextures;
	
	if (inputFile.rfind(".bsp") == std::string::npos)
	{
		wxMessageBox("Input file must be BSP!", "Error", wxOK | wxICON_ERROR);
		return 0;
	}
	
	wxFileInputStream *bspFile = new wxFileInputStream(inputFile);
	if (!bspFile->IsOk())
	{
		wxMessageBox("Failed to open BSP file!", "Error", wxOK | wxICON_ERROR);
		return 0;
	}
	
	long bufferLen = bspFile->GetLength();
	char *buffer = new char[bufferLen];
	UINT8 num1[4], num2[4], num3[4], num4[4];
	
	bspFile->Read(buffer, bufferLen);
	
	int found = 0;
	for (long i = 0; i < bufferLen; i++)
	{
		// where do these magic numbers come from?
		if ((i + 19 + 16) > bufferLen) break;
		
		// Search pattern
		UINT8 nullByte1 = buffer[i];
		UINT8 nullByte2 = buffer[i + 1];
		UINT8 firstNameByte = buffer[i + 2];
		UINT8 secondNameByte = buffer[i + 3];
		
		// +15 bytes of name
		UINT8 lastNameByte = buffer[i + 17];
		
		// can't scratch the feeling that this is a bad way of using memcpy
		memcpy(num1, buffer + i + 18, 4); // 4 = sizeof(int)
		memcpy(num2, buffer + i + 18 + 4, 4);
		memcpy(num3, buffer + i + 18 + 4 * 2, 4);
		memcpy(num4, buffer + i + 18 + 4 * 3, 4);
		
		int num1int = BIN::ToInt32(num1); // width
		int num2int = BIN::ToInt32(num2); // height
		int num3int = BIN::ToInt32(num3); // > 0 for custom wad
		int num4int = BIN::ToInt32(num4); // > 0 for custom wad
		
		if (nullByte2 < 2 &&
			firstNameByte > 32 && firstNameByte < 127 &&
			secondNameByte > 32 && secondNameByte < 127 &&
			lastNameByte == 0 &&
			num1int > 0 && num1int < 1024 &&
			num2int > 0 && num2int < 1024)
		{
			UINT32 textureOffset = (UINT32)i + 2;
			//long textureOffsetAbsolute = textureOffset + i - bufferLen;
			
			if (num3int > 0 && num3int < bufferLen && num4int > 0 && num4int < bufferLen)
			{
				// Set previous texture size
				if (includedTextures.size() > 0)
				{
					UINT32 offset = includedTextures[includedTextures.size() - 1].Offset;
					includedTextures[includedTextures.size() - 1].Size = (UINT32)(textureOffset - offset);
				}
				
				char *textureName = new char[MaxNameLength];
				memcpy(textureName, buffer + (int)textureOffset, MaxNameLength);
				
				// might catch garbage data, check texture integrity
				bool VALID = true;
				for (int c = 0; c < MaxNameLength; c++)
				{
					if (textureName[c] == 0) break;
					
					if (textureName[c] < 33 || textureName[c] > 126)
					{
						VALID = false;
						break;
					}
				}
				
				if ((num1int % 8) != 0 || (num2int % 8) != 0)
				{
					// Width or Height not divisible by 8
					VALID = false;
				}
				
				if (VALID)
				{
					std::string nameNulled(textureName);
					nameNulled += '\0';
					
					// don't snatch RAD lightdata
					if (nameNulled.find("__rad") == std::string::npos)
					{
						IncludedBSPTexture texture;
						texture.Offset = (UINT32)textureOffset;
						texture.Size = 0;
						texture.Name = nameNulled;
						
						includedTextures.push_back(texture);
					}
				}
				
				delete[] textureName;
			}
			else
			{
				if (includedTextures.size() > 0 && textureOffset > includedTextures[includedTextures.size() - 1].Offset)
				{
					UINT32 offset = includedTextures[includedTextures.size() - 1].Offset;
					includedTextures[includedTextures.size() - 1].Size = (UINT32)(textureOffset - offset);
					foundEOF = true;
				}
			}
		}
	}
	
	// Fix last texture size
	if (includedTextures.size() < 1)
	{
		wxMessageBox("BSP contains no embedded textures!", "Nothing found!", wxOK | wxICON_WARNING);
		return 0;
	}
	
	if (includedTextures[includedTextures.size() - 1].Size == 0 && !foundEOF)
	{
		UINT32 offset = includedTextures[includedTextures.size() - 1].Offset;
		includedTextures[includedTextures.size() - 1].Size = (UINT32)(bufferLen - offset);
	}
	
	// Extract textures to single WAD
	wxFileOutputStream *wadFile = new wxFileOutputStream(outputFile);
	if (!wadFile->IsOk())
	{
		wxMessageBox("Failed to write WAD file!", "Error", wxOK | wxICON_ERROR);
		return 0;
	}
	
	wadFile->Write(WadHeaderId.c_str(), WadHeaderId.length());
	BIN::WriteUInt32(wadFile, includedTextures.size());
	BIN::WriteUInt32(wadFile, 0); // offset lumps, later changed
	
	UINT32 *outputWadOffsets = new UINT32[includedTextures.size()];
	
	for (UINT32 i = 0; i < includedTextures.size(); i++)
	{
		bspFile->SeekI((long)includedTextures[i].Offset, wxFromStart);
		UINT8 *texture = new UINT8[includedTextures[i].Size];
		bspFile->Read(texture, includedTextures[i].Size);
		outputWadOffsets[i] = wadFile->TellO();
		wadFile->Write(texture, includedTextures[i].Size);
		delete[] texture;
	}
	
	// Write lump infos
	UINT32 posLumps = wadFile->TellO();
	wadFile->SeekO(8, wxFromStart);
	BIN::WriteUInt32(wadFile, posLumps);
	wadFile->SeekO(posLumps, wxFromStart);
	
	for (UINT32 i = 0; i < includedTextures.size(); i++)
	{
		BIN::WriteUInt32(wadFile, outputWadOffsets[i]); // offset
		BIN::WriteUInt32(wadFile, includedTextures[i].Size); // compressed length
		BIN::WriteUInt32(wadFile, includedTextures[i].Size); // full length
		BIN::WriteUInt8(wadFile, 0x43); // type
		BIN::WriteUInt8(wadFile, 0); // compression
		
		// Padding 2-bytes
		BIN::WriteUInt8(wadFile, 0);
		BIN::WriteUInt8(wadFile, 0);
		
		wadFile->Write(includedTextures[i].Name.c_str(), MaxNameLength); // name
	}
	
	delete[] buffer;
	delete[] outputWadOffsets;
	delete bspFile;
	delete wadFile;
	
	wxString szNumTextures = wxString::Format("Extracted %lu textures.", includedTextures.size());
	wxMessageBox(szNumTextures, "Extraction successful", wxOK | wxICON_INFORMATION);
	
	return (int)includedTextures.size();
}
