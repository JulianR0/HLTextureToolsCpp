#include "ExtractWAD.h"

#include "WAD3Loader.h"
extern WAD3Loader g_WAD;

ExtractWADForm::ExtractWADForm(wxFrame *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name) : wxDialog(parent, id, title, pos, size, style, name)
{
	wxBoxSizer *wadSizer = new wxBoxSizer(wxVERTICAL);
	
	wxStaticText *inputLabel = new wxStaticText(this, wxID_ANY, "BSP file to extract from:");
	inputFile = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, "Select a file", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_USE_TEXTCTRL);
	
	wxStaticText *outputLabel = new wxStaticText(this, wxID_ANY, "Where to save WAD file:");
	outputFile = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, "Select a file", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT | wxFLP_USE_TEXTCTRL);
	
	wxButton *extractButton = new wxButton(this, BUTTON_Extract, "Extract");
	Bind(wxEVT_BUTTON, &ExtractWADForm::ExtractButtonClick, this, BUTTON_Extract);
	
	wadSizer->Add(inputLabel, 0, wxEXPAND | wxALL, 10);
	wadSizer->Add(inputFile, 0, wxEXPAND | wxALL, 10);
	
	wadSizer->Add(outputLabel, 0, wxEXPAND | wxALL, 10);
	wadSizer->Add(outputFile, 0, wxEXPAND | wxALL, 10);
	
	wadSizer->Add( 0, 24, 0, wxEXPAND, 10 ); /* spacer */
	
	wadSizer->Add(extractButton, 0, wxALIGN_RIGHT | wxALL, 10);
	
	SetSizer(wadSizer);
}

void ExtractWADForm::ExtractButtonClick(wxCommandEvent& event)
{
	g_WAD.ExtractWADFromBSP(inputFile->GetPath().utf8_string(), outputFile->GetPath().utf8_string());
}
