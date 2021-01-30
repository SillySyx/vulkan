OUTPUT_PATH = ../../dist
OUTPUT_FILE = $(OUTPUT_PATH)/device
HEADERS = -I headers -I ../../core/headers -I ../../libs/Vulkan-Headers/include
LIBS = -lxcb -lX11 -lvulkan
CFLAGS = -std=c++2a -g -Wall

.PHONY: build

build:
	-@mkdir $(OUTPUT_PATH)
	g++ $(CFLAGS) $(HEADERS) $(LIBS) -o $(OUTPUT_FILE) ../../core/src/**/*.cpp src/main.cpp

run: build
	$(OUTPUT_FILE)