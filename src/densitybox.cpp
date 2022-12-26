/// @file densitybox.cpp
///
/// @brief MPPRINTER 濃度ダイアログ制御
///
/// @date 2015.11.27 Create
///
/// @author Copyright (c) 2015, Sasaji. All rights reserved.
///
#include "densitybox.h"

// Attach Event
BEGIN_EVENT_TABLE(DensityBox, wxDialog)
	// event
END_EVENT_TABLE()

DensityBox::DensityBox(wxWindow* parent, wxWindowID id, int density, bool reverse)
	: wxDialog(parent, id, _("Color Density"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
//	wxSizerFlags flags = wxSizerFlags(1).Expand();
	wxSize siz(200, -1);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Light")), flags);
	sliDensity = new wxSlider(this, IDC_SLIDER_DENSITY, density, 0, COLOR_DENSITY_MAX, wxDefaultPosition, siz, wxSL_HORIZONTAL);
	hbox->Add(sliDensity, flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Dark")), flags);
	szrAll->Add(hbox, 0);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkReverse = new wxCheckBox(this, IDC_CHECK_REVERSE, _("Reverse Color"));
	chkReverse->SetValue(reverse);
	hbox->Add(chkReverse, flags);
	szrAll->Add(hbox, 0);

	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int DensityBox::GetDensity()
{
	int val = 0;
	if (sliDensity) {
		val = sliDensity->GetValue();
	}
	return val;
}

bool DensityBox::GetReverse()
{
	bool val = false;
	if (chkReverse) {
		val = chkReverse->GetValue();
	}
	return val;
}
