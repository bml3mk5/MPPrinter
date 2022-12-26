/// @file server.cpp
///
/// @brief MPPRINTER サーバークラス
///
/// @date 2013.11.27 Create
///
/// @author Copyright (c) 2013, Sasaji. All rights reserved.
///
#include "server.h"
#include "errorinfo.h"
#include "config.h"

typedef wxIPV4address IPaddress;

//
// Server
//
// Attach Event
BEGIN_EVENT_TABLE(MpPrinterServer, wxEvtHandler)
	// event
	EVT_SOCKET(SERVER_ID, MpPrinterServer::OnServerEvent)
	EVT_SOCKET(SOCKET_ID, MpPrinterServer::OnSocketEvent)
	EVT_TIMER(TIMER_ID, MpPrinterServer::OnTimerEvent)
END_EVENT_TABLE()

MpPrinterServer::MpPrinterServer(MpPrinterFrame *parent)
	: wxEvtHandler()
{
	m_ok = false;
	frame = parent;
	sockets.Empty();
	mSocket = NULL;
	m_numClients = 0;

	mTimer.SetOwner(this, TIMER_ID);

	// load ini file
	wxString host = gConfig.GetServerHost();
	long port = gConfig.GetServerPort();

	IPaddress addr;
	addr.Hostname(host);
	addr.Service(wxString::Format(wxT("%ld"), port));

	// Create the socket
	m_server = new wxSocketServer(addr);

	// We use IsOk() here to see if the server is really listening
	if (!m_server->IsOk()) {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrServer, _("Could not listen at the specified port."));
		gErrInfo.ShowMsgBox(frame);
		return;
	}

	IPaddress addrReal;
	if ( !m_server->GetLocal(addrReal) ) {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrServer, _("Couldn't get the address we bound to"));
		gErrInfo.ShowMsgBox(frame);
		return;
	}
	AddDebugLog(wxString::Format(_T("Server listening at %s:%u"),
			 addrReal.IPAddress(), addrReal.Service()));

	// Setup the event handler and subscribe to connection events
	m_server->SetEventHandler(*this, SERVER_ID);
	m_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	m_server->Notify(true);

	m_ok = true;
	return;
}
MpPrinterServer::~MpPrinterServer()
{
	for(size_t i=0; i<sockets.GetCount(); i++) {
		wxSocketBase *sock = (wxSocketBase *)sockets.Item(i);
		if (sock->IsConnected()) {
			sock->Destroy();
		}
	}
	delete m_server;
	m_server = NULL;
	AddDebugLog(_T("Server stop."));
}

void MpPrinterServer::OnServerEvent(wxSocketEvent& event)
{
	wxSocketBase *sock;

	sock = m_server->Accept(false);

	if (sock) {
		IPaddress addr;
		if ( !sock->GetPeer(addr) ) {
			AddDebugLog(_T("New connection from unknown client accepted."));
		} else {
			AddDebugLog(wxString::Format(_T("New client connection from %s:%u accepted"),
				   addr.IPAddress(), addr.Service()));
		}
	} else {
		gErrInfo.SetInfo(__FILE__, __LINE__, myError, myErrConnect);
		gErrInfo.ShowMsgBox(frame);
		return;
	}

	sock->SetEventHandler(*this, SOCKET_ID);
	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	sock->Notify(true);

	sockets.Add((void *)sock);

	m_numClients++;
}

void MpPrinterServer::OnSocketEvent(wxSocketEvent& event)
{
	wxSocketBase *sock = event.GetSocket();

	// Now we process the event
	switch(event.GetSocketEvent()) {
		case wxSOCKET_INPUT:
			{
				// We disable input events, so that the test doesn't trigger
				// wxSocketEvent again.
				sock->SetNotify(wxSOCKET_LOST_FLAG);

				// Which test are we going to run?
				sock->Read((void *)mReadBuffer, MAX_READABLE_BUFFER);
				wxUint32 len = sock->LastReadCount();
				if (sock->Error()) {
					// error
					AddDebugLog(wxString::Format(_T("Socket read error: %d"),
						   sock->LastError()));
				} else {
					frame->SetData(mReadBuffer, len);
					if (gConfig.GetSendAck() && !mTimer.IsRunning()) {
						int msec = gConfig.GetPrintDelay();
						mTimer.Start(msec, true);
						mSocket = sock;
					}
					if (gConfig.GetEchoBack()) {
						sock->Write(mReadBuffer, len);
					}
				}

				// Enable input events again.
				sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
				break;
			}
		case wxSOCKET_LOST:
			{
				m_numClients--;
				AddDebugLog(_T("Deleting socket."));
				sock->Destroy();
				sockets.Remove((void *)sock);
				break;
			}
		default:
			break;
	}
}

void MpPrinterServer::OnTimerEvent(wxTimerEvent& event)
{
	// send message to client
	if (mSocket && mSocket->IsConnected()) {
//		mSocket->Write("ACK", 3);
		mSocket->Write("\x06", 1);
		mSocket = NULL;
	}
	mTimer.Stop();
}
