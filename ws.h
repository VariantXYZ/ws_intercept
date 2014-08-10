#ifndef WS_SEND_H
#define WS_SEND_H

#include <windows.h>

#ifdef EXPORT

#define LIBAPI extern "C" __declspec(dllexport)

#else

#define LIBAPI __declspec(dllimport)

#endif

enum WS_HANDLER_TYPE //Bitmask
{
	WS_HANDLER_SEND = 0x1,
	WS_HANDLER_RECV = 0x2,
	WS_HANDLER_BOTH = 0x3
};


typedef void (WINAPI *tWS_plugin)(SOCKET&, const char*, int&, int&); //For plugin hooks
LIBAPI void register_handler(DWORD packet_id, tWS_plugin func, WS_HANDLER_TYPE type);

#endif //WS_SEND_H
