CC := g++
TARGET := server
SRC := server.cpp
CFLAGS := -g -Wall -Wextra -std=c++17

# Detect the operating system
ifeq ($(OS),Windows_NT)
	# Windows
	TARGET := $(TARGET).exe
	LDFLAGS := -lws2_32
	CLEANUP := del
	CLEANUP_OBJS := del *.o
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		# macOS
		TARGET := $(TARGET).out
		CLEANUP := rm -f
		CLEANUP_OBJS := rm -f *.o
	else ifeq ($(UNAME_S),Linux)
		# Linux
		TARGET := $(TARGET).out
		CLEANUP := rm -f
		CLEANUP_OBJS := rm -f *.o
	endif
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

.PHONY: clean

clean:
	$(CLEANUP) $(TARGET)
	$(CLEANUP_OBJS)



