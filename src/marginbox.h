/// @file marginbox.h
///
/// @brief MPPRINTER マージンダイアログ制御
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#ifndef _MARGINBOX_H_
#define _MARGINBOX_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/cmndata.h>

/// MPPRINTER マージンダイアログ
class MarginBox : public wxDialog
{
private:
	wxPageSetupDialogData *pdata;

	wxSpinCtrl *spinTop;
	wxSpinCtrl *spinBottom;

	void init_dialog();
	void term_dialog();

public:
	MarginBox(wxWindow* parent, wxWindowID id, wxPageSetupDialogData *data);

	enum {
		IDC_SPIN_TOP = 1,
		IDC_SPIN_BOTTOM,
	};

	// function
	int ShowModal();

	// event handler
	void OnButton(wxCommandEvent& event);

	// properties

	DECLARE_EVENT_TABLE()
};

#endif /* _MARGINBOX_H_ */
