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
	MENU_Animate = 20,
	MENU_Transparent
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

	wxMenu *menuImage;
	wxMenu *menuSettings;

	bool drawTransparent;
	MainForm();

private:
	void OnOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void ListBoxChanged(wxCommandEvent& event);
	void AnimateSprite(wxCommandEvent& event);
	void ToggleTransparency(wxCommandEvent& event);
};

bool System::OnInit()
{
	MainForm *main = new MainForm();
	main->Show();
	return true;
}

MainForm::MainForm() : wxFrame(nullptr, wxID_ANY, "Half-Life Texture Tools", wxDefaultPosition, wxSize(960, 544))
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(wxID_OPEN);
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	
	menuImage = new wxMenu;
	menuImage->Append(MENU_Animate, "Animation Play/Stop\tF5", "Animate the sprite");
	menuImage->Enable(MENU_Animate, false);

	menuSettings = new wxMenu;
	menuSettings->Append(MENU_Transparent, "Transparent textures", "Enable transparency for textures and sprites", wxITEM_CHECK);
	menuSettings->Check(MENU_Transparent, true);
	drawTransparent = true;

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "File");
	menuBar->Append(menuImage, "Image");
	menuBar->Append(menuSettings, "Settings");

	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &MainForm::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainForm::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainForm::AnimateSprite, this, MENU_Animate);
	Bind(wxEVT_MENU, &MainForm::ToggleTransparency, this, MENU_Transparent);

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
