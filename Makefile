TARGET := winsock_intercept
SOURCE := ws misc

GCC  := i686-w64-mingw32-g++

STD := c++98
FLAGS := -std=$(STD) -O3 -fdata-sections -ffunction-sections -flto -DEXPORT -Wall
LFLAGS := -lws2_32 -liphlpapi -lpsapi -static-libgcc -static-libstdc++ -static -shared -Wl,--gc-sections -s

SOURCES := $(foreach FILE,$(SOURCE),$(FILE).cpp)

all:
	$(GCC) $(SOURCES) $(FLAGS) $(LFLAGS) -o build/$(TARGET).dll
clean:
	rm -f build/*.dll