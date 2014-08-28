#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "../../ws.h"
#include "../../list.h"

#define LOG(x,...) do { __mingw_printf(x, ##__VA_ARGS__); __mingw_printf("\n"); } while(0)
#define LOGn(x,...) __mingw_printf(x, ##__VA_ARGS__) //Log without newline

#define CHUNK 262144

struct buffer_list
{
	uint64_t time;
	uint32_t size;
	uint16_t msgc;
	uint8_t flag;
	uint8_t compressed;	
	uint8_t *data;	
	struct list_head buf;
};

#pragma pack(1)
struct Pkt_FFXIV_chat //0x00650014
{
        uint8_t unk2[20]; //20..39
        uint32_t id1; //40..43, user ID, constant between sessions/areas
        uint32_t unk3; //44..47, some constant
        uint32_t id2; //48..51, constant between sessions/areas
        uint8_t unk1; //65 needs this, but 67 doesn't... what
        char name[32];
        char message[1024];
};
struct Pkt_FFXIV_chat_2 //0x00670014
{
        uint8_t unk2[20]; //20..39
        uint32_t id1; //40..43, user ID, constant between sessions/areas
        uint32_t unk3; //44..47, some constant
        uint32_t id2; //48..51, constant between sessions/areas
        char name[32];
        char message[1024];
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
};
#pragma pack()


#endif //LOG_H
