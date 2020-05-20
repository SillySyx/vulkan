#VULKAN_SDK_PATH = /home/user/VulkanSDK/x.x.x.x/x86_64
#CFLAGS = -std=c++11 -I$(VULKAN_SDK_PATH)/include
#LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
#LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d ./VulkanTest
#g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

#cflags = -Wall -g -std=c++14
#g++ -Wall -std=c++14 -I$(VULKAN_SDK_PATH)/headers -I$(sourcePath) -o bin/hello $(sourcePath)*.cpp main.cpp -L$(VULKAN_SDK_PATH)/x86_64/lib -lvulkan

VULKAN_SDK_PATH = /home/null/p/vulkan/libs
cFlags = -Wall -std=c++14 -I$(VULKAN_SDK_PATH)/headers -Isource
cOutput = -o build/hello
ldFlags = -L$(VULKAN_SDK_PATH)/x86_64/lib `pkg-config --static --libs glfw3` -lvulkan

build: clean
	mkdir build
	g++ $(cFlags) $(cOutput) source/*.cpp main.cpp $(ldFlags)

clean:
	rm -rf build

run: build
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/x86_64/lib ./build/hello
