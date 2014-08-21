#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <zlib.h>
#include "log.h"

static DWORD WINAPI setup_console(LPVOID param);

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags);
int UncompressData( const unsigned char* abSrc, int nLenSrc, unsigned char* abDst, int nLenDst );

static DWORD threadIDConsole = 0;
static DWORD plugin_id_recv = 0;

#define CHUNK 262144

#pragma pack(1)
struct Pkt_FFXIV_chat //0x00650014
{
        uint8_t unk2[20]; //20..39
        uint32_t id1; //40..43, user ID, constant between sessions/areas
        uint32_t unk3; //44..47, some constant
        uint32_t id2; //48..51, constant between sessions/areas
        uint8_t unk1; //65 needs this, but 67 doesn't... what
        unsigned char name[32];
        unsigned char message[1024];
};
struct Pkt_FFXIV_chat_2 //0x00670014
{
        uint8_t unk2[20]; //20..39
        uint32_t id1; //40..43, user ID, constant between sessions/areas
        uint32_t unk3; //44..47, some constant
        uint32_t id2; //48..51, constant between sessions/areas
        unsigned char name[32];
        unsigned char message[1024];
};
struct Pkt_FFXIV_msg
{
        uint32_t msg_size; //0..3, including size
        uint64_t entity_id; //4..11, variable (changes with area/session, high 4 bits seem constant)
        uint32_t unk1; //12..15
        uint32_t msg_type; //16..19
};
struct Pkt_FFXIV
{
        uint8_t unk1[16]; //magic#? Bytes 0-15
        uint64_t timestamp; //unix timestamp, big endian 16-23
        uint32_t size; //Total payload size (including bytes 0-27), big endian, bytes 24-27
        uint8_t unk2[2]; //28-29
        uint16_t message_count; //Number of messages in payload, big endian, bytes 30-31
        uint8_t flag1; //Unknown, byte 32
        uint8_t flag2; //Compression, byte 33
        uint8_t unk3[6]; //unknown, 34-39
	unsigned char *data;
};
#pragma pack()

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			plugin_id_recv = register_handler(log_ws, WS_HANDLER_RECV, "A logging function for ws2_recv");
			CreateThread(NULL,0,setup_console,NULL,0,&threadIDConsole);
			break;
		case DLL_PROCESS_DETACH:
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

inline void handle_chat(unsigned char *buf)
{

	struct Pkt_FFXIV_chat chat = *(struct Pkt_FFXIV_chat*)(buf);
    	LOG("[%s][%d %d]: %s", chat.name, chat.id1, chat.id2, chat.message);
}

inline void handle_chat_2(unsigned char *buf)
{

	struct Pkt_FFXIV_chat_2 chat = *(struct Pkt_FFXIV_chat_2*)(buf);
    	LOG("[%s][%d %d]: %s", chat.name, chat.id1, chat.id2, chat.message);
}

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags)
{
	if(!*len || *len < (sizeof(struct Pkt_FFXIV)-sizeof(unsigned char*)) )
		return;

	struct Pkt_FFXIV packet;

	uint32_t pos = (uint32_t)buf;
	
	memcpy(&packet,(void*)pos,sizeof(struct Pkt_FFXIV)-sizeof(unsigned char*)); //Packet header info
	pos += sizeof(struct Pkt_FFXIV)-sizeof(unsigned char*);	

	if(packet.size < 19)
		return;

	size_t to_read = *len - (sizeof(struct Pkt_FFXIV) - sizeof(unsigned char*));
	packet.data = malloc(to_read);
	
	memcpy(packet.data, (void*)pos, to_read);
	pos += to_read;
		
	//Decompress stream	
	if(packet.flag2)
	{
		unsigned char *t_data = malloc(CHUNK);
		UncompressData(packet.data,to_read,t_data,CHUNK);
		free(packet.data);
		packet.data = t_data;
	}

	struct Pkt_FFXIV_msg *msg = malloc(sizeof(struct Pkt_FFXIV_msg));
	memcpy(msg, packet.data, sizeof(struct Pkt_FFXIV_msg));
	if(!msg->msg_size)
		return;

	pos = (uint32_t)(packet.data + sizeof(struct Pkt_FFXIV_msg));

	switch(msg->msg_type)
	{
		case 0x00650014: handle_chat((unsigned char*)pos); break;
		case 0x00670014: handle_chat_2((unsigned char*)pos); break;
		default: break;
	}
	free(msg);
	return;
}

static DWORD WINAPI setup_console(LPVOID param)
{
	AllocConsole();
	freopen("CONOUT$","w",stdout);
	freopen("CONIN$","r",stdin);
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
