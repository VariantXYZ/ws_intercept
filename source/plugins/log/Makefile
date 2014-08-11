TARGET := log
SOURCE := log
LWS := -lws
BASE := ../../..

CC  := i686-w64-mingw32-g++

STD := c++98
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -Wall
LDFLAGS := -lws2_32 -static -shared -Wl,--gc-sections -s -L$(BASE)/shared/ $(LWS)

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)

all:
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $(BASE)/plugins/$(TARGET).dll
clean:
	rm -f $(BASE)/plugins/$(TARGET).dll
