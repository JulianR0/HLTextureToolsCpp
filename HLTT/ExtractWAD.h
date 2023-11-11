#pragma once

#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/stattext.h>
#include <wx/filepicker.h>
#include <wx/button.h>

#ifdef linux
#include "linux.h"
#endif

enum
{
	BUTTON_Extract = 90
};

class ExtractWADForm : public wxDialog
{
public:
	wxFilePickerCtrl *inputFile;
	wxFilePickerCtrl *outputFile;

	ExtractWADForm(wxFrame *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString &name = wxDialogNameStr);

private:
	void ExtractButtonClick(wxCommandEvent& event);
};
