VULKAN_SDK_PATH = /home/null/p/vulkan/libs
cFlags = -std=c++14 -I$(VULKAN_SDK_PATH)/headers -I../../vulkanengine -Isource
cOutput = -o build/app
includeFiles = ../../vulkanengine/*.cpp ../../vulkanengine/**/*.cpp main.cpp
ldFlags = -L$(VULKAN_SDK_PATH)/x86_64/lib `pkg-config --static --libs glfw3` -lvulkan

.PHONY: build

build: clean
	mkdir build
	g++ $(cFlags) $(cOutput) $(includeFiles) $(ldFlags)
	echo "LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/x86_64/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/x86_64/etc/explicit_layer.d ./app" > build/start.sh

shaders:
	mkdir build/shaders
	$(VULKAN_SDK_PATH)/x86_64/bin/glslangValidator -V shaders/shader.vert -o build/shaders/vert.spv
	$(VULKAN_SDK_PATH)/x86_64/bin/glslangValidator -V shaders/shader.frag -o build/shaders/frag.spv

run:
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/x86_64/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/x86_64/etc/explicit_layer.d ./build/app

clean:
	rm build -rf
