/// @file ctrl_setting.cpp
///
/// @brief MPPRINTER 設定ダイアログ制御
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#include "ctrl_setting.h"

// Attach Event
BEGIN_EVENT_TABLE(CtrlSettingBox, wxDialog)
	// event
	EVT_RADIOBUTTON(IDC_RADIO_HI_6INCH, CtrlSettingBox::OnChecked)
	EVT_RADIOBUTTON(IDC_RADIO_HI_8INCH, CtrlSettingBox::OnChecked)
	EVT_RADIOBUTTON(IDC_RADIO_HI_NINCH, CtrlSettingBox::OnChecked)
	EVT_RADIOBUTTON(IDC_RADIO_EP_6INCH, CtrlSettingBox::OnChecked)
	EVT_RADIOBUTTON(IDC_RADIO_EP_8INCH, CtrlSettingBox::OnChecked)
	EVT_RADIOBUTTON(IDC_RADIO_EP_NINCH, CtrlSettingBox::OnChecked)
END_EVENT_TABLE()

CtrlSettingBox::CtrlSettingBox(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("Settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
//	wxSizerFlags flags = wxSizerFlags(1).Expand();

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	book = new wxNotebook(this, IDC_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
    book->AddPage(CreateHiPage(book), _("HI type"));
    book->AddPage(CreateEpPage(book), _("EP type"));

    szrAll->Add(book, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

wxWindow *CtrlSettingBox::CreateHiPage(wxWindow *parent)
{
//	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 4);

    wxPanel *page = new wxPanel(parent);
    wxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *box1 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Newline height")), wxHORIZONTAL);
	mHiCtrl.radNewline[0] = new wxRadioButton(page, IDC_RADIO_HI_6INCH, _("1/6inch"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mHiCtrl.radNewline[1] = new wxRadioButton(page, IDC_RADIO_HI_8INCH, _("1/8inch"));
	mHiCtrl.radNewline[2] = new wxRadioButton(page, IDC_RADIO_HI_NINCH, _("n/144inch"));
	mHiCtrl.spinNewline = new wxSpinCtrl(page, IDC_SPIN_HI_NINCH);
	box1->Add(mHiCtrl.radNewline[0], flags);
	box1->Add(mHiCtrl.radNewline[1], flags);
	box1->Add(mHiCtrl.radNewline[2], flags);
	box1->Add(mHiCtrl.spinNewline, flags);
	vbox->Add(box1, flags);

	wxStaticBoxSizer *box2 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Charactor width")), wxHORIZONTAL);
	mHiCtrl.radCharactor[0] = new wxRadioButton(page, IDC_RADIO_HI_CNORMAL, _("Normal"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mHiCtrl.radCharactor[1] = new wxRadioButton(page, IDC_RADIO_HI_CWIDE, _("Wide"));
	box2->Add(mHiCtrl.radCharactor[0], flags);
	box2->Add(mHiCtrl.radCharactor[1], flags);
	vbox->Add(box2, flags);

	wxStaticBoxSizer *box3 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Line width")), wxHORIZONTAL);
	mHiCtrl.radLine[0] = new wxRadioButton(page, IDC_RADIO_HI_L132, _("132 chars"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mHiCtrl.radLine[1] = new wxRadioButton(page, IDC_RADIO_HI_L80, _("80 chars"));
	mHiCtrl.radLine[2] = new wxRadioButton(page, IDC_RADIO_HI_L64, _("64 chars"));
	box3->Add(mHiCtrl.radLine[0], flags);
	box3->Add(mHiCtrl.radLine[1], flags);
	box3->Add(mHiCtrl.radLine[2], flags);
	vbox->Add(box3, flags);

	wxStaticBoxSizer *box4 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Charactor code set")), wxHORIZONTAL);
	mHiCtrl.radCodeset[0] = new wxRadioButton(page, IDC_RADIO_HI_CSET_SI, _("SI(interace)"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mHiCtrl.radCodeset[1] = new wxRadioButton(page, IDC_RADIO_HI_CSET_SO, _("SO(non-interace)"));
	box4->Add(mHiCtrl.radCodeset[0], flags);
	box4->Add(mHiCtrl.radCodeset[1], flags);
	vbox->Add(box4, flags);

    page->SetSizer(vbox);

	return page;
}

wxWindow *CtrlSettingBox::CreateEpPage(wxWindow *parent)
{
//	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 4);

    wxPanel *page = new wxPanel(parent);
    wxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *box1 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Newline height")), wxHORIZONTAL);
	mEpCtrl.radNewline[0] = new wxRadioButton(page, IDC_RADIO_EP_6INCH, _("1/6inch"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mEpCtrl.radNewline[1] = new wxRadioButton(page, IDC_RADIO_EP_8INCH, _("1/8inch"));
	mEpCtrl.radNewline[2] = new wxRadioButton(page, IDC_RADIO_EP_NINCH, _("n/144inch"));
	mEpCtrl.spinNewline = new wxSpinCtrl(page, IDC_SPIN_EP_NINCH);
	box1->Add(mEpCtrl.radNewline[0], flags);
	box1->Add(mEpCtrl.radNewline[1], flags);
	box1->Add(mEpCtrl.radNewline[2], flags);
	box1->Add(mEpCtrl.spinNewline, flags);
	vbox->Add(box1, flags);

	wxSizer *sbox1 = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *sbox1l = new wxBoxSizer(wxVERTICAL);
	wxSizer *sbox1r = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *box2 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Charactor width")), wxHORIZONTAL);
	mEpCtrl.radCharactor[0] = new wxRadioButton(page, IDC_RADIO_EP_CNORMAL, _("Normal"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mEpCtrl.radCharactor[1] = new wxRadioButton(page, IDC_RADIO_EP_CWIDE, _("Wide"));
	box2->Add(mEpCtrl.radCharactor[0], flags);
	box2->Add(mEpCtrl.radCharactor[1], flags);
	sbox1l->Add(box2, flags);

	wxStaticBoxSizer *box3 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Line width")), wxHORIZONTAL);
	mEpCtrl.radLine[0] = new wxRadioButton(page, IDC_RADIO_EP_L132, _("132 chars"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mEpCtrl.radLine[1] = new wxRadioButton(page, IDC_RADIO_EP_L80, _("80 chars"));
	box3->Add(mEpCtrl.radLine[0], flags);
	box3->Add(mEpCtrl.radLine[1], flags);
	sbox1l->Add(box3, flags);

	wxStaticBoxSizer *box4 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("CPI")), wxHORIZONTAL);
	mEpCtrl.radCpi[0] = new wxRadioButton(page, IDC_RADIO_EP_CPI10, _("10cpi"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mEpCtrl.radCpi[1] = new wxRadioButton(page, IDC_RADIO_EP_CPI12, _("12cpi"));
	box4->Add(mEpCtrl.radCpi[0], flags);
	box4->Add(mEpCtrl.radCpi[1], flags);
	sbox1l->Add(box4, flags);

	wxStaticBoxSizer *box5 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Charactor code set")), wxVERTICAL);
	wxSizer *hbox;
	hbox = new wxBoxSizer(wxHORIZONTAL);
	mEpCtrl.radCodeset[0] = new wxRadioButton(page, IDC_RADIO_EP_CSET_1, _("L3 Graph"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	mEpCtrl.radCodeset[1] = new wxRadioButton(page, IDC_RADIO_EP_CSET_2, _("L3 Hiragana"));
	hbox->Add(mEpCtrl.radCodeset[0], flags);
	hbox->Add(mEpCtrl.radCodeset[1], flags);
	box5->Add(hbox, flags);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	mEpCtrl.radCodeset[2] = new wxRadioButton(page, IDC_RADIO_EP_CSET_3, _("FM-8/7"));
	hbox->Add(mEpCtrl.radCodeset[2], flags);
	box5->Add(hbox, flags);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	mEpCtrl.radCodeset[3] = new wxRadioButton(page, IDC_RADIO_EP_CSET_4, _("PC-8001"));
	hbox->Add(mEpCtrl.radCodeset[3], flags);
	box5->Add(hbox, flags);
	sbox1r->Add(box5, flags);

	wxSizerFlags flag0 = wxSizerFlags().Border(wxALL, 0);

	sbox1->Add(sbox1l, flag0);
	sbox1->Add(sbox1r, flag0);
	vbox->Add(sbox1, flag0);

    page->SetSizer(vbox);

	return page;
}

void CtrlSettingBox::InitHiPage()
{
	CtrlSettingBoxParam *pm = &param[0];

	// 改行幅
	pm->height6 = pm->ppiy / 6.0;
	pm->height8 = pm->ppiy / 8.0;

	wxString text_buf;
	text_buf.Printf(_T("n/%d"), pm->ppiy);
	text_buf += _("inch");
	mHiCtrl.radNewline[2]->SetLabel(text_buf);

	if (pm->line_feed_height == pm->height6) {
		mHiCtrl.radNewline[0]->SetValue(true);
		mHiCtrl.spinNewline->Enable(false);
	} else if (pm->line_feed_height == pm->height8) {
		mHiCtrl.radNewline[1]->SetValue(true);
		mHiCtrl.spinNewline->Enable(false);
	} else {
		mHiCtrl.radNewline[2]->SetValue(true);
		mHiCtrl.spinNewline->Enable(true);
	}
	// 改行幅数値
	mHiCtrl.spinNewline->SetRange(1,255);
	mHiCtrl.spinNewline->SetValue(pm->line_feed_height);

	// 文字幅
	switch (pm->wide_mode) {
		case 0:
			mHiCtrl.radCharactor[0]->SetValue(true);
			break;
		case 1:
			mHiCtrl.radCharactor[1]->SetValue(true);
			break;
		default:
			break;
	}

	// 行幅
	switch (pm->line_mode) {
		case 0:
			mHiCtrl.radLine[0]->SetValue(true);
			break;
		case 1:
			mHiCtrl.radLine[1]->SetValue(true);
			break;
		case 2:
			mHiCtrl.radLine[2]->SetValue(true);
			break;
		default:
			break;
	}

	// 文字コード
	switch (pm->char_set) {
		case 0:
			mHiCtrl.radCodeset[0]->SetValue(true);
			break;
		case 1:
			mHiCtrl.radCodeset[1]->SetValue(true);
			break;
		default:
			break;
	}

}

void CtrlSettingBox::InitEpPage()
{
	CtrlSettingBoxParam *pm = &param[1];

	// 改行幅
	pm->height6 = pm->ppiy / 6.0;
	pm->height8 = pm->ppiy / 8.0;

	wxString text_buf;
	text_buf.Printf(_T("n/%d"), pm->ppiy);
	text_buf += _("inch");
	mEpCtrl.radNewline[2]->SetLabel(text_buf);

	if (pm->line_feed_height == pm->height6) {
		mEpCtrl.radNewline[0]->SetValue(true);
		mEpCtrl.spinNewline->Enable(false);
	} else if (pm->line_feed_height == pm->height8) {
		mEpCtrl.radNewline[1]->SetValue(true);
		mEpCtrl.spinNewline->Enable(false);
	} else {
		mEpCtrl.radNewline[2]->SetValue(true);
		mEpCtrl.spinNewline->Enable(true);
	}
	// 改行幅数値
	mEpCtrl.spinNewline->SetRange(1,255);
	mEpCtrl.spinNewline->SetValue(pm->line_feed_height);

	// 文字幅
	switch (pm->wide_mode) {
		case 0:
			mEpCtrl.radCharactor[0]->SetValue(true);
			break;
		case 1:
		case 2:
			mEpCtrl.radCharactor[1]->SetValue(true);
			break;
		default:
			break;
	}

	// 行幅
	switch (pm->line_mode) {
		case 0:
			mEpCtrl.radLine[0]->SetValue(true);
			break;
		case 1:
			mEpCtrl.radLine[1]->SetValue(true);
			break;
		default:
			break;
	}

	// CPI
	switch (pm->char_per_inch) {
		case 10:
			mEpCtrl.radCpi[0]->SetValue(true);
			break;
		case 12:
			mEpCtrl.radCpi[1]->SetValue(true);
			break;
		default:
			break;
	}

	// 文字コード
	if (0 <= pm->char_set && pm->char_set <= 3) {
		mEpCtrl.radCodeset[pm->char_set]->SetValue(true);
	}
}

void CtrlSettingBox::OnChecked(wxCommandEvent& event)
{
	int id = event.GetId();
	switch(id) {
		case IDC_RADIO_HI_6INCH:
		case IDC_RADIO_HI_8INCH:
			mHiCtrl.spinNewline->Enable(false);
			break;
		case IDC_RADIO_HI_NINCH:
			mHiCtrl.spinNewline->Enable(true);
			break;
		case IDC_RADIO_EP_6INCH:
		case IDC_RADIO_EP_8INCH:
			mEpCtrl.spinNewline->Enable(false);
			break;
		case IDC_RADIO_EP_NINCH:
			mEpCtrl.spinNewline->Enable(true);
			break;
	}
}

void CtrlSettingBox::UpdateHiPage()
{
	CtrlSettingBoxParam *pm = &param[0];

	// 改行幅
	if (mHiCtrl.radNewline[1]->GetValue()) {
		pm->line_feed_height = pm->height8;
	} else if (mHiCtrl.radNewline[2]->GetValue()) {
		pm->line_feed_height = mHiCtrl.spinNewline->GetValue();
	} else {
		pm->line_feed_height = pm->height6;
	}

	// 文字幅
	if (mHiCtrl.radCharactor[1]->GetValue()) {
		pm->wide_mode = 1;
	} else {
		pm->wide_mode = 0;
	}

	// 行幅
	if (mHiCtrl.radLine[2]->GetValue()) {
		pm->line_mode = 2;
	} else if (mHiCtrl.radLine[0]->GetValue()) {
		pm->line_mode = 0;
	} else {
		pm->line_mode = 1;
	}

	// 文字コード
	if (mHiCtrl.radCodeset[1]->GetValue()) {
		pm->char_set = 1;
	} else {
		pm->char_set = 0;
	}
}

void CtrlSettingBox::UpdateEpPage()
{
	CtrlSettingBoxParam *pm = &param[1];

	// 改行幅
	if (mEpCtrl.radNewline[1]->GetValue()) {
		pm->line_feed_height = pm->height8;
	} else if (mEpCtrl.radNewline[2]->GetValue()) {
		pm->line_feed_height = mEpCtrl.spinNewline->GetValue();
	} else {
		pm->line_feed_height = pm->height6;
	}

	// 文字幅
	if (mEpCtrl.radCharactor[1]->GetValue()) {
		pm->wide_mode |= 2;
	} else {
		pm->wide_mode &= ~2;
	}

	// 行幅
	if (mEpCtrl.radLine[0]->GetValue()) {
		pm->line_mode = 0;
	} else {
		pm->line_mode = 1;
	}

	// CPI
	if (mEpCtrl.radCpi[1]->GetValue()) {
		pm->char_per_inch = 12;
	} else {
		pm->char_per_inch = 10;
	}

	// 文字コード
	for(int i=0; i<=3; i++) {
		if (mEpCtrl.radCodeset[i]->GetValue()) {
			pm->char_set = i;
			break;
		}
	}
}

int CtrlSettingBox::ShowModal()
{
	// initialize
	InitHiPage();
	InitEpPage();
	int rc = wxDialog::ShowModal();
	if (rc == wxID_OK) {
		// update
		UpdateHiPage();
		UpdateEpPage();
	}
	return rc;
}

void CtrlSettingBox::SelectTabNo(int no)
{
	book->SetSelection((size_t)no);
}
void CtrlSettingBox::SetPPIX(int idx, int val)
{
	param[idx].ppix = val;
}
void CtrlSettingBox::SetPPIY(int idx, int val)
{
	param[idx].ppiy = val;
}
void CtrlSettingBox::SetLineFeedHeight(int idx,double val)
{
	param[idx].line_feed_height = val;
}
double CtrlSettingBox::GetLineFeedHeight(int idx)
{
	return param[idx].line_feed_height;
}
void CtrlSettingBox::SetWideMode(int idx, int val)
{
	param[idx].wide_mode = val;
}
int  CtrlSettingBox::GetWideMode(int idx)
{
	return param[idx].wide_mode;
}
void CtrlSettingBox::SetLineMode(int idx, int val)
{
	param[idx].line_mode = val;
}
int  CtrlSettingBox::GetLineMode(int idx)
{
	return param[idx].line_mode;
}
void CtrlSettingBox::SetCharSet(int idx, int val)
{
	param[idx].char_set = val;
}
int  CtrlSettingBox::GetCharSet(int idx)
{
	return param[idx].char_set;
}
void CtrlSettingBox::SetCPI(int idx, int val)
{
	param[idx].char_per_inch = val;
}
int  CtrlSettingBox::GetCPI(int idx)
{
	return param[idx].char_per_inch;
}

