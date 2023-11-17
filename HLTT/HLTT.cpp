#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/treelist.h>
#include <wx/dc.h>
#include <wx/graphics.h>

#ifdef linux
#include "linux.h"
#endif

#include "WAD3Loader.h"
WAD3Loader g_WAD;

#include "SpriteLoader.h"
SpriteLoader g_SPR;

#include "ExtractWAD.h"

const int ANIMATE_EVENT_ID = wxID_HIGHEST + 1;

class System : public wxApp
{
public:
	bool OnInit() override;
};
wxIMPLEMENT_APP(System);

class ImageBox : public wxPanel
{
public:
	ImageBox(wxFrame *parent);
	wxBitmap *lumpImage;
	std::vector<Frame> spriteImage;
	
	wxTimer *animateTimer;
	bool shouldAnimate;
	UINT32 lastSpriteFrame;

	void OnPaint(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void Tick(wxTimerEvent& event);

	DECLARE_EVENT_TABLE();
};
ImageBox::ImageBox(wxFrame* parent) : wxPanel(parent)
{
	lumpImage = nullptr;
	shouldAnimate = false;
	lastSpriteFrame = 0;
	animateTimer = new wxTimer(this, ANIMATE_EVENT_ID);
}

BEGIN_EVENT_TABLE(ImageBox, wxPanel)
EVT_PAINT(ImageBox::OnPaint)
EVT_TIMER(ANIMATE_EVENT_ID, ImageBox::Tick)
EVT_SIZE(ImageBox::OnResize)
END_EVENT_TABLE()

enum // WindowID
{
	ID_List = 10,
	ID_Image
};

enum // MenuID
{
	MENU_Extract = 20,
	MENU_ExtractAllBMP,
	MENU_ExtractAllPNG,
	MENU_ExtractAllJPEG,
	MENU_Animate,
	MENU_Transparent,
	MENU_ExtractBSP
};

enum HLFileType
{
	FILE_Wad = 0,
	FILE_Sprite
};
HLFileType fileType;

class MainForm : public wxFrame
{
public:
	wxListBox *listBox;
	ImageBox *imageBox;

	wxMenu *menuFile;
	wxMenu *submenuExtractAll;
	wxMenu *menuImage;
	wxMenu *menuSettings;
	wxMenu *menuTools;

	bool drawTransparent;
	MainForm();

private:
	void OnOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void ListBoxChanged(wxCommandEvent& event);
	
	// File
	void ExtractImage(wxCommandEvent& event);
	void ExtractAllImages(wxCommandEvent& event);

	// Image
	void AnimateSprite(wxCommandEvent& event);
	
	// Settings
	void ToggleTransparency(wxCommandEvent& event);
	
	// Tools
	void ExtractFromBSP(wxCommandEvent& event);
};

bool System::OnInit()
{
	//wxImage::AddHandler(new wxBMPHandler); // always installed
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxJPEGHandler);
	
	MainForm *main = new MainForm();
	main->Show();
	return true;
}

MainForm::MainForm() : wxFrame(nullptr, wxID_ANY, "Half-Life Texture Tools", wxDefaultPosition, wxSize(960, 544))
{
	menuFile = new wxMenu;
	menuFile->Append(wxID_OPEN);
	menuFile->AppendSeparator();
	menuFile->Append(MENU_Extract, "Extract\tCtrl+S", "Extract selected texture/sprite frame");
	
	submenuExtractAll = new wxMenu;
	submenuExtractAll->Append(MENU_ExtractAllBMP, "As BMP", "Extracts all textures/sprite frames as BMP images");
	submenuExtractAll->Append(MENU_ExtractAllPNG, "As PNG", "Extracts all textures/sprite frames as PNG images");
	submenuExtractAll->Append(MENU_ExtractAllJPEG, "As JPEG", "Extracts all textures/sprite frames as JPEG images");
	menuFile->AppendSubMenu(submenuExtractAll, "Extract all", wxEmptyString);
	
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	menuFile->Enable(MENU_Extract, false);
	menuFile->Enable(MENU_ExtractAllBMP, false);
	menuFile->Enable(MENU_ExtractAllPNG, false);
	menuFile->Enable(MENU_ExtractAllJPEG, false);

	menuImage = new wxMenu;
	menuImage->Append(MENU_Animate, "Animation Play/Stop\tF5", "Animate the sprite");
	menuImage->Enable(MENU_Animate, false);

	menuSettings = new wxMenu;
	menuSettings->Append(MENU_Transparent, "Transparent textures", "Enable transparency for textures and sprites", wxITEM_CHECK);
	menuSettings->Check(MENU_Transparent, true);
	drawTransparent = true;
	
	menuTools = new wxMenu;
	menuTools->Append(MENU_ExtractBSP, "Extract WAD from BSP", "Extract embedded textures from a BSP map");
	
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "File");
	menuBar->Append(menuImage, "Image");
	menuBar->Append(menuSettings, "Settings");
	menuBar->Append(menuTools, "Tools");
	
	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &MainForm::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainForm::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainForm::ExtractImage, this, MENU_Extract);
	Bind(wxEVT_MENU, &MainForm::ExtractAllImages, this, MENU_ExtractAllBMP);
	Bind(wxEVT_MENU, &MainForm::ExtractAllImages, this, MENU_ExtractAllPNG);
	Bind(wxEVT_MENU, &MainForm::ExtractAllImages, this, MENU_ExtractAllJPEG);
	Bind(wxEVT_MENU, &MainForm::AnimateSprite, this, MENU_Animate);
	Bind(wxEVT_MENU, &MainForm::ToggleTransparency, this, MENU_Transparent);
	Bind(wxEVT_MENU, &MainForm::ExtractFromBSP, this, MENU_ExtractBSP);

	//wxPanel *mainPanel = new wxPanel(this);
	wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);

	imageBox = new ImageBox((wxFrame*)this);
	
	listBox = new wxListBox(this, ID_List, wxDefaultPosition, wxDefaultSize);
	Bind(wxEVT_LISTBOX, &MainForm::ListBoxChanged, this, ID_List);

	mainSizer->Add(imageBox, 1, wxEXPAND | wxALL, 10);
	mainSizer->Add(listBox, 1, wxEXPAND | wxALL, 10);

	SetSizer(mainSizer);

	CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);
	SetStatusText("Ready");
	Center(wxBOTH);
}

void MainForm::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MainForm::OnOpen(wxCommandEvent& event)
{
	std::string fileFilters;
	fileFilters = "Half-Life WAD/SPR files|*.wad;*.spr";
	fileFilters += "|HL WAD Files|*.wad";
	fileFilters += "|HL SPR Files|*.spr";

	wxFileDialog openDialog(this, "Open file", wxEmptyString, wxEmptyString, fileFilters, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openDialog.ShowModal() == wxID_CANCEL)
		return;
	
	wxFileInputStream inputFile(openDialog.GetPath());
	if (!inputFile.IsOk())
	{
		wxMessageBox("Unable to open file.", "Error", wxOK | wxICON_ERROR);
		return;
	}
	
	bool isWAD = openDialog.GetPath().EndsWith(".wad");
	bool isSPR = openDialog.GetPath().EndsWith(".spr");
	
	if (!isWAD && !isSPR)
	{
		wxMessageBox("Invalid input file.", "Error", wxOK | wxICON_ERROR);
		return;
	}

	listBox->Clear();
	menuFile->Enable(MENU_Extract, true);
	menuFile->Enable(MENU_ExtractAllBMP, true);
	menuFile->Enable(MENU_ExtractAllPNG, true);
	menuFile->Enable(MENU_ExtractAllJPEG, true);

	if (isWAD && g_WAD.LoadFile(openDialog.GetPath().utf8_string()))
	{
		fileType = FILE_Wad;
		menuImage->Enable(MENU_Animate, false);
		g_WAD.LoadLumps();

		for (UINT32 i = 0; i < g_WAD.LumpsInfo.size(); i++)
		{
			listBox->Insert(g_WAD.LumpsInfo[i].Name, i);
		}
	}
	else if (isSPR)
	{
		fileType = FILE_Sprite;
		menuImage->Enable(MENU_Animate, true);
		imageBox->spriteImage = g_SPR.LoadFile(openDialog.GetPath().utf8_string(), drawTransparent);

		for (UINT32 i = 0; i < imageBox->spriteImage.size(); i++)
		{
			char temp[12];
			sprintf(temp, "Frame #%i", i);
			std::string szFrame = temp;
			listBox->Insert(szFrame, i);
		}
	}
}

void MainForm::ListBoxChanged(wxCommandEvent& event)
{
	if (fileType == FILE_Wad && g_WAD.Filename.length() == 0 || fileType == FILE_Sprite && g_SPR.Filename.length() == 0)
		return;
	
	if (fileType == FILE_Wad)
		imageBox->lumpImage = g_WAD.GetLumpImage((UINT32)event.GetSelection(), drawTransparent);
	else if (fileType == FILE_Sprite)
	{
		if (!imageBox->shouldAnimate)
		{
			imageBox->lumpImage = imageBox->spriteImage[(UINT32)event.GetSelection()].Image;
			imageBox->lastSpriteFrame = (UINT32)event.GetSelection();
		}
	}

	if (fileType == FILE_Wad || fileType == FILE_Sprite && !imageBox->shouldAnimate)
		imageBox->Refresh();
}

void MainForm::AnimateSprite(wxCommandEvent& event)
{
	if (fileType == FILE_Sprite)
	{
		imageBox->shouldAnimate = !imageBox->shouldAnimate;
		if (imageBox->shouldAnimate)
			imageBox->animateTimer->Start(100);
		else
			imageBox->animateTimer->Stop();
	}
}

void MainForm::ToggleTransparency(wxCommandEvent& event)
{
	drawTransparent = menuSettings->IsChecked(MENU_Transparent);

	if (fileType == FILE_Wad && g_WAD.Filename.length() == 0 || fileType == FILE_Sprite && g_SPR.Filename.length() == 0)
		return;

	if (fileType == FILE_Wad)
		imageBox->lumpImage = g_WAD.GetLumpImage(listBox->GetSelection(), drawTransparent);
	else if (fileType == FILE_Sprite)
	{
		imageBox->spriteImage = g_SPR.LoadFile(g_SPR.Filename, drawTransparent);
		imageBox->lumpImage = imageBox->spriteImage[listBox->GetSelection()].Image;
	}

	imageBox->Refresh();
}

void MainForm::ExtractFromBSP(wxCommandEvent& event)
{
	ExtractWADForm *form = new ExtractWADForm((wxFrame*)this, wxID_ANY, "Extract embedded WAD textures from BSP", wxDefaultPosition, wxSize(568, 300));
	form->ShowModal();
}

void MainForm::ExtractImage(wxCommandEvent& event)
{
	if (imageBox->lumpImage != nullptr)
	{
		if (imageBox->lumpImage->IsOk())
		{
			const UINT32 uiSelection = (UINT32)listBox->GetSelection();
			
			std::string fileFilters;
			fileFilters = "BMP Image|*.bmp";
			fileFilters += "|PNG Image|*.png";
			fileFilters += "|JPEG Image|*.jpeg";
			
			wxFileDialog saveDialog(this, "Extract to", wxEmptyString, listBox->GetString(uiSelection), fileFilters, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

			if (saveDialog.ShowModal() == wxID_CANCEL)
				return;
			
			std::string extCheck = saveDialog.GetPath().utf8_string();
			
			wxBitmapType TYPE;
			if (extCheck.rfind(".bmp") != std::string::npos)
				TYPE = wxBITMAP_TYPE_BMP;
			else if (extCheck.rfind(".png") != std::string::npos)
				TYPE = wxBITMAP_TYPE_PNG;
			else if (extCheck.rfind(".jpeg") != std::string::npos)
				TYPE = wxBITMAP_TYPE_JPEG;
			else
			{
				wxMessageBox("Invalid file type.", "Error", wxOK | wxICON_ERROR);
				return;
			}
			
			bool result = false;
			if (fileType == FILE_Wad)
				result = g_WAD.GetLumpImage(uiSelection, false)->SaveFile(saveDialog.GetPath(), TYPE);
			else if (fileType == FILE_Sprite)
			{
				std::vector<Frame> sprite = g_SPR.LoadFile(g_SPR.Filename, false);
				wxBitmap* image = sprite[uiSelection].Image;
				if (image != nullptr)
				{
					if (image->IsOk())
						result = image->SaveFile(saveDialog.GetPath(), TYPE);
				}
			}
			
			if (!result)
				wxMessageBox("Unable to extract image. (Write failure)", "Error", wxOK | wxICON_ERROR);
		}
	}
}

void MainForm::ExtractAllImages(wxCommandEvent& event)
{
	wxDirDialog saveDialog(this, "Extract to folder", wxEmptyString, wxDD_DEFAULT_STYLE);

	if (saveDialog.ShowModal() == wxID_CANCEL)
		return;
	
	const std::string destFolder = saveDialog.GetPath().utf8_string() + "/";
	
	if (fileType == FILE_Wad)
	{
		for (UINT32 i = 0; i < g_WAD.LumpsInfo.size(); i++)
		{
			wxBitmap* image = g_WAD.GetLumpImage(i, false);
			if (image != nullptr)
			{
				if (image->IsOk())
				{
					// null terminator on name it's getting on the way
					std::string textureName = g_WAD.LumpsInfo[i].Name; textureName.resize(textureName.length() - 1);
					
					switch (event.GetId())
					{
						case MENU_ExtractAllBMP: image->SaveFile(destFolder + textureName + ".bmp", wxBITMAP_TYPE_BMP); break;
						case MENU_ExtractAllPNG: image->SaveFile(destFolder + textureName + ".png", wxBITMAP_TYPE_PNG); break;
						case MENU_ExtractAllJPEG: image->SaveFile(destFolder + textureName + ".jpeg", wxBITMAP_TYPE_JPEG); break;
					}
				}
			}
		}
	}
	else if (fileType == FILE_Sprite)
	{
		std::vector<Frame> sprite = g_SPR.LoadFile(g_SPR.Filename, false);
		
		for (UINT32 i = 0; i < sprite.size(); i++)
		{
			wxBitmap* image = sprite[i].Image;
			if (image != nullptr)
			{
				if (image->IsOk())
				{
					char temp[12];
					sprintf(temp, "Frame #%i", i);
					std::string szFrame = temp;
					
					switch (event.GetId())
					{
						case MENU_ExtractAllBMP: image->SaveFile(destFolder + szFrame + ".bmp", wxBITMAP_TYPE_BMP); break;
						case MENU_ExtractAllPNG: image->SaveFile(destFolder + szFrame + ".png", wxBITMAP_TYPE_PNG); break;
						case MENU_ExtractAllJPEG: image->SaveFile(destFolder + szFrame + ".jpeg", wxBITMAP_TYPE_JPEG); break;
					}
				}
			}
		}
	}
}

void ImageBox::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	if (lumpImage != nullptr)
	{
		if (lumpImage->IsOk())
		{
			// calculate center
			wxSize screenSize = GetClientSize();
			int X = (screenSize.GetWidth() / 2) - (lumpImage->GetWidth() / 2);
			int Y = (screenSize.GetHeight() / 2) - (lumpImage->GetHeight() / 2);
			dc.DrawBitmap(*lumpImage, X, Y);
		}
	}

	event.Skip();
}

void ImageBox::Tick(wxTimerEvent& event)
{
	if (lastSpriteFrame >= (spriteImage.size() - 1))
		lastSpriteFrame = 0;
	else
		lastSpriteFrame++;

	lumpImage = spriteImage[lastSpriteFrame].Image;
	Refresh();

	event.Skip();
}

void ImageBox::OnResize(wxSizeEvent& event)
{
	Refresh();
	event.Skip();
}
