#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <psapi.h>

#include "ws.h"
#include "misc.h"

#define MAX_PACKET 4096

typedef int (WINAPI *tWS)(SOCKET, const char*, int, int); //For base functions

static int WINAPI repl_recv(SOCKET s, const char *buf, int len, int flags);
static int WINAPI repl_send(SOCKET s, const char *buf, int len, int flags);

//Trampolenes
static int (WINAPI *pRecv)(SOCKET s, const char* buf, int len, int flags); 
static int (WINAPI *pSend)(SOCKET s, const char* buf, int len, int flags);

static DWORD WINAPI initialize(LPVOID param);

LIBAPI void register_handler(DWORD packet_id, tWS_plugin func, WS_HANDLER_TYPE type)
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
			CreateThread(NULL,0,initialize,NULL,0,NULL);
			break;
		}
		case DLL_PROCESS_DETACH:
			VirtualFree((LPVOID)pSend, 0, MEM_RELEASE);
			VirtualFree((LPVOID)pRecv, 0, MEM_RELEASE);
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
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "send");
	
	if(apply_patch(0xE9,addr,(void*)(&repl_send),&orig_size, replaced))
	{
		pSend = (tWS)VirtualAlloc(NULL, orig_size << 2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy((void*)pSend,replaced,orig_size);
		apply_patch(0xE9,(DWORD)pSend+orig_size,(void*)(addr+orig_size),&orig_size, replaced);
		VirtualProtect((LPVOID)pSend,orig_size+5,PAGE_EXECUTE_READWRITE,NULL); //DEP sucks :(
	}
/*	   
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "recv");
	if(apply_patch(0xE9,addr,(void*)(&repl_recv),&orig_size, replaced))
	{
		pRecv = (tWS)VirtualAlloc(NULL, orig_size << 2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy((void*)pRecv,replaced,orig_size); 
		apply_patch(0xE9,(DWORD)pRecv+orig_size,(void*)(addr+orig_size),&orig_size, replaced); 
		VirtualProtect((LPVOID)pRecv,orig_size+5,PAGE_EXECUTE_READWRITE,NULL);
	}
*/

	return 0;
}

static int WINAPI repl_send(SOCKET s, const char *buf, int len, int flags)
{
	return pSend(s,buf,len,flags);
}

static int WINAPI repl_recv(SOCKET s, const char *buf, int len, int flags)
{
	return pRecv(s,buf,len,flags);
}