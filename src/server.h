/// @file server.h
///
/// @brief MPPRINTER サーバークラス
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#ifndef _MPPRINTER_SERVER_H_
#define _MPPRINTER_SERVER_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/sckipc.h>
#include "main.h"

#define MAX_READABLE_BUFFER 1024

class MpPrinterFrame;

/// @brief MPPRINTER サーバクラス
class MpPrinterServer : public wxEvtHandler
{
private:
	MpPrinterFrame *frame;
	wxSocketServer *m_server;
	bool m_ok;
	int m_numClients;

	wxTimer mTimer;
	wxSocketBase *mSocket;

	wxByte mReadBuffer[MAX_READABLE_BUFFER];

	wxArrayPtrVoid sockets;

public:
	MpPrinterServer(MpPrinterFrame *parent);
	~MpPrinterServer();

	// event procedures
	void OnServerEvent(wxSocketEvent& event);
	void OnSocketEvent(wxSocketEvent& event);
	void OnTimerEvent(wxTimerEvent& event);

	// properties
	bool IsOk() { return m_ok; }

	enum {
		SERVER_ID = 10,
		SOCKET_ID,
		TIMER_ID,
	};

	DECLARE_EVENT_TABLE()
};

#endif /* _MPPRINTER_SERVER_H_ */

