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
	unsigned char *data; //40+
};

struct Pkt_FFXIV_Chat
{
	uint32_t packet_id; //0..3
	uint64_t unk1; //something player/chat related 4..11
	uint32_t unk2; //something player/chat related 12..15
	uint64_t unk3; //Something specific to chat type 16..23
	uint64_t unk4; //Something specific to chat type 24..31
	uint32_t unk5; //Something session specific 32..35
	uint32_t unk6; //36..39
	uint64_t id1; //Something character specific 40..47
	uint32_t id2; //Something character specific 48..51
	uint8_t zero; //Just a random 0... 52
	char name[32]; //53+ is 32 byte name and message
	char message[1024]; 
};
#pragma pack()

struct Pkt_FFXIV packet;

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
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

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags) //Note that you're given pointers to everything! (buf was already a pointer though)
{
	if(!len)	
		return;
	memcpy(&packet, buf, sizeof(struct Pkt_FFXIV)-sizeof(unsigned char*));
	size_t to_read = (*len)-(sizeof(struct Pkt_FFXIV)-sizeof(unsigned char*));
	packet.data = malloc(to_read); //Try to hold the uncompressed size too
	memcpy(&packet.data,buf+sizeof(struct Pkt_FFXIV)-sizeof(unsigned char*), to_read);
	//Decompress stream
	if(packet.flag2)
	{
    		unsigned char *t_data = malloc(CHUNK);
		UncompressData(packet.data,to_read,t_data,CHUNK);
		free(packet.data);
		packet.data = t_data;
    	}
	
	if(*((unsigned int*)packet.data) == 0x00000458)
	{
		struct Pkt_FFXIV_Chat chat = *((struct Pkt_FFXIV_Chat*)packet.data);
		LOG("[%s]|[ID1: %llu, ID2:%u]: %s", chat.name, chat.id1, chat.id2, chat.message);
	}	
	free(packet.data);
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
