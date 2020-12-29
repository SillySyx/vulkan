OUTPUT_PATH = ../../dist
OUTPUT_FILE = $(OUTPUT_PATH)/test
HEADERS = -I ../../libs/Vulkan-Headers/include -I ../../core/headers
LIBS = -lvulkan -lxcb
CFLAGS = -std=c++2a -g -Wall

.PHONY: build

build:
	-@mkdir $(OUTPUT_PATH)
	g++ $(CFLAGS) $(HEADERS) $(LIBS) -o $(OUTPUT_FILE) ../../core/src/**/*.cpp src/*.cpp

run: build
	$(OUTPUT_FILE)