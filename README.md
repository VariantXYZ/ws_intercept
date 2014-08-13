http://xkcd.com/1296/

#Winsock 2 Interception Library

##What is it?

A simple hook into ws2_32.dll's send and receive functions that supports plugins to process/modify packets. 

Its major purpose is to be an easy to use go-to for packet logging and modification for any application that uses winsock that can be hooked into.

####Why should I use it?

If you've got an application (game, browser, or some personal application) whose packets you want to monitor but don't need all the provided functionality of a heavyweight tool like Microsoft's Network Monitor or Wireshark, or if you've got an application you want to modify the packets for (e.g, implementing your own encryption algorithm). 

####How does it work?

As soon as the DLL hooks in and modifies the call to ws2_send and ws2_receive, it runs a LoadLibrary on everything it can find in the ./plugins/ folder. 

####But I could accomplish this with a simple proxy DLL, what makes yours so special?

Mines isn't particularly special. Outside of supporting plugins, it's a rather simple implementation of a hook. 

The reason to prefer this over a proxy is in the case that in certain systems or applications, you'll be unable to load anything except the provided system library, which makes it impossible to get through with just a simple proxy/no executable memory modifications. 

Another alternative would be to write a simple proxy server and handle things that way, and it's a valid approach too, but you've gotta worry about things like what sockets to monitor and how to get that information in the first place.

####Alright, I guess this could serve my purposes well, how do I set it up?

Well first you need to clone it, after that...

To build it using the provided Makefile, you'll need [mingw-w64](http://mingw-w64.sourceforge.net/), or just install it on some linux distribution (it's found in most of them, if not in a core repository, then at least in the user repositories).

####Ok, it looks like I've got a ws.dll, a folder called shared with a ws.a in there, and a folder called plugins with log.dll in there.

Seems like it worked then. Here's what the files are:

ws.dll - The main thing you want to load, this is what handles modifying the executable memory and loading all the things in the plugin folder.

shared/ws.a - The static library you'll want to use to link with your plugins (see source/plugins/log/* for an example of a plugin implementation).

plugins/log.dll - A plugin that logs both sent and received packets in a console window

Note that the rest of the files in build are just the unlinked object files, you can ignore them.

####How do I load the DLL?

A number of ways. You could use [Lord PE](http://www.woodmann.com/collaborative/tools/index.php/LordPE) to modify your client's PE header and add the DLL as an import.

You could also modify the registry key for AppInit_DLLs to load it:

HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Windows for 64-bit applications on 64-bit windows (32-bit on 32-bit windows)

HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows\AppInit_DLLs for 32-bit applications on 64-bit windows

####How do I write my own plugins?

Here's the fun part! Go ahead and look in source/plugins/log/ for an example implementation. The Makefile at the top level of the project builds all plugins too, so you can go ahead and take advantage of that if you want.

You'll need to link against ws.a and include ws.h (probably also include winsock2.h and windows.h) in your project (C or C++ should work fine). Here are the relevant functions you'll need to use:

######LIBAPI DWORD register_handler(tWS_plugin func, WS_HANDLER_TYPE type, char *comment)

A function that'll register your function to be called when a packet is sent or received (based on WS_HANDLER_TYPE).

tWS_plugin func - A function of the type (SOCKET* s, const char* buf, int* len, int* flags). These arguments are almost identical to the winsock calls themselves, except they pass pointers (well, buf already was a pointer). This allows you to modify them if necessary. See log for implementation examples

WS_HANDLER_TYPE type - Two options, WS_HANDLER_SEND and WS_HANDLER_RECV, pretty straightforward. You'll need to call this function twice to register for both

char* comment - Currently unnecessary, you could just pass a "", but this at least allows someone to make a plugin to monitor other plugins and see some relevant information about it

RETURNS: A plugin identifier, save this somewhere because you'll need it to unload your handler.

######LIBAPI void unregister_handler(DWORD plugin_id, WS_HANDLER_TYPE type);

If for whatever reason your DLL is being unloaded, this'll remove your function handler. Just pass the plugin_id from register_handler and the handler type (send or receive)

Also available to you are:

LIBVAR struct WS_plugins ws_plugins

LIBVAR struct WS_handler ws_handlers

Which are lists of the plugins and handlers loaded. Check out ws.h and list.h for how to use these. 
