/// @file ctrl_setting.h
///
/// @brief MPPRINTER 設定ダイアログ制御
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#ifndef _CTRLSETTING_H_
#define _CTRLSETTING_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>

/// @brief MPPRINTER 設定ダイアログ用のパラメータ
class CtrlSettingBoxParam
{
public:
	int ppix;
	int ppiy;
	double height6;
	double height8;

	double line_feed_height;
	int wide_mode;
	int line_mode;
	int char_set;
	int char_per_inch;
};

/// @brief MPPRINTER 設定ダイアログ
class CtrlSettingBox : public wxDialog
{
private:
	CtrlSettingBoxParam param[2];

	wxNotebook *book;
	/// @brief 設定ダイアログ 内部コントロール
	struct stCtrl {
		wxRadioButton *radNewline[3];
		wxSpinCtrl *spinNewline;

		wxRadioButton *radCharactor[2];

		wxRadioButton *radLine[3];

		wxRadioButton *radCodeset[4];

		wxRadioButton *radCpi[2];
	};

	struct stCtrl mHiCtrl;
	struct stCtrl mEpCtrl;

	wxWindow *CreateHiPage(wxWindow *parent);
	wxWindow *CreateEpPage(wxWindow *parent);
	void InitHiPage();
	void InitEpPage();
	void UpdateHiPage();
	void UpdateEpPage();

public:
	CtrlSettingBox(wxWindow* parent, wxWindowID id);

	enum {
		IDC_NOTEBOOK = 1,

		IDC_RADIO_HI_6INCH,
		IDC_RADIO_HI_8INCH,
		IDC_RADIO_HI_NINCH,
		IDC_SPIN_HI_NINCH,

		IDC_RADIO_HI_CNORMAL,
		IDC_RADIO_HI_CWIDE,

		IDC_RADIO_HI_L132,
		IDC_RADIO_HI_L80,
		IDC_RADIO_HI_L64,

		IDC_RADIO_HI_CSET_SI,
		IDC_RADIO_HI_CSET_SO,

		IDC_RADIO_EP_6INCH,
		IDC_RADIO_EP_8INCH,
		IDC_RADIO_EP_NINCH,
		IDC_SPIN_EP_NINCH,

		IDC_RADIO_EP_CNORMAL,
		IDC_RADIO_EP_CWIDE,

		IDC_RADIO_EP_L132,
		IDC_RADIO_EP_L80,

		IDC_RADIO_EP_CPI10,
		IDC_RADIO_EP_CPI12,

		IDC_RADIO_EP_CSET_1,
		IDC_RADIO_EP_CSET_2,
		IDC_RADIO_EP_CSET_3,
		IDC_RADIO_EP_CSET_4,
	};
	// function
	int ShowModal();

	// event handler
	void OnChecked(wxCommandEvent& event);

	// properties
	void SelectTabNo(int);

	void SetPPIX(int, int);
	void SetPPIY(int, int);
	void SetLineFeedHeight(int, double);
	double GetLineFeedHeight(int);
	void SetWideMode(int, int);
	int  GetWideMode(int);
	void SetLineMode(int, int);
	int  GetLineMode(int);
	void SetCharSet(int, int);
	int  GetCharSet(int);
	void SetCPI(int, int);
	int  GetCPI(int);

	DECLARE_EVENT_TABLE()
};

#endif /* _CTRLSETTING_H_ */
