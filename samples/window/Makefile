OUTPUT_PATH = ../../dist
OUTPUT_FILE = $(OUTPUT_PATH)/window
HEADERS = -I headers
LIBS = -lxcb
CFLAGS = -std=c++2a -g -Wall

.PHONY: build

build:
	-@mkdir $(OUTPUT_PATH)
	g++ $(CFLAGS) $(HEADERS) $(LIBS) -o $(OUTPUT_FILE) src/**/*.cpp src/main.cpp

run: build
	$(OUTPUT_FILE)