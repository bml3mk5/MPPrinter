/// @file densitybox.h
///
/// @brief MPPRINTER 濃度ダイアログ制御
///
/// @date 2015.11.27 Create
///
/// @author Copyright (c) 2015, Sasaji. All rights reserved.
///
#ifndef _DENSITYBOX_H_
#define _DENSITYBOX_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dialog.h>

/// MPPRINTER 濃度ダイアログ
class DensityBox : public wxDialog
{
private:
	wxSlider *sliDensity;
	wxCheckBox *chkReverse;

public:
	DensityBox(wxWindow* parent, wxWindowID id, int density, bool reverse);

	enum {
		IDC_SLIDER_DENSITY = 1,
		IDC_CHECK_REVERSE,
	};

	// function

	// event handler

	// properties
	int GetDensity();
	bool GetReverse();

	DECLARE_EVENT_TABLE()
};

#endif /* _DENSITYBOX_H_ */
