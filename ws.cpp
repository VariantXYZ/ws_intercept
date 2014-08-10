#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <psapi.h>
#include <stdio.h>

#include "ws.h"
#include "log.h"
#include "misc.h"

static char *buf_new;
static DWORD threadIDConsole = 0;
static DWORD threadID = 0;

typedef int (WINAPI *tWS)(SOCKET, const char*, int, int);

int (WINAPI *pRecv)(SOCKET s, const char* buf, int len, int flags);
int WINAPI repl_recv(SOCKET s, const char *buf, int len, int flags);

int (WINAPI *pSend)(SOCKET s, const char* buf, int len, int flags);
int WINAPI repl_send(SOCKET s, const char *buf, int len, int flags);

DWORD WINAPI initialize(LPVOID param);
DWORD WINAPI setup_console(LPVOID param);

extern "C" __declspec(dllexport) void register_handler(void)
{
	return;
}

extern "C" BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			char moduleName[MAX_PATH];
			GetModuleBaseName(GetCurrentProcess(), NULL, moduleName, MAX_PATH);
			if (strcmp(moduleName, "ffxiv.exe"))
			   break;
			Sleep(10000);
			buf_new = (char*)malloc(MAX_PACKET*sizeof(unsigned char)); //Pre-allocate a buffer
			CreateThread(NULL,0,setup_console,NULL,0,&threadIDConsole);
			CreateThread(NULL,0,initialize,NULL,0,&threadID);
			break;
		}
		case DLL_PROCESS_DETACH:
			free(buf_new);
			VirtualFree((LPVOID)pSend, 0, MEM_RELEASE);
			VirtualFree((LPVOID)pRecv, 0, MEM_RELEASE);
			if (threadIDConsole)
				PostThreadMessage(threadIDConsole, WM_QUIT, 0, 0);
#if LOGGING == 1
			fclose(logfile);
#endif
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

DWORD WINAPI initialize(LPVOID param)
{
	DWORD addr;
	DWORD orig_size;
	BYTE replaced[10];
	while(!setupFlag);
	LOG("Initialized!");

	LOG("Replacing WinSock send call");
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "send");
	LOG("WS2_32.dll:send @ %X",(unsigned int)addr);
	if(apply_patch(0xE9,addr,(void*)(&repl_send),&orig_size, replaced))
	{
		pSend = (tWS)VirtualAlloc(NULL, orig_size << 2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy((void*)pSend,replaced,orig_size);
		apply_patch(0xE9,(DWORD)pSend+orig_size,(void*)(addr+orig_size),&orig_size, replaced);
		VirtualProtect((LPVOID)pSend,orig_size+5,PAGE_EXECUTE_READWRITE,NULL); //DEP sucks :(
		LOG("Success! pSend Address = %X",(unsigned int)pSend);
	}
	   
	LOG("Replacing WinSock recv call");
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "recv");
	LOG("WS2_32.dll:recv @ %X",(unsigned int)addr);
	if(apply_patch(0xE9,addr,(void*)(&repl_recv),&orig_size, replaced))
	{
		pRecv = (tWS)VirtualAlloc(NULL, orig_size << 2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy((void*)pRecv,replaced,orig_size); 
		apply_patch(0xE9,(DWORD)pRecv+orig_size,(void*)(addr+orig_size),&orig_size, replaced); 
		VirtualProtect((LPVOID)pRecv,orig_size+5,PAGE_EXECUTE_READWRITE,NULL);
		LOG("Success! pRecv Address = %X",(unsigned int)pRecv);
	}

#if LOGGING != 2	 		
	while(1); //If we aren't logging, then everything has already been setup and we're good to go
#endif
	return 0;
}

int WINAPI repl_send(SOCKET s, const char *buf, int len, int flags)
{
	memcpy(buf_new,buf,len);
	
#if LOGGING != 2
	struct sockaddr_in info;
	int infolen;
	getpeername(s,(sockaddr*)(&info),&infolen);
	const int port = ntohs(info.sin_port);
	LOG("SEND");
	LOG("%s:%u, Len %d, Flags %d, socket %u",inet_ntoa(info.sin_addr),port,len,flags, s);
	LOGn("Data: ");  
	for(int i = 0; i < len; i++) 
		LOGn("%02X ",(unsigned char)buf_new[i]);
#endif

	return pSend(s,buf_new,len,flags);
}

int WINAPI repl_recv(SOCKET s, const char *buf, int len, int flags)
{
	memcpy(buf_new,buf,len);
	
#if LOGGING != 2
	struct sockaddr_in info;
	int infolen;
	getpeername(s,(sockaddr*)(&info),&infolen);
	const int port = ntohs(info.sin_port);
	LOG("RECV");
	LOG("%s:%u, Len %d, Flags %d, socket %u",inet_ntoa(info.sin_addr),port,len,flags, s);
	LOGn("Data: ");  
	for(int i = 0; i < len; i++) 
		LOGn("%02X ",(unsigned char)buf_new[i]);
#endif

	return pRecv(s,buf_new,len,flags);
}

DWORD WINAPI setup_console(LPVOID param)
{
#ifndef LOGGING
	AllocConsole();
	freopen("CONOUT$","w",stdout);
	freopen("CONIN$","r",stdin);
#endif
	setupFlag = true;
#ifndef LOGGING
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
#endif
	return 0;
}
