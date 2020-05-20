#include "VulkanWrapper.h"

void InitVulkanLibrary()
{
	LibraryHandle handle;
#if defined(_WIN32)
	handle = LoadLibrary("vulkan-1.dll");
#elif defined(__linux__) || defined(__ANDROID__)
	handle = dlopen("libvulkan.so", RTLD_NOW);
#endif

	// Vulkan supported, set function addresses
	vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(LoadProcAddress(handle, "vkCreateInstance"));
	vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(LoadProcAddress(handle, "vkDestroyInstance"));
	vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(LoadProcAddress(handle, "vkEnumeratePhysicalDevices"));
	vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(LoadProcAddress(handle, "vkGetPhysicalDeviceFeatures"));
	vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(LoadProcAddress(handle, "vkGetPhysicalDeviceFormatProperties"));
	vkGetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(LoadProcAddress(handle, "vkGetPhysicalDeviceImageFormatProperties"));
	vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(LoadProcAddress(handle, "vkGetPhysicalDeviceProperties"));
	vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(LoadProcAddress(handle, "vkGetPhysicalDeviceQueueFamilyProperties"));
	vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(LoadProcAddress(handle, "vkGetPhysicalDeviceMemoryProperties"));
	vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(LoadProcAddress(handle, "vkGetInstanceProcAddr"));
	vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(LoadProcAddress(handle, "vkGetDeviceProcAddr"));
	vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(LoadProcAddress(handle, "vkCreateDevice"));
	vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(LoadProcAddress(handle, "vkDestroyDevice"));
	vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(LoadProcAddress(handle, "vkEnumerateInstanceExtensionProperties"));
	vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(LoadProcAddress(handle, "vkEnumerateDeviceExtensionProperties"));
	vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(LoadProcAddress(handle, "vkEnumerateInstanceLayerProperties"));
	vkEnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(LoadProcAddress(handle, "vkEnumerateDeviceLayerProperties"));
	vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(LoadProcAddress(handle, "vkGetDeviceQueue"));
	vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(LoadProcAddress(handle, "vkQueueSubmit"));
	vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(LoadProcAddress(handle, "vkQueueWaitIdle"));
	vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(LoadProcAddress(handle, "vkDeviceWaitIdle"));
	vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(LoadProcAddress(handle, "vkAllocateMemory"));
	vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(LoadProcAddress(handle, "vkFreeMemory"));
	vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(LoadProcAddress(handle, "vkMapMemory"));
	vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(LoadProcAddress(handle, "vkUnmapMemory"));
	vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(LoadProcAddress(handle, "vkFlushMappedMemoryRanges"));
	vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(LoadProcAddress(handle, "vkInvalidateMappedMemoryRanges"));
	vkGetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(LoadProcAddress(handle, "vkGetDeviceMemoryCommitment"));
	vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(LoadProcAddress(handle, "vkBindBufferMemory"));
	vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(LoadProcAddress(handle, "vkBindImageMemory"));
	vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(LoadProcAddress(handle, "vkGetBufferMemoryRequirements"));
	vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(LoadProcAddress(handle, "vkGetImageMemoryRequirements"));
	vkGetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(LoadProcAddress(handle, "vkGetImageSparseMemoryRequirements"));
	vkGetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(LoadProcAddress(handle, "vkGetPhysicalDeviceSparseImageFormatProperties"));
	vkQueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(LoadProcAddress(handle, "vkQueueBindSparse"));
	vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(LoadProcAddress(handle, "vkCreateFence"));
	vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(LoadProcAddress(handle, "vkDestroyFence"));
	vkResetFences = reinterpret_cast<PFN_vkResetFences>(LoadProcAddress(handle, "vkResetFences"));
	vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(LoadProcAddress(handle, "vkGetFenceStatus"));
	vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(LoadProcAddress(handle, "vkWaitForFences"));
	vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(LoadProcAddress(handle, "vkCreateSemaphore"));
	vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(LoadProcAddress(handle, "vkDestroySemaphore"));
	vkCreateEvent = reinterpret_cast<PFN_vkCreateEvent>(LoadProcAddress(handle, "vkCreateEvent"));
	vkDestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(LoadProcAddress(handle, "vkDestroyEvent"));
	vkGetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(LoadProcAddress(handle, "vkGetEventStatus"));
	vkSetEvent = reinterpret_cast<PFN_vkSetEvent>(LoadProcAddress(handle, "vkSetEvent"));
	vkResetEvent = reinterpret_cast<PFN_vkResetEvent>(LoadProcAddress(handle, "vkResetEvent"));
	vkCreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(LoadProcAddress(handle, "vkCreateQueryPool"));
	vkDestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(LoadProcAddress(handle, "vkDestroyQueryPool"));
	vkGetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(LoadProcAddress(handle, "vkGetQueryPoolResults"));
	vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(LoadProcAddress(handle, "vkCreateBuffer"));
	vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(LoadProcAddress(handle, "vkDestroyBuffer"));
	vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(LoadProcAddress(handle, "vkCreateBufferView"));
	vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(LoadProcAddress(handle, "vkDestroyBufferView"));
	vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(LoadProcAddress(handle, "vkCreateImage"));
	vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(LoadProcAddress(handle, "vkDestroyImage"));
	vkGetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(LoadProcAddress(handle, "vkGetImageSubresourceLayout"));
	vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(LoadProcAddress(handle, "vkCreateImageView"));
	vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(LoadProcAddress(handle, "vkDestroyImageView"));
	vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(LoadProcAddress(handle, "vkCreateShaderModule"));
	vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(LoadProcAddress(handle, "vkDestroyShaderModule"));
	vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(LoadProcAddress(handle, "vkCreatePipelineCache"));
	vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(LoadProcAddress(handle, "vkDestroyPipelineCache"));
	vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(LoadProcAddress(handle, "vkGetPipelineCacheData"));
	vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(LoadProcAddress(handle, "vkMergePipelineCaches"));
	vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(LoadProcAddress(handle, "vkCreateGraphicsPipelines"));
	vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(LoadProcAddress(handle, "vkCreateComputePipelines"));
	vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(LoadProcAddress(handle, "vkDestroyPipeline"));
	vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(LoadProcAddress(handle, "vkCreatePipelineLayout"));
	vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(LoadProcAddress(handle, "vkDestroyPipelineLayout"));
	vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(LoadProcAddress(handle, "vkCreateSampler"));
	vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(LoadProcAddress(handle, "vkDestroySampler"));
	vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(LoadProcAddress(handle, "vkCreateDescriptorSetLayout"));
	vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(LoadProcAddress(handle, "vkDestroyDescriptorSetLayout"));
	vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(LoadProcAddress(handle, "vkCreateDescriptorPool"));
	vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(LoadProcAddress(handle, "vkDestroyDescriptorPool"));
	vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(LoadProcAddress(handle, "vkResetDescriptorPool"));
	vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(LoadProcAddress(handle, "vkAllocateDescriptorSets"));
	vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(LoadProcAddress(handle, "vkFreeDescriptorSets"));
	vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(LoadProcAddress(handle, "vkUpdateDescriptorSets"));
	vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(LoadProcAddress(handle, "vkCreateFramebuffer"));
	vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(LoadProcAddress(handle, "vkDestroyFramebuffer"));
	vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(LoadProcAddress(handle, "vkCreateRenderPass"));
	vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(LoadProcAddress(handle, "vkDestroyRenderPass"));
	vkGetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(LoadProcAddress(handle, "vkGetRenderAreaGranularity"));
	vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(LoadProcAddress(handle, "vkCreateCommandPool"));
	vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(LoadProcAddress(handle, "vkDestroyCommandPool"));
	vkResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(LoadProcAddress(handle, "vkResetCommandPool"));
	vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(LoadProcAddress(handle, "vkAllocateCommandBuffers"));
	vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(LoadProcAddress(handle, "vkFreeCommandBuffers"));
	vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(LoadProcAddress(handle, "vkBeginCommandBuffer"));
	vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(LoadProcAddress(handle, "vkEndCommandBuffer"));
	vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(LoadProcAddress(handle, "vkResetCommandBuffer"));
	vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(LoadProcAddress(handle, "vkCmdBindPipeline"));
	vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(LoadProcAddress(handle, "vkCmdSetViewport"));
	vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(LoadProcAddress(handle, "vkCmdSetScissor"));
	vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(LoadProcAddress(handle, "vkCmdSetLineWidth"));
	vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(LoadProcAddress(handle, "vkCmdSetDepthBias"));
	vkCmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(LoadProcAddress(handle, "vkCmdSetBlendConstants"));
	vkCmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(LoadProcAddress(handle, "vkCmdSetDepthBounds"));
	vkCmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(LoadProcAddress(handle, "vkCmdSetStencilCompareMask"));
	vkCmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(LoadProcAddress(handle, "vkCmdSetStencilWriteMask"));
	vkCmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(LoadProcAddress(handle, "vkCmdSetStencilReference"));
	vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(LoadProcAddress(handle, "vkCmdBindDescriptorSets"));
	vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(LoadProcAddress(handle, "vkCmdBindIndexBuffer"));
	vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(LoadProcAddress(handle, "vkCmdBindVertexBuffers"));
	vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(LoadProcAddress(handle, "vkCmdDraw"));
	vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(LoadProcAddress(handle, "vkCmdDrawIndexed"));
	vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(LoadProcAddress(handle, "vkCmdDrawIndirect"));
	vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(LoadProcAddress(handle, "vkCmdDrawIndexedIndirect"));
	vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(LoadProcAddress(handle, "vkCmdDispatch"));
	vkCmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(LoadProcAddress(handle, "vkCmdDispatchIndirect"));
	vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(LoadProcAddress(handle, "vkCmdCopyBuffer"));
	vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(LoadProcAddress(handle, "vkCmdCopyImage"));
	vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(LoadProcAddress(handle, "vkCmdBlitImage"));
	vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(LoadProcAddress(handle, "vkCmdCopyBufferToImage"));
	vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(LoadProcAddress(handle, "vkCmdCopyImageToBuffer"));
	vkCmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(LoadProcAddress(handle, "vkCmdUpdateBuffer"));
	vkCmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(LoadProcAddress(handle, "vkCmdFillBuffer"));
	vkCmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(LoadProcAddress(handle, "vkCmdClearColorImage"));
	vkCmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(LoadProcAddress(handle, "vkCmdClearDepthStencilImage"));
	vkCmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(LoadProcAddress(handle, "vkCmdClearAttachments"));
	vkCmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(LoadProcAddress(handle, "vkCmdResolveImage"));
	vkCmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(LoadProcAddress(handle, "vkCmdSetEvent"));
	vkCmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(LoadProcAddress(handle, "vkCmdResetEvent"));
	vkCmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(LoadProcAddress(handle, "vkCmdWaitEvents"));
	vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(LoadProcAddress(handle, "vkCmdPipelineBarrier"));
	vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(LoadProcAddress(handle, "vkCmdBeginQuery"));
	vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(LoadProcAddress(handle, "vkCmdEndQuery"));
	vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(LoadProcAddress(handle, "vkCmdResetQueryPool"));
	vkCmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(LoadProcAddress(handle, "vkCmdWriteTimestamp"));
	vkCmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(LoadProcAddress(handle, "vkCmdCopyQueryPoolResults"));
	vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(LoadProcAddress(handle, "vkCmdPushConstants"));
	vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(LoadProcAddress(handle, "vkCmdBeginRenderPass"));
	vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(LoadProcAddress(handle, "vkCmdNextSubpass"));
	vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(LoadProcAddress(handle, "vkCmdEndRenderPass"));
	vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(LoadProcAddress(handle, "vkCmdExecuteCommands"));
	vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(LoadProcAddress(handle, "vkDestroySurfaceKHR"));
	vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceSurfaceSupportKHR"));
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
	vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(LoadProcAddress(handle, "vkCreateSwapchainKHR"));
	vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(LoadProcAddress(handle, "vkDestroySwapchainKHR"));
	vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(LoadProcAddress(handle, "vkGetSwapchainImagesKHR"));
	vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(LoadProcAddress(handle, "vkAcquireNextImageKHR"));
	vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(LoadProcAddress(handle, "vkQueuePresentKHR"));
	vkGetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
	vkGetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
	vkGetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(LoadProcAddress(handle, "vkGetDisplayPlaneSupportedDisplaysKHR"));
	vkGetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(LoadProcAddress(handle, "vkGetDisplayModePropertiesKHR"));
	vkCreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(LoadProcAddress(handle, "vkCreateDisplayModeKHR"));
	vkGetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(LoadProcAddress(handle, "vkGetDisplayPlaneCapabilitiesKHR"));
	vkCreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(LoadProcAddress(handle, "vkCreateDisplayPlaneSurfaceKHR"));
	vkCreateSharedSwapchainsKHR = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(LoadProcAddress(handle, "vkCreateSharedSwapchainsKHR"));

#ifdef VK_USE_PLATFORM_XLIB_KHR
	vkCreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(LoadProcAddress(handle, "vkCreateXlibSurfaceKHR"));
	vkGetPhysicalDeviceXlibPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceXlibPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
	vkCreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(LoadProcAddress(handle, "vkCreateXcbSurfaceKHR"));
	vkGetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	vkCreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(LoadProcAddress(handle, "vkCreateWaylandSurfaceKHR"));
	vkGetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
	vkCreateMirSurfaceKHR = reinterpret_cast<PFN_vkCreateMirSurfaceKHR>(LoadProcAddress(handle, "vkCreateMirSurfaceKHR"));
	vkGetPhysicalDeviceMirPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceMirPresentationSupportKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceMirPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	vkCreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(LoadProcAddress(handle, "vkCreateAndroidSurfaceKHR"));
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
	vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(LoadProcAddress(handle, "vkCreateWin32SurfaceKHR"));
	vkGetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(LoadProcAddress(handle, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif
}

// No Vulkan support, do not set function addresses
PFN_vkCreateInstance vkCreateInstance;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
PFN_vkQueueBindSparse vkQueueBindSparse;
PFN_vkCreateFence vkCreateFence;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkResetFences vkResetFences;
PFN_vkGetFenceStatus vkGetFenceStatus;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkCreateEvent vkCreateEvent;
PFN_vkDestroyEvent vkDestroyEvent;
PFN_vkGetEventStatus vkGetEventStatus;
PFN_vkSetEvent vkSetEvent;
PFN_vkResetEvent vkResetEvent;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkCreateBufferView vkCreateBufferView;
PFN_vkDestroyBufferView vkDestroyBufferView;
PFN_vkCreateImage vkCreateImage;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
PFN_vkMergePipelineCaches vkMergePipelineCaches;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkResetCommandPool vkResetCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
PFN_vkCmdFillBuffer vkCmdFillBuffer;
PFN_vkCmdClearColorImage vkCmdClearColorImage;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCmdResolveImage vkCmdResolveImage;
PFN_vkCmdSetEvent vkCmdSetEvent;
PFN_vkCmdResetEvent vkCmdResetEvent;
PFN_vkCmdWaitEvents vkCmdWaitEvents;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;
PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR;
PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR;
PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR;
PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;
PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR;

#ifdef VK_USE_PLATFORM_XLIB_KHR
PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;
PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
PFN_vkCreateMirSurfaceKHR vkCreateMirSurfaceKHR;
PFN_vkGetPhysicalDeviceMirPresentationSupportKHR vkGetPhysicalDeviceMirPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif