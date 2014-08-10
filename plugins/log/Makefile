TARGET := log
SOURCE := log

CC  := i686-w64-mingw32-g++

STD := c++98
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -Wall
LDFLAGS := -lws2_32 -static -shared -Wl,--gc-sections -s -L../../shared/ -lws

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)

all:
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o ../../build/plugins/$(TARGET).dll
clean:
	rm -f ../../build/$(TARGET).dll