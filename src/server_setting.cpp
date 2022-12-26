/// @file server_setting.cpp
///
/// @brief MPPRINTER サーバーダイアログ制御
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#include "server_setting.h"

// Attach Event
BEGIN_EVENT_TABLE(ServerBox, wxDialog)
	// event
	EVT_BUTTON(wxID_OK, ServerBox::OnButton)
END_EVENT_TABLE()

ServerBox::ServerBox(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("Server settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer *gbox = new wxFlexGridSizer(3, 0, 0);

	txtHost = new wxTextCtrl(this, IDC_TXT_HOST);
	gbox->Add(new wxStaticText(this, wxID_ANY, _("Hostname:")), flags);
	gbox->Add(txtHost, flags);
	gbox->Add(new wxStaticText(this, wxID_ANY, wxT("")), flags);

	txtPort = new wxTextCtrl(this, IDC_TXT_PORT);
	gbox->Add(new wxStaticText(this, wxID_ANY, _("Port:")), flags);
	gbox->Add(txtPort, flags);
	gbox->Add(new wxStaticText(this, wxID_ANY, wxT("")), flags);

	szrAll->Add(gbox, flags);

	wxBoxSizer *hbox;

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkReverse = new wxCheckBox(this, IDC_CHK_REVERSE, _("Reverse received data."));
	hbox->Add(chkReverse, flags);
	szrAll->Add(hbox, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkEchoBack = new wxCheckBox(this, IDC_CHK_ECHOBACK, _("Echo back received data."));
	hbox->Add(chkEchoBack, flags);
	szrAll->Add(hbox, flags);

#ifdef USE_DELAY_SETTING
	wxStaticBoxSizer *sbox = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Delay Emulation")), wxVERTICAL);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkSendAck = new wxCheckBox(this, IDC_CHK_SENDACK, _("Send ACK(0x06) code when received data."));
	hbox->Add(chkSendAck, flags);
	sbox->Add(hbox, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	spinDelay = new wxSpinCtrl(this, IDC_SPIN_DELAY);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Delay:")), flags);
	hbox->Add(spinDelay, flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("msec.")), flags);
	sbox->Add(hbox, flags);

	szrAll->Add(sbox, flags);
#endif

	wxSizer *szrButtons = CreateButtonSizer(wxOK|wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

void ServerBox::init_dialog()
{
	txtHost->SetValue(mHost);
	txtPort->SetValue(wxString::Format(wxT("%ld"), mPort));
	chkReverse->SetValue(mReverse);
	chkEchoBack->SetValue(mEchoBack);
#ifdef USE_DELAY_SETTING
	chkSendAck->SetValue(mSendAck);
	spinDelay->SetRange(0, 10000);
	spinDelay->SetValue(mDelay);
#endif
}

void ServerBox::term_dialog()
{
	mHost = txtHost->GetValue();
	long l;
	txtPort->GetValue().ToLong(&l);
	if (0 < l && l <= 65535) mPort = l;
	mReverse = chkReverse->GetValue();
	mEchoBack = chkEchoBack->GetValue();
#ifdef USE_DELAY_SETTING
	mSendAck = chkSendAck->GetValue();
	int i = spinDelay->GetValue();
	if (0 <= i && i <= 10000) mDelay = i;
#endif
}

void ServerBox::OnButton(wxCommandEvent& event)
{
	wxDialog::EndDialog(wxID_OK);
}

int ServerBox::ShowModal()
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

