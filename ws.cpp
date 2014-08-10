#include <winsock2.h>
#include <windows.h>
#include <psapi.h>

#include "ws.h"
#include "misc.h"
#include "plugins.h"
#include "list.h"

#define MAX_PACKET 4096
//#define APPLICATION_NAME "ffxiv.exe"


typedef int (WINAPI *tWS)(SOCKET, const char*, int, int); //For base functions

static DWORD WINAPI initialize(LPVOID param);
static void revert();

static int WINAPI repl_recv(SOCKET s, const char *buf, int len, int flags);
static int WINAPI repl_send(SOCKET s, const char *buf, int len, int flags);

//Trampolenes
static int (WINAPI *pRecv)(SOCKET s, const char* buf, int len, int flags) = NULL; 
static int (WINAPI *pSend)(SOCKET s, const char* buf, int len, int flags) = NULL;

//Keep track to undo change before closing
static BYTE replaced_send[10];
static BYTE replaced_recv[10];
static DWORD orig_size_send = 0;
static DWORD orig_size_recv = 0;

LIBAPI DWORD register_handler(tWS_plugin func, WS_HANDLER_TYPE type, char *comment)
{
	struct WS_handler t;
	t.func = func;
	t.comment = (char*)malloc(sizeof(char)*strlen(comment));
	strcpy(t.comment,comment);
	if(type & WS_HANDLER_SEND)
		ws_handlers_send.push_back(t);
	else
		ws_handlers_recv.push_back(t);
	return (DWORD)(type & WS_HANDLER_SEND?ws_handlers_send.end():ws_handlers_recv.end()); //Returns pointer to node we just added
}

LIBAPI void unregister_handler(DWORD plugin_id, WS_HANDLER_TYPE type)
{
	if(type & WS_HANDLER_SEND)
		ws_handlers_send.del((list_node<WS_handler>*)plugin_id);
	else
		ws_handlers_recv.del((list_node<WS_handler>*)plugin_id);
	return;
}

extern "C" BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
		{
#ifdef APPLICATION_NAME		
		 	char moduleName[MAX_PATH];
			GetModuleBaseName(GetCurrentProcess(), NULL, moduleName, MAX_PATH);
			if (strcmp(moduleName, APPLICATION_NAME))
				return FALSE;
#endif				
			CreateThread(NULL,0,initialize,NULL,0,NULL);
			break;
		}
		case DLL_PROCESS_DETACH:
			revert();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

static DWORD WINAPI initialize(LPVOID param)
{
	DWORD addr;
	
	//TODO: Clean this area up and move these to a function
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "send");
	if(apply_patch(0xE9,addr,(void*)(&repl_send),&orig_size_send, replaced_send))
	{
		pSend = (tWS)VirtualAlloc(NULL, orig_size_send << 2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy((void*)pSend,replaced_send,orig_size_send);
		apply_patch(0xE9,(DWORD)pSend+orig_size_send,(void*)(addr+orig_size_send),&orig_size_send, replaced_send);
		VirtualProtect((LPVOID)pSend,orig_size_send+5,PAGE_EXECUTE_READWRITE,NULL); //DEP sucks :(
	}

	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "recv");
	if(apply_patch(0xE9,addr,(void*)(&repl_recv),&orig_size_recv, replaced_recv))
	{
		pRecv = (tWS)VirtualAlloc(NULL, orig_size_recv << 2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy((void*)pRecv,replaced_recv,orig_size_recv); 
		apply_patch(0xE9,(DWORD)pRecv+orig_size_recv,(void*)(addr+orig_size_recv),&orig_size_recv, replaced_recv); 
		VirtualProtect((LPVOID)pRecv,orig_size_recv+5,PAGE_EXECUTE_READWRITE,NULL);
	}

	load_plugins("./plugins/", ws_plugins);
	return 0;
}

static void revert() //TODO: Figure out why things seem to crash on closing on FFXIV
{
	DWORD addr;	
	if(!orig_size_send && !orig_size_recv)
		return;
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "send");
	memcpy((void*)addr,replaced_send,orig_size_send);
	addr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("WS2_32.dll")), "recv");
	memcpy((void*)addr,replaced_recv,orig_size_recv);
	return;
}

static int WINAPI repl_send(SOCKET s, const char *buf, int len, int flags)
{
	
	for(list_node<WS_handler>* t = ws_handlers_send.begin(); t->next != NULL; t = t->next)
		t->data.func(s,buf,len,flags);
		
	return pSend(s,buf,len,flags);
}

static int WINAPI repl_recv(SOCKET s, const char *buf, int len, int flags)
{
	for(list_node<WS_handler>* t = ws_handlers_recv.begin(); t->next != NULL; t = t->next)
		t->data.func(s,buf,len,flags);

	return pRecv(s,buf,len,flags);
}