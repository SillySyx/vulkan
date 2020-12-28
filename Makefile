OUTPUT_PATH = ./dist
OUTPUT_FILE = $(OUTPUT_PATH)/test
HEADERS = -I libs/Vulkan-Headers/include -I headers
LIBS = -lvulkan
CFLAGS = -std=c++2a -g -Wall

default: build

clean:
	rm $(OUTPUT_PATH) -rf

build: clean
	mkdir $(OUTPUT_PATH)
	g++ $(CFLAGS) $(HEADERS) $(LIBS) -o $(OUTPUT_FILE) src/**/*.cpp src/*.cpp

run:
	$(OUTPUT_FILE)