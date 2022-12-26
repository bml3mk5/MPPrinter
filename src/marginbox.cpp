/// @file marginbox.cpp
///
/// @brief MPPRINTER マージンダイアログ制御
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#include "marginbox.h"

// Attach Event
BEGIN_EVENT_TABLE(MarginBox, wxDialog)
	// event
	EVT_BUTTON(wxID_OK, MarginBox::OnButton)
END_EVENT_TABLE()

MarginBox::MarginBox(wxWindow* parent, wxWindowID id, wxPageSetupDialogData *data)
	: wxDialog(parent, id, _("Margins"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	pdata = data;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer *gbox = new wxFlexGridSizer(2,3,0,0);

	spinTop = new wxSpinCtrl(this, IDC_SPIN_TOP);
	gbox->Add(new wxStaticText(this, wxID_ANY, _("Top:")), flags);
	gbox->Add(spinTop, flags);
	gbox->Add(new wxStaticText(this, wxID_ANY, _("mm")), flags);

	spinBottom = new wxSpinCtrl(this, IDC_SPIN_BOTTOM);
	gbox->Add(new wxStaticText(this, wxID_ANY, _("Bottom:")), flags);
	gbox->Add(spinBottom, flags);
	gbox->Add(new wxStaticText(this, wxID_ANY, _("mm")), flags);

    szrAll->Add(gbox, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

void MarginBox::init_dialog()
{
	wxSize sz =	pdata->GetPaperSize();
	wxPoint tl = pdata->GetMarginTopLeft();
	wxPoint br = pdata->GetMarginBottomRight();

	spinTop->SetRange(0, sz.GetHeight());
	spinTop->SetValue(tl.y);
	spinBottom->SetRange(0, sz.GetHeight());
	spinBottom->SetValue(br.y);
}

void MarginBox::term_dialog()
{
	int top = spinTop->GetValue();
	int bottom = spinBottom->GetValue();
	wxPoint tl = pdata->GetMarginTopLeft();
	wxPoint br = pdata->GetMarginBottomRight();
	tl.y = top;
	br.y = bottom;
	pdata->SetMarginTopLeft(tl);
	pdata->SetMarginBottomRight(br);
}

void MarginBox::OnButton(wxCommandEvent& event)
{
	wxSize sz =	pdata->GetPaperSize();
	int top = spinTop->GetValue();
	int bottom = spinBottom->GetValue();

	if (sz.GetHeight() - top - bottom <= 0) {
		wxMessageBox(_("Size is too large."));
		return;
	}
	wxDialog::EndDialog(wxID_OK);
}

int MarginBox::ShowModal()
{
	// initialize
	init_dialog();
	int rc = wxDialog::ShowModal();
	if (rc == wxID_OK) {
		// update
		term_dialog();
	}
	return rc;
}

