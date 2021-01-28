OUTPUT_PATH = ../../dist
OUTPUT_FILE = $(OUTPUT_PATH)/window
HEADERS = -I headers -I../../core/headers
LIBS = -lxcb -lX11
CFLAGS = -std=c++2a -g -Wall

.PHONY: build

build:
	-@mkdir $(OUTPUT_PATH)
	g++ $(CFLAGS) $(HEADERS) $(LIBS) -o $(OUTPUT_FILE) ../../core/src/**/*.cpp src/main.cpp

run: build
	$(OUTPUT_FILE)