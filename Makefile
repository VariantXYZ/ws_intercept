TARGET := ws
SOURCE := ws misc plugins

BUILD = build
PLUGINS = plugins
SHARED = shared

CC  := i686-w64-mingw32-g++

STD := c++98
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -DEXPORT -Wall -Wno-reorder
LDFLAGS := -lws2_32 -liphlpapi -lpsapi -static -shared -Wl,--gc-sections -Wl,--out-implib,shared/lib$(TARGET).a -s

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)

all: $(PLUGINS) $(SHARED)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o build/$(TARGET).dll
clean:
	rm -rf $(BUILD) $(SHARED)

$(PLUGINS): $(BUILD)
	mkdir $(BUILD)/$(PLUGINS)

$(BUILD):
	mkdir $(BUILD)

$(SHARED):
	mkdir $(SHARED)
