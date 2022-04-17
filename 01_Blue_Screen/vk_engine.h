#pragma once

#include <windows.h>


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#include "vk_types.h"


struct QueueFamilyIndices {

	//uint32_t graphicsFamily;
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	//optional is wrapper that contains no value until you assign something to it..has_value()
	
	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

/*
* There are basically three kinds of properties we need to check:
* Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
* Surface formats (pixel format, color space)
* Available presentation modes
* 
*/
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanEngine {
public:

	bool m_isInitialized;// { false };
	int m_frameNumber;// { 0 };

	HWND m_hwnd;
	HINSTANCE m_hInstance;
	//init
	void init(HINSTANCE hInstance, int iCmdShow);

	//
	void cleanup();
	//draw loop
	void resize(int width, int height);
	//main loop
	void run();

	VulkanEngine() :
		m_isInitialized(false), 
		m_frameNumber(0), 
		m_hwnd(NULL), 
		m_hInstance (NULL),
		m_bFullscreen(false),  
		m_wpPrev({ sizeof(WINDOWPLACEMENT) }), 
		gWidth (0),
		gHeight (0),
		gpFile(NULL), 
		m_enableValidationLayers(true)  
	{}

	bool m_bFullscreen;
	
	DWORD m_dwStyle;
	WINDOWPLACEMENT m_wpPrev;// = { sizeof(WINDOWPLACEMENT) };
	
	float gWidth, gHeight;

	void ToggleFullscreen(void);


	//vulkan specific variables

	VkInstance m_vkinstance; //vulkan handle
	VkDebugUtilsMessengerEXT m_debug_messenger;
	VkPhysicalDevice m_chosenGPUPhysicalDevice;
	VkDevice m_device; //device for commands (logical device to interface)
	//logical device dont directly interact with instances
	VkQueue m_graphicsQueue;

	//to establish connection between vulkan and window system to present results to the screen
	//we need to use WSI (windows system integration) extensions.

	VkSurfaceKHR m_surface; //window surface

	//presentation queue
	VkQueue m_presentQueue;

	//swapChain
	VkSwapchainKHR m_swapChain;

	FILE* gpFile;

	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	//this imageView is ready to be used as texture but not yet as renderTarget
	//this require framebuffer
	std::vector<VkImageView> m_swapChainImageViews;
	//view into an image
	//how to access the image and which part of image to access

	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;

	VkPipeline m_graphicsPipeline;

	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	//create a framebuffer for all the images in the swapchain

	//command pools
	VkCommandPool m_commandPool;

	//commandBuffer
	// For Single Frame
	//VkCommandBuffer m_commandBuffer;
	//semaphore objects and fence object
	//VkSemaphore m_imageAvailableSemaphore;
	//VkSemaphore m_renderFinishedSemaphore;
	//VkFence m_inFlightFence;

	//for multiple frames
	std::vector<VkCommandBuffer> m_commandBuffers;
	//semaphore objects and fence object
	std::vector <VkSemaphore> m_imageAvailableSemaphores;
	std::vector <VkSemaphore> m_renderFinishedSemaphores;
	std::vector <VkFence> m_inFlightFences;

	bool framebufferResized = false;
private:
	
	bool m_enableValidationLayers;
	
	/*setup*/
	void init_vulkan();
	void createInstance();
	void setupDebugMessanger();

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger, 
		const VkAllocationCallbacks* pAllocator);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	//physical devicve
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);

	
	//queue
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	//logicalDevice
	void createLogicalDevice();
	uint32_t m_enabled_layer_count;
	char* m_enabled_layers[64];

	/*Presentation*/
	//surface
	void createSurface();

	//swapChain
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void createSwapChain();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


	//imageViews
	void createImageViews();
	//creates a basic image view for every image in the swap chain
	//so that we can use them as color targets later on.

	/*GraphicsPipeline*/
	void createGraphicsPipeline();

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createRenderPass();

	void createFramebuffers();

	//commandPool creation
	void createCommandPool();
	//command buffer
	void createCommandBuffers();

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void createSyncObjects();

	//for resizing
	void recreateSwapChain();

	void cleanupSwapChain();
};

//for shaders
static std::vector<char> readFile(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}