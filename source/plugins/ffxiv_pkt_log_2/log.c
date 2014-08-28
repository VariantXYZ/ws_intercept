#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <zlib.h>
#include "log.h"

static DWORD WINAPI setup(LPVOID param);
static DWORD WINAPI handle_buf(LPVOID param);

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags);
int UncompressData( const unsigned char* abSrc, int nLenSrc, unsigned char* abDst, int nLenDst );

static DWORD threadIDConsole = 0;
static DWORD threadIDBuf = 0;
static DWORD plugin_id_recv = 0;
static struct buffer_list buffer;

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			plugin_id_recv = register_handler(log_ws, WS_HANDLER_RECV, "A logging function for ws2_recv");
			CreateThread(NULL,0,setup,NULL,0,&threadIDConsole);
			CreateThread(NULL,0,handle_buf,NULL,0,&threadIDBuf);
			break;
		case DLL_PROCESS_DETACH:
			unregister_handler(plugin_id_recv, WS_HANDLER_RECV);
			if (threadIDConsole)
				PostThreadMessage(threadIDConsole, WM_QUIT, 0, 0);
			if (threadIDBuf)
				PostThreadMessage(threadIDBuf, WM_QUIT, 0, 0);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

inline void handle_chat(uint8_t *buf, size_t size)
{

        struct Pkt_FFXIV_chat *chat = malloc(sizeof(struct Pkt_FFXIV_chat));
        memcpy(chat, buf, size);
        LOGn("Message Size: %d ", size);
        LOG("[%s][%d %d]: %s", chat->name, chat->id1, chat->id2, chat->message);
        free(chat);
}

inline void handle_chat_2(uint8_t *buf, size_t size)
{

        struct Pkt_FFXIV_chat_2 *chat = malloc(sizeof(struct Pkt_FFXIV_chat_2));
        memcpy(chat, buf, size);
        LOGn("Message Size: %d ", size);
        LOG("[%s][%d %d]: %s", chat->name, chat->id1, chat->id2, chat->message);
        free(chat);
}

static DWORD WINAPI handle_buf(LPVOID PARAM) //Serialize packet parsing, TODO: safety for adding a new entry while working through
{
	while(1)
	{
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
                case WM_QUIT:
                    return msg.wParam;
            }
        }

		if(list_empty(&(buffer.buf)))
			continue;
		list_for_each_safe(t, s, &buffer.buf)
		{
			struct buffer_list *tmp = list_entry(t, struct buffer_list, buf);
			LOGn("[%llu]",tmp->time);
		
			//Decompress stream	
			if(tmp->compressed)
			{
				uint8_t *t_data = malloc(CHUNK);
				UncompressData(tmp->data,tmp->size,t_data,CHUNK);
				free(tmp->data);
				tmp->data = t_data;
			}
	
			uint8_t *pos = tmp->data;
			for(uint32_t i = 0; i < tmp->msgc; i++)
			{
				struct Pkt_FFXIV_msg msg = *(struct Pkt_FFXIV_msg*)(pos);
				pos += sizeof(struct Pkt_FFXIV_msg);
				LOGn("[%u]",msg.msg_type);						
				switch(msg.msg_type)
				{
					case 0x00650014: handle_chat(pos, msg.msg_size); break;
					case 0x00670014: handle_chat_2(pos, msg.msg_size); break;
					default: LOGn("\n"); break;
				}
				pos += msg.msg_size;
			}

			free(tmp->data);
			list_del(t);
			free(tmp);
		}
	}
}

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags)
{
	if(*len < sizeof(struct Pkt_FFXIV))
		return;

	struct Pkt_FFXIV p = *(struct Pkt_FFXIV*)(buf);
	struct buffer_list *t = malloc(sizeof(struct buffer_list));

	if(p.size != *len)
	{
		LOG("p.size != len (%u != %u)", p.size, *len); //I'm sure I'll have to handle this case properly eventually
		return;
	}

	t->time = p.timestamp;
	t->size = p.size;
	t->msgc = p.message_count;
	t->flag = p.flag1;
	t->compressed = p.flag2;
	t->data	= malloc(p.size); //Only the message data is in here
	
	memcpy(t->data,buf+sizeof(struct Pkt_FFXIV),p.size); //Let the thread handle it
	list_add_tail(&(t->buf),&(buffer.buf));
	return;
}

static DWORD WINAPI setup(LPVOID param)
{
	AllocConsole();
	freopen("CONOUT$","w",stdout);
	freopen("CONIN$","r",stdin);
	INIT_LIST_HEAD(&buffer.buf);
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
	return 0;
}


int UncompressData( const unsigned char* abSrc, int nLenSrc, unsigned char* abDst, int nLenDst )
{
    z_stream zInfo ={0};
    zInfo.total_in=  zInfo.avail_in=  nLenSrc;
    zInfo.avail_out= nLenDst;
    zInfo.next_in= (unsigned char*)abSrc;
    zInfo.next_out= abDst;

    int nErr, nRet= -1;
    nErr= inflateInit( &zInfo );               // zlib function
    if ( nErr == Z_OK ) {
        nErr= inflate( &zInfo, Z_FINISH );     // zlib function
        if ( nErr == Z_STREAM_END ) {
            nRet= zInfo.total_out;
        }
    }
    inflateEnd( &zInfo );   // zlib function
    return( nRet ); // -1 or len of output
}
