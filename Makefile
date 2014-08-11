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

CC  := i686-w64-mingw32-g++

STD := c++98
CFLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -DEXPORT -Wall -Wno-reorder -shared
LDFLAGS := -lws2_32 -liphlpapi -lpsapi -static -shared -Wl,--gc-sections -Wl,--out-implib,shared/lib$(TARGET).a -s

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)
O_SOURCE := $(foreach FILE,$(SOURCES),$(SRC)/$(FILE))

OBJ := $(foreach FILE,$(SOURCE),$(FILE).o)
O_OBJS := $(foreach FILE,$(OBJ),$(BUILD)/$(FILE))

all: $(BUILD) $(SHARED) $(TARGET_OUT) $(PLUGINS)

ws: $(BUILD) $(SHARED) $(TARGET_OUT)

clean:
	rm -rf $(BUILD) $(SHARED) $(TARGET_OUT)
	make clean -C $(SRC)/$(PLUGINS)/*/
	rm -rf $(PLUGINS)

$(TARGET_OUT): $(O_OBJS)
	$(CC) -o $@ $(O_OBJS) $(LDFLAGS)

$(BUILD)/%.o: $(SRC)/%.cpp
	$(CC) -c $(CFLAGS) $(SRC)/$*.cpp -o $@

$(SRC)/%.cpp: $(SRC)/%.h

#Make directories if necessary
$(BUILD):
	mkdir $(BUILD)

$(PLUGINS):
	mkdir $(PLUGINS)
	$(MAKE) -C $(SRC)/$(PLUGINS)/*/

$(SHARED):
	mkdir $(SHARED)
