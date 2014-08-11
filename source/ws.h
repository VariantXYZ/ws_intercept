#ifndef WS_SEND_H
#define WS_SEND_H

#include <windows.h>
#include "list.h"

#ifdef EXPORT
#define LIBAPI __declspec(dllexport)
#else
#define LIBAPI __declspec(dllimport)
#endif

typedef void (WINAPI *tWS_plugin)(SOCKET&, const char*, int&, int&); //For plugin hooks, returns plugin ID

enum WS_HANDLER_TYPE
{
	WS_HANDLER_SEND = 0x1,
	WS_HANDLER_RECV = 0x2
};

struct WS_handler
{
	tWS_plugin func;
	char *comment;
};

extern "C" LIBAPI DWORD register_handler(tWS_plugin func, WS_HANDLER_TYPE type, char *comment = (char*)""); //0x0 affects all packets
extern "C" LIBAPI void unregister_handler(DWORD plugin_id, WS_HANDLER_TYPE type);

LIBAPI list<HMODULE> ws_plugins; //Why is the list of DLLs exposed? So someone can implement a plugin to reload them, of course!
LIBAPI list<WS_handler> ws_handlers_send; //Contains ordered list of function handlers for send
LIBAPI list<WS_handler> ws_handlers_recv; //Contains ordered list of function handlers for recv


#endif //WS_SEND_H
