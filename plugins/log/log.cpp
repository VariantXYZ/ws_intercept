#include <windows.h>
#include "log.h"

DWORD WINAPI setup_console(LPVOID param);

void WINAPI log_send(SOCKET& s, const char *buf, int& len, int& flags);
void WINAPI log_recv(SOCKET& s, const char *buf, int& len, int& flags);

static DWORD threadIDConsole = 0;

extern "C" BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL,0,setup_console,NULL,0,NULL);
			register_handler(0x0, tWS_plugin func, WS_HANDLER_BOTH);
			break;
		case DLL_PROCESS_DETACH:
			if (threadIDConsole)
				PostThreadMessage(threadIDConsole, WM_QUIT, 0, 0);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

void WINAPI repl_send(SOCKET& s, const char *buf, int& len, int& flags)
{
	struct sockaddr_in info;
	int infolen;
	getpeername(s,(sockaddr*)(&info),&infolen);
	const int port = ntohs(info.sin_port);
	LOG("SEND");
	LOG("%s:%u, Len %d, Flags %d, socket %u",inet_ntoa(info.sin_addr), port, len, flags, s);
	LOGn("Data: ");  
	for(int i = 0; i < len; i++) 
		LOGn("%02X ",(unsigned char)buf_new[i]);
	LOGn("\n");	
	return;
}

void WINAPI repl_recv(SOCKET& s, const char *buf, int& len, int& flags)
{
	struct sockaddr_in info;
	int infolen;
	getpeername(s,(sockaddr*)(&info),&infolen);
	const int port = ntohs(info.sin_port);
	LOG("RECV");
	LOG("%s:%u, Len %d, Flags %d, socket %u",inet_ntoa(info.sin_addr), port, len, flags, s);
	LOGn("Data: ");  
	for(int i = 0; i < len; i++) 
		LOGn("%02X ",(unsigned char)buf_new[i]);
	LOGn("\n");		
	return;
}

DWORD WINAPI setup_console(LPVOID param)
{
	AllocConsole();
	freopen("CONOUT$","w",stdout);
	freopen("CONIN$","r",stdin);
	while(true)
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			switch (msg.message) 
			{
				case WM_QUIT:
					FreeConsole();		
					return msg.wParam;
			}
		}
	}
	return 0;
}



