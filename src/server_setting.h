/// @file server_setting.h
///
/// @brief MPPRINTER サーバーダイアログ制御
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#ifndef _SERVER_SETTING_H_
#define _SERVER_SETTING_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/cmndata.h>

#undef USE_DELAY_SETTING

/// @brief サーバ設定ダイアログ
class ServerBox : public wxDialog
{
private:
	wxTextCtrl *txtHost;
	wxTextCtrl *txtPort;
	wxCheckBox *chkReverse;
	wxCheckBox *chkEchoBack;
#ifdef USE_DELAY_SETTING
	wxCheckBox *chkSendAck;
	wxSpinCtrl *spinDelay;
#endif
	wxString mHost;
	long mPort;
	bool mReverse;
	bool mEchoBack;
	bool mSendAck;
	int  mDelay;

	void init_dialog();
	void term_dialog();

public:
	ServerBox(wxWindow* parent, wxWindowID id);

	enum {
		IDC_TXT_HOST = 1,
		IDC_TXT_PORT,
		IDC_CHK_REVERSE,
		IDC_CHK_ECHOBACK,
		IDC_CHK_SENDACK,
		IDC_SPIN_DELAY,
	};

	// function
	int ShowModal();

	// event handler
	void OnButton(wxCommandEvent& event);

	// properties
	void SetHostname(const wxString &val) { mHost = val; }
	const wxString &GetHostname() const { return mHost; }
	void SetPort(long val) { mPort = val; }
	long GetPort() const { return mPort; }
	void SetReverse(bool val) { mReverse = val; }
	bool GetReverse() const { return mReverse; }
	void SetEchoBack(bool val) { mEchoBack = val; }
	bool GetEchoBack() const { return mEchoBack; }
	void SetSendAck(bool val) { mSendAck = val; }
	bool GetSendAck() const { return mSendAck; }
	void SetDelay(int val) { mDelay = val; }
	int GetDelay() const { return mDelay; }

	DECLARE_EVENT_TABLE()
};

#endif /* _SERVER_SETTING_H_ */
