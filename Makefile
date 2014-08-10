TARGET := ws
SOURCE := ws misc plugins

CC  := i686-w64-mingw32-g++

STD := c++98
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -DEXPORT -Wall -Wno-reorder
LDFLAGS := -lws2_32 -liphlpapi -lpsapi -static -shared -Wl,--gc-sections -Wl,--out-implib,shared/lib$(TARGET).a -s

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)

all:
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o build/$(TARGET).dll
clean:
	rm -f build/$(TARGET).dll shared/lib$(TARGET).a
