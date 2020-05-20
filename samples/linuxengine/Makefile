VULKAN_SDK_PATH = /home/null/p/vulkan/libs
cFlags = -std=c++14 -I$(VULKAN_SDK_PATH)/headers -I../../vulkanengine -Isource
cOutput = -o build/app
includeFiles = ../../vulkanengine/*.cpp ../../vulkanengine/**/*.cpp source/*.cpp main.cpp
ldFlags = -L$(VULKAN_SDK_PATH)/x86_64/lib `pkg-config --static --libs glfw3` -lvulkan

build: clean
	mkdir build
	g++ $(cFlags) $(cOutput) $(includeFiles) $(ldFlags)

run: build
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/x86_64/lib ./build/app

clean:
	rm build -rf
