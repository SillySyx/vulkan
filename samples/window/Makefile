OUTPUT_PATH = ../../dist
OUTPUT_FILE = $(OUTPUT_PATH)/window
LIBS = -lxcb
CFLAGS = -std=c++2a -g -Wall

.PHONY: build

build:
	-@mkdir $(OUTPUT_PATH)
	g++ $(CFLAGS) $(LIBS) -o $(OUTPUT_FILE) src/*.cpp

run: build
	$(OUTPUT_FILE)