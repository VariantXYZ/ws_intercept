TARGET := ffxiv_pkt_log
SOURCE := log
LWS := -lws
BASE := ../../..

CC  := i686-w64-mingw32-gcc

STD := c99
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -Wall
LDFLAGS := -lws2_32 -static -shared -Wl,--gc-sections -s -L$(BASE)/shared/ $(LWS) -lz

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).c)

all: $(BASE)/plugins/$(TARGET).dll

$(BASE)/plugins/$(TARGET).dll: $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $(BASE)/plugins/$(TARGET).dll
clean:
	rm -f $(BASE)/plugins/$(TARGET).dll
