#ifndef WS_SEND_H
#define WS_SEND_H

#include <windows.h>
#include "list.h"

#ifdef __cplusplus
#define EXT extern "C"
#else
#define EXT
#endif

#ifdef EXPORT
#define LIBVAR __declspec(dllexport)
#define LIBAPI EXT LIBVAR
#else
#define LIBVAR __declspec(dllimport)
#define LIBAPI EXT LIBVAR
#endif

typedef void (WINAPI *tWS_plugin)(SOCKET*, const char*, int*, int*); //For plugin hooks, passes a pointer to all the relevant data EXCEPT buf because that's already a pointer; Pointers can be rather scary.

typedef enum
{
	WS_HANDLER_SEND = 0x1,
	WS_HANDLER_RECV = 0x2
} WS_HANDLER_TYPE;

//I swear the linux linked list implementation is demon magic
struct WS_handler
{
	tWS_plugin func;
	char *comment;
	struct list_head ws_handlers_send; //Contains ordered list of function handlers for send
	struct list_head ws_handlers_recv; //Contains ordered list of function handlers for recv

};

struct WS_plugins
{
    HMODULE plugin;
    struct list_head plugins;
};

LIBAPI DWORD register_handler(tWS_plugin func, WS_HANDLER_TYPE type, char *comment);
LIBAPI void unregister_handler(DWORD plugin_id, WS_HANDLER_TYPE type);

LIBVAR struct WS_plugins ws_plugins; //Why is the list of DLLs exposed? So someone can implement a plugin to reload them, of course!
LIBVAR struct WS_handler ws_handlers;

#endif //WS_SEND_H
