#include <winsock2.h>
#include <windows.h>
#include "log.h"

static DWORD WINAPI setup_console(LPVOID param);

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags);

static DWORD threadIDConsole = 0;

static DWORD plugin_id_send = NULL;
static DWORD plugin_id_recv = NULL;

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL,0,setup_console,NULL,0,&threadIDConsole);
			plugin_id_send = register_handler(log_ws, WS_HANDLER_SEND, "");
			plugin_id_recv = register_handler(log_ws, WS_HANDLER_RECV, "");
			break;
		case DLL_PROCESS_DETACH:
			unregister_handler(plugin_id_send, WS_HANDLER_SEND);
			unregister_handler(plugin_id_recv, WS_HANDLER_RECV);
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


#if LOGGING == 1
void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags) //Note that you're given pointers to everything! (buf was already a pointer though)
{
	struct sockaddr_in info;
	int infolen;
	getpeername(*s,(struct sockaddr*)(&info),&infolen);
	const short port = ntohs(info.sin_port);

	//<SOCKET:4><ADDR:4><PORT:2><LEN:4><FLAGS:4><DATA:LEN>
	LOG(s,sizeof(SOCKET),1);
	LOG(&info.sin_addr,sizeof(struct in_addr),1);
	LOG(&port,sizeof(short),1);
	LOG(len,sizeof(int),1);
	LOG(flags,sizeof(int),1);
	LOG(buf,sizeof(char),*len); //If for whatever reason len != buffer size, then there's some bigger underlying problem... or another plugin is messing with something

	return;
}
#else
void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags)
{
	struct sockaddr_in info;
	int infolen;
	getpeername(*s,(struct sockaddr*)(&info),&infolen);
	const int port = ntohs(info.sin_port);
	
	LOG("%s:%u, Len %d, Flags %d, socket %u",inet_ntoa(info.sin_addr), port, *len, *flags, *s);
	LOGn("Data: ");  
	for(int i = 0; i < *len; i++) 
		LOGn("%02X ",(unsigned char)buf[i]);
	LOGn("\n");
	return;
}
#endif


static DWORD WINAPI setup_console(LPVOID param)
{
	AllocConsole();
	freopen("CONOUT$","w",stdout);
	freopen("CONIN$","r",stdin);
#if LOGGING == 1
	char *name = malloc(sizeof(char)*30);
	sprintf(name,"log_%u.bin",(unsigned int)time(NULL));
	logfile = fopen(name,"wb");
	free(name);
#endif
	while(1)
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
#if LOGGING == 1
	fclose(logfile);
#endif
	return 0;
}
