TARGET := winsock_intercept
TARGET_CONSOLE := $(TARGET)_console
TARGET_TEXT := $(TARGET)_text

SOURCE := ws misc
LOG := log.h

GCC  := i686-w64-mingw32-g++

STD := c++98
FLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -Wall
LFLAGS := -lws2_32 -liphlpapi -lpsapi -static-libgcc -static-libstdc++ -static -shared -Wl,--gc-sections -s

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)

#Define constants
NOLOG := -DLOGGING=2
LOGGING := -DLOGGING=1

all:
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) $(NOLOG) -o $(TARGET).dll
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) -o $(TARGET_CONSOLE).dll
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) $(LOGGING) -o $(TARGET_TEXT).dll
nolog: 
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) $(NOLOG) -o $(TARGET).dll
consolelog:
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) -o $(TARGET_CONSOLE).dll
textlog: 
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) $(LOGGING) -o $(TARGET_TEXT).dll
clean:
	rm -f $(TARGET).dll $(TARGET_CONSOLE).dll $(TARGET_TEXT).dll
