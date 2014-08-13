TARGET := ws
TARGET_TYPE := dll
PLUGIN_TYPE := dll
SOURCE := ws misc plugins

TARGET_OUT := $(TARGET).$(TARGET_TYPE)

BASE := .
BUILD := $(BASE)/build
PLUGINS := $(BASE)/plugins
SHARED := $(BASE)/shared
SRC := $(BASE)/source

CC  := i686-w64-mingw32-gcc

STD := c99
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -DEXPORT -Wall -shared
LDFLAGS := -lws2_32 -liphlpapi -lpsapi -static -shared -Wl,--gc-sections -Wl,--out-implib,shared/lib$(TARGET).a -s

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).c)
O_SOURCE := $(foreach FILE,$(SOURCES),$(SRC)/$(FILE))

OBJ := $(foreach FILE,$(SOURCE),$(FILE).o)
O_OBJS := $(foreach FILE,$(OBJ),$(BUILD)/$(FILE))

#Confusing, I know, this is to build every plugin subdirectory
PLUGINSRC := $(wildcard $(SRC)/$(PLUGINS)/*/.)

.PHONY: $(PLUGINSRC) #Gotta run this regardless of timestamp on the folder, let the plugin Makefile handle things 
all: $(BUILD) $(SHARED) $(TARGET_OUT) $(PLUGINS) $(PLUGINSRC)

ws: $(BUILD) $(SHARED) $(TARGET_OUT)

clean:
	rm -rf $(BUILD) $(SHARED) $(TARGET_OUT)
	rm -rf $(PLUGINS)

$(TARGET_OUT): $(O_OBJS)
	$(CC) -o $@ $(O_OBJS) $(LDFLAGS)

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $(SRC)/$*.c -o $@

$(SRC)/%.c: $(SRC)/%.h

#Plugins
$(PLUGINSRC):
	$(MAKE) -C $@

#Make directories if necessary
$(BUILD):
	mkdir $(BUILD)

$(PLUGINS):
	mkdir $(PLUGINS)

$(SHARED):
	mkdir $(SHARED)
