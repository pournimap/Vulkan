// HEADER FILES
#include<windows.h>

#include<cstdio>	// FOR FILE I/O FUNTION  
#include<stdlib.h>	// FOR "exit()" 
#include <chrono>
#include <iostream>

#include <random>
//#include<vector>

#include "Vk.h"

//vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

//glm related macros and header files
#define GLM_FORCE_RADIANS   //we are forcing glm to get all angles in radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE //depth la consider kartana 0 to 1 ch kar, otherwise clip kar (GL_LEQUAL in opengl)


#include "glm/glm.hpp"  //hpp hi cpp special ahe ti c la nai chalnar
#include "glm/gtc/matrix_transform.hpp" //gtc hi texture and texture compression chi directory keliy



//vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//const uint32_t PARTICLE_COUNT = 8196;
const uint32_t PARTICLE_COUNT = 2048;

// GLOBAL FUNCTION DECLARATIONS 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// GLOBAL VARIABLE DECLARATIONS
HWND ghwnd = NULL;
HDC ghdc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbWindowMinimised = FALSE; //for minimize case

FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

const char* gpszAppName = "ARTR";

//vulkan related global variables

//Instance Extension related variables
//Vulkan is compulsory 64 bit
uint32_t enabledInstanceExtensionCount = 0;
//VK_KHR_SURFACE_EXTENSION_NAME , VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[3];

//Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

//Vulkan Presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//vulkan physical device related global variable
VkPhysicalDevice vkPhysicalDevice_Selected = VK_NULL_HANDLE;

uint32_t graphicsQueueFamilyIndex_Selected = UINT32_MAX;

VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

//variable declarations
uint32_t physicalDeviceCount = 0;

VkPhysicalDevice* vkPhysicalDevice_array = NULL;

//device extension
uint32_t enabledDeviceExtensionCount = 0;
// VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_array[1];
//in raytracing array become size 8

VkDevice vkDevice = VK_NULL_HANDLE;

VkQueue vkQueue;
VkQueue vkComputeQueue;

//Color format and color space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//PresentationMode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
//VkPresentModeKHR vkPresent_Mode_FIFO_KHR;

//SwapChain
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

//SwapChain images and ImgageViews Related
uint32_t swapChainImageCount = UINT32_MAX;
VkImage* swapChainImage_array = NULL;
VkImageView* swapChainImageView_array = NULL;

//CommandPool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

//Command Buffer
VkCommandBuffer* vkCommandBuffer_array = NULL;
VkCommandBuffer* vkComputeCommandBuffer_array = NULL;

//RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//framebuffer
VkFramebuffer* vkFramebuffer_array = NULL;

VkSemaphore vkSamaphore_backBuffer;

std::vector<VkSemaphore> vkSamaphore_renderComplete;
std::vector<VkSemaphore> vkComputeSemaphore_renderComplete;

VkFence* vkFence_array = NULL;
VkFence* vkComputeFence_array = NULL;
//std::vector< VkFence> vkComputeFence_array;

//clearColors
VkClearColorValue vkClearColorValue;

BOOL bInitialised = FALSE;
BOOL bInitialisedResize = FALSE;

uint32_t currentImageIndex = UINT_MAX;

//validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayerNames_array[1]; //For VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//vertex buffer related variables
typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
} VertexData;

//Position
VertexData vertexData_Position;
VertexData vertexData_Color;
//std::vector<VertexData> vertexData_Position_SSBO;
VertexData vertexData_Position_SSBO[2];
//uniform related variables
struct MyUniformData
{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

//compute
struct MyComputeUniformData
{
	float deltaTime = 1.0f;
};

//vertex buffer related variables
typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
} UniformData;
UniformData uniformData;
std::vector<UniformData> computeUniformData;

//shader related variable 
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_compute_shader = VK_NULL_HANDLE;

//DescriptorSetLayout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout vkComputeDescriptorSetLayout = VK_NULL_HANDLE;

//VkPipelineLayout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

VkPipelineLayout vkComputePipelineLayout = VK_NULL_HANDLE;

//descriptor pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

//descriptor set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;
std::vector<VkDescriptorSet> vkComputeDescriptorSets;

VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline;
VkPipeline vkComputePipeline;

struct Particle {
	glm::vec2 position; //8
	glm::vec2 velocity; //8
	glm::vec4 color;	//16

	//static VkVertexInputBindingDescription getBindingDescription() {
	//	VkVertexInputBindingDescription bindingDescription{};
	//	bindingDescription.binding = 0;
	//	bindingDescription.stride = sizeof(Particle);
	//	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	//	return bindingDescription;
	//}

	/*static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Particle, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Particle, color);

		return attributeDescriptions;
	}*/
};
std::vector<Particle> particles(PARTICLE_COUNT);
float lastFrameTime = 0.0f;
double lastTime = 0.0f;

// Define a type for high-resolution time
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<high_resolution_clock>;

// Function to get the current time in seconds
double get_time() {
	static time_point start_time = high_resolution_clock::now();
	time_point current_time = high_resolution_clock::now();
	return std::chrono::duration<double>(current_time - start_time).count();
}

// ENTRY POINT FUNCTION
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// FUNCTION DECLARATION
	VkResult initialize(void);
	void uninitialize(void);
	VkResult display(void);
	void update(void);


	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	BOOL bDone = FALSE;		// NEWLY ADDED VARIABLE FOR GAME LOOP
	int iRetVal = 0;
	RECT rc;
	BOOL fResult;
	int XCoordinate;
	int YCoordinate;

	VkResult vkResult = VK_SUCCESS;

	// CODE
	fResult = SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	if (fResult)
	{
		XCoordinate = (((rc.left + rc.right) / 2) - (WIN_WIDTH / 2));
		YCoordinate = (((rc.top + rc.bottom) / 2) - (WIN_HEIGHT / 2));
	}

	gpFile = fopen("Log.txt", "w");

	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("Creation of log file is failed. Exitting."), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "WinMain() : Log file is opened successfully.\n\n");
		fflush(gpFile);
	}

	wsprintf(szAppName, TEXT("%s"), gpszAppName);

	// INITIALIZATION OF WNDCLASSEX STRUCTURE
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	//REGISTER WNDDCLASSEX
	RegisterClassEx(&wndclass);

	//CREATE THE WINDOW
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Vulkan : 34_MultiColorTriangle"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		XCoordinate,
		YCoordinate,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	// ASSIGNING hwnd to GLOBALE ghwnd
	ghwnd = hwnd;

	// INITIALIZE
	vkResult = initialize();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "WinMain() : Initialize() Is Failed \n\n");
		fflush(gpFile);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fprintf(gpFile, "WinMain() : Initialize() successfull \n\n");
		fflush(gpFile);
	}

	// SHOW WINDOW
	ShowWindow(hwnd, iCmdShow);

	// FORGROUNDING AND FOCUSING THE WINDOW
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// GAME LOOP
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				if (gbWindowMinimised == FALSE)
				{
					// RENDER THE SCEAN
					vkResult = display();
					//the two case not required ErrorOutOfdate and suboptimal, for future use
					if (vkResult != VK_FALSE && vkResult != VK_SUCCESS && vkResult != VK_ERROR_OUT_OF_DATE_KHR && vkResult != VK_SUBOPTIMAL_KHR)
					{
						fprintf(gpFile, "WinMain() : call to display() failed \n\n");
						fflush(gpFile);
						bDone = TRUE;
					}
					// UPDATE THE SCEAN
					update();

					double currentTime = get_time();
					lastFrameTime = (currentTime - lastTime) * 1000.0;
					lastTime = currentTime;
				}
			}
		}
	}

	uninitialize();
	return((int)msg.wParam);
}

// CALLBACK FUNCTION
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// FUNCTION DECLARATION 
	void ToggleFullScreen(void);
	VkResult resize(int, int);
	void uninitialize(void);

	// CODE
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;

	case WM_ERASEBKGND:
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;

		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			DestroyWindow(hwnd);
			break;

		default:
			break;
		}
		break;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) //handling minimize case
		{
			gbWindowMinimised = TRUE;
		}
		else
		{
			resize(LOWORD(lParam), HIWORD(lParam));
			gbWindowMinimised = FALSE;
		}

		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		uninitialize();
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;

	case WM_CREATE:
		memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
		wpPrev.length = sizeof(WINDOWPLACEMENT);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
	// VARIABLE DECLARATIONS

	MONITORINFO mi;

	// CODE
	//

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
			gbFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}

VkResult initialize(void)
{
	// CODE

	//Function declaration
	VkResult createVulkanInstance(void);
	VkResult getSupportedSurface(void);
	VkResult getPhysicalDevice(void);
	VkResult printVkInfo();

	VkResult fillDeviceExtensionNames(void);
	VkResult createVulkanDevice(void);
	void getDeviceQueue();


	VkResult createSwapchainHere(VkBool32);
	VkResult createImagesAndImageViews(void);

	VkResult createCommandPool(void);
	VkResult createCommandBuffers(void);
	VkResult createComputeCommandBuffers(void);

	VkResult createVertexBuffer(void);

	VkResult createUniformBuffer(void);
	VkResult createComputeUniformBuffer(void);

	VkResult createShaders(void);

	VkResult createRenderPass(void);

	VkResult createPipeline(void);
	VkResult createComputePipeline(void);

	VkResult createFramebuffers(void);

	VkResult CreateSemaphores(void);
	VkResult createFences(void);

	VkResult buildCommandBuffers(void);
	VkResult buildComputeCommandBuffers(void);

	VkResult createDescriptorSetLayout(void);
	VkResult createComputeDescriptorSetLayout(void);

	VkResult createPipelineLayout(void);
	VkResult createComputePipelineLayout(void);

	VkResult createDescriptorPool(void);
	VkResult createDescriptorSet(void);
	VkResult createComputeDescriptorSet(void);

	VkResult createShaderStorageBuffers(void);

	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createVulkanInstance() Is Failed due to unknown reason (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createVulkanInstance() successfull \n\n");
		fflush(gpFile);
	}

	//create Vulkan Presentation Surface
	vkResult = getSupportedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : getSupportedSurface() Is Failed due to unknown reason (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : getSupportedSurface() successfull \n\n");
		fflush(gpFile);
	}

	//enumerate and select required physical device and queue index
	vkResult = getPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : getPhysicalDevice() Is Failed due to unknown reason (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : getPhysicalDevice() successfull \n\n");
		fflush(gpFile);
	}

	//print Vulkan Info
	vkResult = printVkInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : printVkInfo() Is Failed due to unknown reason (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : printVkInfo() successfull \n\n");
		fflush(gpFile);
	}

	//get device exitension
	/*vkResult = fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : fillDeviceExtensionNames() Is Failed \n\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : fillDeviceExtensionNames() successfull \n\n");
	}*/

	vkResult = createVulkanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createVulkanDevice() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createVulkanDevice() successfull \n\n");
		fflush(gpFile);
	}
	getDeviceQueue();

	//swapChain
	vkResult = createSwapchainHere(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createSwapchain() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;  //Why here return Hradcoded error init Failed
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createSwapchain() successfull \n\n");
		fflush(gpFile);
	}

	//create vulkan Images and ImageViews
	vkResult = createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createImagesAndImageViews() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;  //Why here return Hradcoded error init Failed
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createImagesAndImageViews() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createCommandPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createCommandPool() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createCommandPool() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createCommandBuffers() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createComputeCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createComputeCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createComputeCommandBuffers() successfull \n\n");
		fflush(gpFile);
	}

	/*vkResult = createVertexBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createVertexBuffer() Is failed due to %d \n\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createVertexBuffer() successfull \n\n");
	}*/

	vkResult = createShaderStorageBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createShaderStorageBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createShaderStorageBuffers() successfull \n\n");
		fflush(gpFile);
	}

	/*vkResult = createUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createUniformBuffer() Is failed due to %d \n\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createUniformBuffer() successfull \n\n");
	}*/

	vkResult = createComputeUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createComputeUniformBuffer() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createComputeUniformBuffer() successfull \n\n");
		fflush(gpFile);
	}

	/*vkResult = createDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createDescriptorSetLayout() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createDescriptorSetLayout() successfull \n\n");
		fflush(gpFile);
	}*/
	vkResult = createComputeDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createDescriptorSetLayout() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createDescriptorSetLayout() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createPipelineLayout() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createPipelineLayout() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createComputePipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createComputePipelineLayout() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createComputePipelineLayout() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createDescriptorPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createDescriptorPool() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createDescriptorPool() successfull \n\n");
		fflush(gpFile);
	}

	/*vkResult = createDescriptorSet();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createDescriptorSet() Is failed due to %d \n\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createDescriptorSet() successfull \n\n");
	}*/

	vkResult = createComputeDescriptorSet();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createComputeDescriptorSet() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createComputeDescriptorSet() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createShaders();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createShaders() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createShaders() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createRenderPass() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createRenderPass() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createPipeline() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createPipeline() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createComputePipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createComputePipeline() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createComputePipeline() successfull \n\n");
		fflush(gpFile);
	}

	vkResult = createFramebuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createFramebuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createFramebuffers() successfull \n\n");
		fflush(gpFile);
	}

	//create semaphores
	vkResult = CreateSemaphores();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : CreateSemaphores() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : CreateSemaphores() successfull \n\n");
		fflush(gpFile);
	}


	vkResult = buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : buildCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = buildComputeCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : buildComputeCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	//init clear color values
	memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));
	vkClearColorValue.float32[0] = 0.0f;
	vkClearColorValue.float32[1] = 0.0f;
	vkClearColorValue.float32[2] = 0.0f; //Black background
	vkClearColorValue.float32[3] = 1.0f;
	//analogous to clear color


	//create Fences
	vkResult = createFences();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "initialize() : createFences() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "initialize() : createFences() successfull \n\n");
		fflush(gpFile);
	}

	//initialization is completed
	bInitialised = TRUE;

	fprintf(gpFile, "initialize() :  Initialisation is completed\n\n");
	fflush(gpFile);

	lastTime = get_time();
	return(vkResult);
}

VkResult resize(int width, int height)
{
	//function declaration
	VkResult createSwapchainHere(VkBool32);
	VkResult createImagesAndImageViews(void);
	VkResult createCommandBuffers(void);
	VkResult createComputeCommandBuffers(void);
	VkResult createPipelineLayout(void);
	VkResult createComputePipelineLayout(void);
	VkResult createPipeline(void);
	VkResult createComputePipeline(void);
	VkResult createRenderPass(void);
	VkResult createFramebuffers(void);
	VkResult buildCommandBuffers(void);
	VkResult buildComputeCommandBuffers(void);

	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	// CODE
	if (height <= 0)
		height = 1;

	//check the bInitialized variable
	if (bInitialised == FALSE)
	{
		fprintf(gpFile, "resize() : Initialisation yet not completed or failed\n\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	//as recreation of swapchain is needed, we are going to repeat many steps of initialized again, Hence set bInitiased to false again
	bInitialised = FALSE;

	//Step 1 - set global winWidth and winHeight Variable
	winWidth = width;
	winHeight = height;

	// 8 things destroy
	//wait for device to complete inhand task
	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
	}

	//check presence of swapchain
	if (vkSwapchainKHR == VK_NULL_HANDLE) //nullptr, (void*)0, *0, ull
	{
		fprintf(gpFile, "\n resize() : swapchain is already NULL, cannot proceed\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	//Detroy frameBuffer
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
		fflush(gpFile);
		fprintf(gpFile, "\n resize() : vkDestroyFramebuffer() of %d is Done\n", i);
	}
	if (vkFramebuffer_array)
	{
		free(vkFramebuffer_array);
		vkFramebuffer_array = NULL;
	}

	//destroy renderPass
	if (vkRenderPass)
	{
		vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
		vkRenderPass = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize() : vkDestroyRenderPass() is Done\n");
		fflush(gpFile);
	}

	//destroy pipeline
	if (vkComputePipeline)
	{
		vkDestroyPipeline(vkDevice, vkComputePipeline, NULL);
		vkComputePipeline = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize() : vkDestroyPipeline() is Done\n");
		fflush(gpFile);
	}
	if (vkPipeline)
	{
		vkDestroyPipeline(vkDevice, vkPipeline, NULL);
		vkPipeline = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize() : vkDestroyPipeline() is Done\n");
		fflush(gpFile);
	}
	//destroy pipeline layout
	if (vkComputePipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, vkComputePipelineLayout, NULL);
		vkComputePipelineLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize() : vkDestroyPipelineLayout() is Done\n");
		fflush(gpFile);
	}
	if (vkPipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
		vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize() : vkDestroyPipelineLayout() is Done\n");
		fflush(gpFile);
	}

	//destroy commandBuffer
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
		fprintf(gpFile, "\n resize() : vkFreeCommandBuffers() of %d is Done\n", i);
		fflush(gpFile);
	}
	if (vkCommandBuffer_array)
	{
		free(vkCommandBuffer_array);
		vkCommandBuffer_array = NULL;
	}

	//destroy commandBuffer
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkComputeCommandBuffer_array[i]);
		fprintf(gpFile, "\n resize() : vkFreeCommandBuffers() of %d is Done\n", i);
		fflush(gpFile);
	}
	if (vkComputeCommandBuffer_array)
	{
		free(vkComputeCommandBuffer_array);
		vkComputeCommandBuffer_array = NULL;
	}

	//Destroy ImageViews
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkDestroyImageView(vkDevice, swapChainImageView_array[i], NULL); //ithe atlya Images free hotil
		fprintf(gpFile, "\n resize() : vkDestroyImageView() is Done\n");
		fflush(gpFile);

	}
	if (swapChainImageView_array)
	{
		free(swapChainImageView_array);

		swapChainImageView_array = NULL;

	}
	//destroy swapChainImage
	if (swapChainImage_array)
	{
		free(swapChainImage_array);
		swapChainImage_array = NULL;
	}

	//Destroy SwapChain
	if (vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize() : vkDestroySwapchain() is Done\n");
		fflush(gpFile);
	}

	//recreate for resize

	//createSwapChain

	vkResult = createSwapchainHere(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createSwapchain() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;  //Why here return Hradcoded error init Failed
		return(vkResult);
	}

	//create vulkan Images and ImageViews
	vkResult = createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createImagesAndImageViews() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;  //Why here return Hradcoded error init Failed
		return(vkResult);
	}

	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createComputeCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createComputeCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createPipelineLayout() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createComputePipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createComputePipelineLayout() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createRenderPass() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createPipeline() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createComputePipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createComputePipeline() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = createFramebuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : createFramebuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	/*vkResult = buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : buildCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	vkResult = buildComputeCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "resize() : buildComputeCommandBuffers() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}*/
	bInitialised = TRUE;

	return(vkResult);
}
//1st Usecase : Output is satisfactory, but validation showing validation err

VkResult display(void)
{
	// CODE
	//SHREE
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	VkResult resize(int, int);
	VkResult updateUniformBuffer(void);
	VkResult updateComputeUniformBuffer(void);
	VkResult recordCommandBuffers(uint32_t currentImageIndex);
	VkResult recordComputeCommandBuffers(uint32_t currentImageIndex);

	/*fprintf(gpFile, "display() : Hi Am In Dispaly**********\n\n");
	fflush(gpFile);*/
	//code
	//if control comes here,before initialisation gets completed, return False.
	if (bInitialised == FALSE)
	{
		fprintf(gpFile, "display() : Initialisation yet not completed\n\n");
		fflush(gpFile);
		return (VkResult)VK_FALSE;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly0**********\n\n");
	fflush(gpFile);*/

	//Aquire index of next swapChain Image
	vkResult = vkAcquireNextImageKHR(vkDevice,
		vkSwapchainKHR,
		UINT64_MAX,//timeout param (is in nano seconds) //here we are waiting "for swapchain" 
		vkSamaphore_backBuffer,//semaphore (it is waiting for another queue to release the Image held by another queue demanded by swapchain
		VK_NULL_HANDLE,//fence (host la thambvtat device sathi)
		&currentImageIndex
	);
	//if this funtion could not get image Index within given time -> VK_NOT_READY

	/*semaphore is used for interqueue operation(device to device operation)
	host la toparyant thambvaych asel tar Fence
	(compute + graphics => Fence vapra)*/
	/*fprintf(gpFile, "display() : Hi Am In Dispaly5**********\n\n");
	fflush(gpFile);*/

	if (vkResult != VK_SUCCESS)
	{
		//VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface and can no longer be used for rendering. Usually happens after a window resize.
		//VK_SUBOPTIMAL_KHR = The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched exactly.
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR | vkResult == VK_SUBOPTIMAL_KHR)
		{
			resize(winWidth, winHeight);
		}
		else
		{
			fprintf(gpFile, "display() : vkAquireNextImageKHR() failed\n\n");
			fflush(gpFile);
			return vkResult;
		}
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly6**********\n\n");
	fflush(gpFile);*/

	//Wait for one or more fences to become signaled
	vkResult = vkWaitForFences(vkDevice, 1, //array of fences (one at a time)
		&vkComputeFence_array[currentImageIndex],
		VK_TRUE, //I will wait for all fences to complete , to get sinalled(blocking) , kuthalahi ek complete zala tar return kar (unblocking)
		UINT64_MAX); //timeout
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : vkWaitForFences() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly1**********\n\n");
	fflush(gpFile);*/
	////now ready the fences for execution of next command Buffer
	vkResult = vkResetFences(vkDevice, 1, &vkComputeFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : vkResetFences() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}

	recordComputeCommandBuffers(currentImageIndex);

	/*fprintf(gpFile, "display() : Hi Am In Dispaly2**********\n\n");
	fflush(gpFile);*/
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	//
	/*fprintf(gpFile, "display() : Hi Am In Dispaly3**********\n\n");
	fflush(gpFile);*/
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkComputeCommandBuffer_array[currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &vkComputeSemaphore_renderComplete[currentImageIndex];

	//now submit above work to the queue
	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkComputeFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : vkQueueSubmit() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly4**********\n\n");
	fflush(gpFile);*/


	//
	///*use fence to allow host to wait for completion of execution of previous command buffer
	//command buffer ch operation complete vhav mhanun apan fence vapartoy

	//Wait for one or more fences to become signaled*/
	vkResult = vkWaitForFences(vkDevice, 1, //array of fences (one at a time)
		&vkFence_array[currentImageIndex],
		VK_TRUE, //I will wait for all fences to complete , to get sinalled(blocking) , kuthalahi ek complete zala tar return kar (unblocking)
		UINT64_MAX); //timeout

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : vkWaitForFences() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly7**********\n\n");
	fflush(gpFile);*/
	//now ready the fences for execution of next command Buffer
	vkResult = vkResetFences(vkDevice, 1, &vkFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : vkResetFences() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly8**********\n\n");
	fflush(gpFile);*/

	recordCommandBuffers(currentImageIndex);
	VkSemaphore waitSemaphores[] = { vkComputeSemaphore_renderComplete[currentImageIndex], vkSamaphore_backBuffer };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	////VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	//////const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//////declare , memset and initialize VkSubmitInfo Structure
	////VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = waitStages; //stage to stage wait -> barrier
	vkSubmitInfo.waitSemaphoreCount = 2;
	vkSubmitInfo.pWaitSemaphores = waitSemaphores;
	//vkSubmitInfo.pWaitSemaphores = &vkSamaphore_backBuffer; //kunasathi thambaych
	
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &vkSamaphore_renderComplete[currentImageIndex];

	//now submit above work to the queue
	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : vkQueueSubmit() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly9**********\n\n");
	fflush(gpFile);*/
	//we are going to present the rendered image after declaring and initialising VkPresentInfoKHR structure
	VkPresentInfoKHR vkPresentInfoKHR;
	memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount = 1;
	vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
	vkPresentInfoKHR.pImageIndices = &currentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
	vkPresentInfoKHR.pWaitSemaphores = &vkSamaphore_renderComplete[currentImageIndex]; //render samplyavar render karaych

	//now present the queue
	vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
	if (vkResult != VK_SUCCESS)
	{
		//VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface and can no longer be used for rendering. Usually happens after a window resize.
		//VK_SUBOPTIMAL_KHR = The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched exactly.
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR | vkResult == VK_SUBOPTIMAL_KHR)
		{
			resize(winWidth, winHeight);
		}
		else
		{
			fprintf(gpFile, "display() : vkQueuePresentKHR() failed\n\n");
			fflush(gpFile);
			return vkResult;
		}
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly10**********\n\n");
	fflush(gpFile);*/

	vkResult = updateComputeUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "display() : updateComputeUniformBuffer() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	/*fprintf(gpFile, "display() : Hi Am In Dispaly11**********\n\n");
	fflush(gpFile);*/
	//usecase no 1
	vkDeviceWaitIdle(vkDevice);

	return vkResult;
}

void update(void)
{
	// CODE
}

void uninitialize(void)
{
	// FUNCTION DECLARATIONS
	void ToggleFullScreen(void);

	//UNINITIALIZATION CODE
	if (gbFullScreen == TRUE)
	{
		ToggleFullScreen();
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);

		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkDestroyFence(vkDevice, vkFence_array[i], NULL);
			fprintf(gpFile, "\n vkDestroyFence() of %d is Done\n", i);
			fflush(gpFile);
		}

		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			if (vkFence_array[i])
			{
				free(vkFence_array[i]);
				vkFence_array[i] = NULL;
			}
			if (vkComputeFence_array[i])
			{
				free(vkComputeFence_array[i]);
				vkComputeFence_array[i] = NULL;
			}
		}
		for (size_t i = 0; i < swapChainImageCount; i++)
		{
			if (vkSamaphore_renderComplete[i])
			{
				vkDestroySemaphore(vkDevice, vkSamaphore_renderComplete[i], NULL);
				vkSamaphore_renderComplete[i] = VK_NULL_HANDLE;
				fprintf(gpFile, "\n vkDestroySemaphore() is Done\n");
				fflush(gpFile);
			}
		}
		for (size_t i = 0; i < swapChainImageCount; i++)
		{
			if (vkComputeSemaphore_renderComplete[i])
			{
				vkDestroySemaphore(vkDevice, vkComputeSemaphore_renderComplete[i], NULL);
				vkComputeSemaphore_renderComplete[i] = VK_NULL_HANDLE;
				fprintf(gpFile, "\n vkDestroySemaphore() is Done\n");
				fflush(gpFile);
			}
		}
		if (vkSamaphore_backBuffer)
		{
			vkDestroySemaphore(vkDevice, vkSamaphore_backBuffer, NULL);
			vkSamaphore_backBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroySemaphore() is Done\n");
			fflush(gpFile);
		}
		
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
			fprintf(gpFile, "\n vkDestroyFramebuffer() of %d is Done\n", i);
			fflush(gpFile);
		}
		if (vkFramebuffer_array)
		{
			free(vkFramebuffer_array);
			vkFramebuffer_array = NULL;
		}
		if (vkComputePipeline)
		{
			vkDestroyPipeline(vkDevice, vkComputePipeline, NULL);
			vkComputePipeline = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyPipeline() is Done\n");
			fflush(gpFile);
		}
		if (vkPipeline)
		{
			vkDestroyPipeline(vkDevice, vkPipeline, NULL);
			vkPipeline = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyPipeline() is Done\n");
			fflush(gpFile);
		}
		if (vkRenderPass)
		{
			vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
			vkRenderPass = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyRenderPass() is Done\n");
			fflush(gpFile);
		}
		//destroy descriptor Pool
		//when vkDescriptorPool destroyed, VkDescriptorSet also get destroyed implicitly
		if (vkDescriptorPool)
		{
			vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
			vkDescriptorPool = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyDescriptorPool() is Done\n");
			fflush(gpFile);
		}
		if (vkComputeDescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(vkDevice, vkComputeDescriptorSetLayout, NULL);
			vkComputeDescriptorSetLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyDescriptorSetLayout() is Done\n");
			fflush(gpFile);
		}
		if (vkDescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
			vkDescriptorSetLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyDescriptorSetLayout() is Done\n");
			fflush(gpFile);
		}
		if (vkComputePipelineLayout)
		{
			vkDestroyPipelineLayout(vkDevice, vkComputePipelineLayout, NULL);
			vkComputePipelineLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyPipelineLayout() is Done\n");
			fflush(gpFile);
		}
		if (vkPipelineLayout)
		{
			vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
			vkPipelineLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyPipelineLayout() is Done\n");
			fflush(gpFile);
		}

		if (vkShaderModule_vertex_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
			vkShaderModule_vertex_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyShaderModule() for vertex shader is Done\n");
			fflush(gpFile);
		}
		if (vkShaderModule_fragment_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
			vkShaderModule_fragment_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyShaderModule() for fragment shader is Done\n");
			fflush(gpFile);
		}
		if (vkShaderModule_compute_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_compute_shader, NULL);
			vkShaderModule_compute_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyShaderModule() for compute shader is Done\n");
			fflush(gpFile);
		}
		if (uniformData.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
			uniformData.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyBuffer() for uniformData is Done\n");
			fflush(gpFile);
		}
		if (uniformData.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
			uniformData.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkFreeMemory() for uniformData is Done\n");
			fflush(gpFile);
		}
		if (vertexData_Color.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, vertexData_Color.vkDeviceMemory, NULL);
			vertexData_Color.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkFreeMemory() for vertexData is Done\n");
			fflush(gpFile);
		}
		if (vertexData_Color.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, vertexData_Color.vkBuffer, NULL);
			vertexData_Color.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyBuffer() for ColorData is Done\n");
			fflush(gpFile);
		}
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			if (vertexData_Position_SSBO[i].vkDeviceMemory)
			{
				vkFreeMemory(vkDevice, vertexData_Position_SSBO[i].vkDeviceMemory, NULL);
				vertexData_Position_SSBO[i].vkDeviceMemory = VK_NULL_HANDLE;
				fprintf(gpFile, "\n vkFreeMemory() for vertexData is Done\n");
				fflush(gpFile);
			}
			if (vertexData_Position_SSBO[i].vkBuffer)
			{
				vkDestroyBuffer(vkDevice, vertexData_Position_SSBO[i].vkBuffer, NULL);
				vertexData_Position_SSBO[i].vkBuffer = VK_NULL_HANDLE;
				fprintf(gpFile, "\n vkDestroyBuffer() for vertexData is Done\n");
				fflush(gpFile);
			}
		}
		if (vertexData_Position.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, vertexData_Position.vkDeviceMemory, NULL);
			vertexData_Position.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkFreeMemory() for vertexData is Done\n");
			fflush(gpFile);
		}
		if (vertexData_Position.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, vertexData_Position.vkBuffer, NULL);
			vertexData_Position.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroyBuffer() for vertexData is Done\n");
			fflush(gpFile);
		}
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
			fprintf(gpFile, "\n vkFreeCommandBuffers() of %d is Done\n", i);
			fflush(gpFile);
		}
		if (vkCommandBuffer_array)
		{
			free(vkCommandBuffer_array);
			vkCommandBuffer_array = NULL;
		}
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkComputeCommandBuffer_array[i]);
			fprintf(gpFile, "\n vkFreeCommandBuffers() of %d is Done\n", i);
			fflush(gpFile);
		}
		if (vkComputeCommandBuffer_array)
		{
			free(vkComputeCommandBuffer_array);
			vkComputeCommandBuffer_array = NULL;
		}

		if (vkCommandPool)
		{
			vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
			fprintf(gpFile, "\n vkDestroyCommandPool() is Done\n");
			fflush(gpFile);
		}
		//step 9
		//Destroy ImageViews
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkDestroyImageView(vkDevice, swapChainImageView_array[i], NULL); //ithe atlya Images free hotil
			fprintf(gpFile, "\n vkDestroyImageView() is Done\n");
			fflush(gpFile);

		}
		if (swapChainImageView_array)
		{
			free(swapChainImageView_array);

			swapChainImageView_array = NULL;

		}

		//step 7 : free swapChainImage
		/*for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkDestroyImage(vkDevice, swapChainImage_array[i], NULL);
			fclose(gpFile);*/
			//UseCases
				/*MPD_Validation : debugReportCallback() : Validation (0) = Validation Error: [ VUID-vkDestroyImage-image-04882 ] Object 0: handle = 0xe7e6d0000000000f, type = VK_OBJECT_TYPE_IMAGE; | MessageID = 0xd636e760 | vkDestroyImage(): image VkImage 0xe7e6d0000000000f[] is a presentable image controlled by the implementation and must be destroyed with vkDestroySwapchainKHR.
	The Vulkan spec states: image must not have been acquired from vkGetSwapchainImagesKHR (https://vulkan.lunarg.com/doc/view/1.4.304.0/windows/1.4-extensions/vkspec.html#VUID-vkDestroyImage-image-04882)
	*/
	/*	fprintf(gpFile, "\n vkDestroyImage() is Done\n");
	}*/
		if (swapChainImage_array)
		{
			free(swapChainImage_array);
			swapChainImage_array = NULL;
		}



		//Destroy SwapChain
		if (vkSwapchainKHR)
		{
			vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
			vkSwapchainKHR = VK_NULL_HANDLE;
			fprintf(gpFile, "\n vkDestroySwapchain() is Done\n");
			fflush(gpFile);
		}

		//no need to destroy queue

		//Destroy vulkan device
		//if (vkDevice)
		//{
		fprintf(gpFile, "\n vkDeviceWaitIdle() is Done\n");
		fflush(gpFile);
		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpFile, "\n vkDestroyDevice() is Done\n");
		fflush(gpFile);

	}

	//no need to destroy physical device as logical device destruction internally take care of its destruction



	/*
	* STEP 4 :
	* // Provided by VK_KHR_surface
	* void vkDestroySurfaceKHR(
	* VkInstance                                  instance,
	* VkSurfaceKHR                                surface,
	* const VkAllocationCallbacks*                pAllocator);
	*/
	if (vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "uninitialize() : vkDestroySurfaceKHR Succedded\n\n");
		fflush(gpFile);
	}

	if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
	{
		vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
		vkDebugReportCallbackEXT = VK_NULL_HANDLE;
		vkDestroyDebugReportCallbackEXT_fnptr = NULL;
		fprintf(gpFile, "uninitialize() : vkDestroyDebugReportCallbackEXT_fnptr Succedded\n\n");
		fflush(gpFile);
	}

	/* STEP 4 :
	* Destroy Vulkan Instance				// Provided by VK_VERSION_1_0
	*	void vkDestroyInstance(
	*	VkInstance                                  instance,
	*	const VkAllocationCallbacks*                pAllocator);
	*/
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "uninitialize() : vkDestroyInstance Succedded\n\n");
		fflush(gpFile);
	}

	if (gpFile)
	{
		fprintf(gpFile, "uninitialize() : Program Terminated successfully closed\n\n");
		fflush(gpFile);
		fclose(gpFile);
		gpFile = NULL;
	}
}

/*
*
* Definations related to Vulkan
*
*/

VkResult createVulkanInstance(void)
{
	//Function declarations
	VkResult fillInstanceExtensionNames(void);
	VkResult fillValidationLayerNames(void);
	VkResult createValidationCallbackFunction(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code

	/* STEP 1 :
	*  Fill and initialize require extension names and count in global variables
	*/
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanInstance() : fillInstanceExtensionNames() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createVulkanInstance() : fillInstanceExtensionNames() successfull \n\n");
		fflush(gpFile);
	}

	/* STEP  :
	*  Fill and initialize require validation names and count in global variables
	*/
	if (bValidation == TRUE)
	{
		vkResult = fillValidationLayerNames();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createVulkanInstance() : fillValidationLayerNames() Is Failed \n\n");
			fflush(gpFile);
			return(vkResult);
		}
		else
		{
			fprintf(gpFile, "createVulkanInstance() : fillValidationLayerNames() successfull \n\n");
			fflush(gpFile);
		}
	}

	/* STEP 2 :
	*  Initialize struct VkApplicationInfo

	*	Structure specifying application information		// Provided by VK_VERSION_1_0
	*
	*	typedef struct VkApplicationInfo {
	*	VkStructureType    sType;
	*	const void*        pNext;
	*	const char*        pApplicationName;
	*	uint32_t           applicationVersion;
	*	const char*        pEngineName;
	*	uint32_t           engineVersion;
	*	uint32_t           apiVersion;
	*	} VkApplicationInfo;
	*/
	VkApplicationInfo vkApplicationInfo;
	memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.pNext = NULL;
	vkApplicationInfo.pApplicationName = gpszAppName;
	vkApplicationInfo.applicationVersion = 1;
	vkApplicationInfo.pEngineName = gpszAppName;
	vkApplicationInfo.engineVersion = 1;
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;//***must be the highest version of Vulkan that the application is designed to use

	/* STEP 3 :
	*	Initialize struct VkInstanceCreateInfo
	*
	*	Structure specifying parameters of a newly created instance		// Provided by VK_VERSION_1_0
	*	typedef struct VkInstanceCreateInfo {
	*	VkStructureType             sType;
	*	const void*                 pNext;
	*	VkInstanceCreateFlags       flags;
	*	const VkApplicationInfo*    pApplicationInfo;
	*	uint32_t                    enabledLayerCount;
	*	const char* const*          ppEnabledLayerNames;
	*	uint32_t                    enabledExtensionCount;
	*	const char* const*          ppEnabledExtensionNames;
	*	} VkInstanceCreateInfo;
	*/

	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount; //global variable already declared
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array; //global variable already declared

	if (bValidation == TRUE)
	{
		vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;
	}
	else
	{
		vkInstanceCreateInfo.enabledLayerCount = 0;
		vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
	}


	/* STEP 4 :
	* Call VkCreateInstance to get VkInstance in global variable and do error checking
	*
	*	Create a new Vulkan instance			// Provided by VK_VERSION_1_0
	*	VkResult vkCreateInstance(
	*	const VkInstanceCreateInfo*                 pCreateInfo,
	*	const VkAllocationCallbacks*                pAllocator,			//I dont have custom memory allocator, please use yours
	*	VkInstance*                                 pInstance);
	*/
	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "createVulkanInstance() : VkCreateInstance() Is Failed due to incompatible driver (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "createVulkanInstance() : VkCreateInstance() Is Failed due to extension not present (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanInstance() : VkCreateInstance() Is Failed due to unknown reason (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createVulkanInstance() : VkCreateInstance() Is Successful\n\n");
		fflush(gpFile);
	}

	//to for validation Callback
	if (bValidation == TRUE)
	{
		vkResult = createValidationCallbackFunction();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createVulkanInstance() : createValidationCallbackFunction() Is Failed \n\n");
			fflush(gpFile);
			return(vkResult);
		}
		else
		{
			fprintf(gpFile, "createVulkanInstance() : createValidationCallbackFunction() successfull \n\n");
			fflush(gpFile);
		}
	}

	return vkResult;
}

VkResult fillInstanceExtensionNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	/* STEP 1 :
	*  Find how many instance extension are supported by vulkan driver of this version
	*  and keep it in local variable
	*/

	uint32_t instanceExtensionCount = 0; //local variable

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	/*
	*	 Returns up to requested number of global extension properties		// Provided by VK_VERSION_1_0
	*
	*	VkResult vkEnumerateInstanceExtensionProperties(
	*	const char*                                 pLayerName,			//which layer extensions you want?
	*	uint32_t*                                   pPropertyCount,		//count : number of extension properties available
	*	VkExtensionProperties*                      pProperties);		//pointer to an array of VkExtensionProperties structures.
	*/
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : first call to vkEnumerateInstanceExtensionProperties() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : first call to vkEnumerateInstanceExtensionProperties() successfull \n\n");
		fflush(gpFile);
	}

	/* STEP 2 :
	*  Allocate and fill struct VkExtensionProperties array corresponding to above count
	*/
	VkExtensionProperties* vkExtensionProperties_array = NULL;

	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	/*
	*	Structure specifying an extension properties				// Provided by VK_VERSION_1_0
	*
	*	typedef struct VkExtensionProperties {
	*	char        extensionName[VK_MAX_EXTENSION_NAME_SIZE];
	*	uint32_t    specVersion;
	*	} VkExtensionProperties;
	*/

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : second call to vkEnumerateInstanceExtensionProperties() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : second call to vkEnumerateInstanceExtensionProperties() successfull \n\n");
		fflush(gpFile);
	}

	/* STEP 3 :
	*  Fill and display local string array of extension names obtained from VkExtensionProperties array
	*/
	//we dont know the size of array, so here we will use dynamic allocation
	char** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
	for (uint32_t i = 0; i < instanceExtensionCount; ++i)
	{
		instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1); //+1 for Null terminated char
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		fprintf(gpFile, "fillInstanceExtensionNames() : Vulkan instance extension name = %s \n\n", instanceExtensionNames_array[i]);
		fflush(gpFile);
	}

	/* STEP 4 :
	*  Free VkExtensionProperties array as it is not required henceforth
	*/
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;


	/* STEP 5 :
	*  Find whether above extension names contain our required two extensions
	*/
	// Provided by VK_VERSION_1_0
	//typedef uint32_t VkBool32;
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 win32SurfaceExtensionFound = VK_FALSE;
	//new declare here
	VkBool32 debugReportExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanSurfaceExtensionFound = VK_TRUE;

			enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			win32SurfaceExtensionFound = VK_TRUE;

			enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
		{
			debugReportExtensionFound = VK_TRUE;

			if (bValidation == TRUE)
			{
				enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
			else
			{
				//array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
			}
		}
	}

	/* STEP 6 :
	*  Free local string array "instanceExtensionNames_array" as it is not required henceforth
	*/
	for (uint32_t i = 0; i < instanceExtensionCount; ++i)
	{
		free(instanceExtensionNames_array[i]);
		instanceExtensionNames_array[i] = NULL;
	}
	free(instanceExtensionNames_array);
	instanceExtensionNames_array = NULL;

	/* STEP 7 :
	*  Print whether required extensions supported by our vulkan driver or not
	*/
	if (vulkanSurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;

		fprintf(gpFile, "fillInstanceExtensionNames() : VK_KHR_SURFACE_EXTENSION_NAME Not Found\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : VK_KHR_SURFACE_EXTENSION_NAME Found\n\n");
		fflush(gpFile);
	}

	if (win32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;

		fprintf(gpFile, "fillInstanceExtensionNames() : VK_KHR_WIN32_SURFACE_EXTENSION_NAME Not Found\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : VK_KHR_WIN32_SURFACE_EXTENSION_NAME Found\n\n");
		fflush(gpFile);
	}

	if (debugReportExtensionFound == VK_FALSE)
	{
		if (bValidation == TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED;

			fprintf(gpFile, "fillInstanceExtensionNames() : Validation is ON but required VK_EXT_DEBUG_REPORT_EXTENSION_NAME Not Supported\n\n");
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "fillInstanceExtensionNames() : Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME Not Supported\n\n");
			fflush(gpFile);
		}
	}
	else
	{
		if (bValidation == TRUE)
		{
			fprintf(gpFile, "fillInstanceExtensionNames() : Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is Supported\n\n");
			fflush(gpFile);
		}
		else
		{
			fprintf(gpFile, "fillInstanceExtensionNames() : Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is Supported\n\n");
			fflush(gpFile);
		}
	}
	/* STEP 8 :
	*  Print Only Enabled extensions name supported by our vulkan driver
	*/
	for (uint32_t i = 0; i < enabledInstanceExtensionCount; ++i)
	{
		fprintf(gpFile, "fillInstanceExtensionNames() : Enabled Vulkan Instance Extension Name = %s\n\n", enabledInstanceExtensionNames_array[i]);
		fflush(gpFile);
	}


	return vkResult;
}

VkResult fillValidationLayerNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	uint32_t validationLayerCount = 0; //local variable

	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
	/*
	* // Provided by VK_VERSION_1_0
	* VkResult vkEnumerateInstanceLayerProperties(
	* uint32_t*                                   pPropertyCount,
	* VkLayerProperties*                          pProperties);
	*/
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillValidationLayerNames() : first call to vkEnumerateInstanceLayerProperties() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "fillValidationLayerNames() : first call to vkEnumerateInstanceLayerProperties() successfull \n\n");
		fflush(gpFile);
	}

	VkLayerProperties* vkLayerProperties_array = NULL;
	vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * validationLayerCount);

	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillValidationLayerNames() : second call to vkEnumerateInstanceLayerProperties() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "fillValidationLayerNames() : second call to vkEnumerateInstanceLayerProperties() successfull \n\n");
		fflush(gpFile);
	}

	char** validationLayerNames_array = NULL;
	validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
	for (uint32_t i = 0; i < validationLayerCount; ++i)
	{
		validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1); //+1 for Null terminated char
		memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);

		fprintf(gpFile, "fillValidationLayerNames() : Vulkan validation layer name = %s \n\n", validationLayerNames_array[i]);
		fflush(gpFile);
	}

	free(vkLayerProperties_array);
	vkLayerProperties_array = NULL;

	//for required one layer
	VkBool32 vulkanValidationLayerFound = VK_FALSE;
	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		if (strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0) //diff ""
		{
			vulkanValidationLayerFound = VK_TRUE;

			enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
		}
	}

	for (uint32_t i = 0; i < validationLayerCount; ++i)
	{
		free(validationLayerNames_array[i]);
		validationLayerNames_array[i] = NULL;
	}
	free(validationLayerNames_array);
	validationLayerNames_array = NULL;

	if (vulkanValidationLayerFound == VK_FALSE)
	{
		if (bValidation == TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED;
			fprintf(gpFile, "fillValidationLayerNames() : Validation is ON but VK_LAYER_KHRONOS_validation Not Supported\n\n");
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "fillValidationLayerNames() : Validation is OFF and VK_LAYER_KHRONOS_validation Not Supported\n\n");
			fflush(gpFile);
		}
	}
	else
	{
		if (bValidation == TRUE)
		{
			fprintf(gpFile, "fillValidationLayerNames() :  Validation is ON and VK_LAYER_KHRONOS_validation Found\n\n");
			fflush(gpFile);
		}
		else
		{
			fprintf(gpFile, "fillValidationLayerNames() :  Validation is OFF and VK_LAYER_KHRONOS_validation Found\n\n");
			fflush(gpFile);

		}

	}

	for (uint32_t i = 0; i < enabledValidationLayerCount; ++i)
	{
		fprintf(gpFile, "fillValidationLayerNames() : Enabled Vulkan Validation Layer Name = %s\n\n", enabledValidationLayerNames_array[i]);
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createValidationCallbackFunction(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

	//function declaration
	// VKAPI_ATTR is calling convention for gcc -> linux and clang -> mac/ iphone
	// VKAPI_ATTR should used after C++11 
	//VkBool32 VKAPI_CALL is calling convention for win32 sathi
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
		VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT,
		uint64_t, size_t, int32_t, const char*, const char*,
		void*);
	//code

	//get the required function pointers
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "createValidationCallbackFunction() : vkGetInstanceProcAddr() failed to get function pointer for vkCreateDebugReportCallbackEXT\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createValidationCallbackFunction() : vkGetInstanceProcAddr() succeded to get function pointer for vkCreateDebugReportCallbackEXT\n\n");
	}

	vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "createValidationCallbackFunction() : vkGetInstanceProcAddr() failed to get function pointer for vkDestroyDebugReportCallbackEXT\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createValidationCallbackFunction() : vkGetInstanceProcAddr() succeded to get function pointer for vkDestroyDebugReportCallbackEXT\n\n");
		fflush(gpFile);
	}

	//get the vulkan debug report callback object
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
	memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
	/*
	* // Provided by VK_EXT_debug_report
typedef struct VkDebugReportCallbackCreateInfoEXT {
	VkStructureType                 sType;
	const void*                     pNext;
	VkDebugReportFlagsEXT           flags;
	PFN_vkDebugReportCallbackEXT    pfnCallback;
	void*                           pUserData;
} VkDebugReportCallbackCreateInfoEXT;
	*/
	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = NULL; //parameter to your callback function
	//all errors, warnings, perf issues, profiling, information
	//printk in device driver(MACRO) (KERN_DEBUG, KERN_PERF, KERN_INFO, KERN_ERR, KERN_WARNING)

	vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createValidationCallbackFunction() : vkCreateDebugReportCallbackEXT_fnptr() failed \n\n");
		fflush(gpFile);
	}
	else
	{
		fprintf(gpFile, "createValidationCallbackFunction() : vkCreateDebugReportCallbackEXT_fnptr() succeded \n\n");
		fflush(gpFile);
	}
	return vkResult;
}

VkResult getSupportedSurface(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	/* STEP 1 :
	*  Declare a global variable to hold a presentation surface Object
	*
	* STEP 2 :
	* Declare and memset platform specific Vk SurfaceCreateInfo structure
	* // Provided by VK_KHR_win32_surface
	* typedef struct VkWin32SurfaceCreateInfoKHR {
	* VkStructureType                 sType;
	* const void*                     pNext;
	* VkWin32SurfaceCreateFlagsKHR    flags;
	* HINSTANCE                       hinstance;
	* HWND                            hwnd;
	* } VkWin32SurfaceCreateInfoKHR;
	*/
	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

	/* STEP 3 :
	* Initialize it particularly its hInstance & hwnd members
	*/

	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);//(HINSTANCE)GetModuleHandle(NULL); one way
	vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

	/*
	* STEP 4 : call vkCreateWin32SrfaceKHR() to create presentation surface object
	* // Provided by VK_KHR_win32_surface
	* VkResult vkCreateWin32SurfaceKHR(
	* VkInstance                                  instance,
	* const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
	* const VkAllocationCallbacks*                pAllocator,
	* VkSurfaceKHR*                               pSurface);
	*/
	vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getSupportedSurface() : vkCreateWin32SurfaceKHR() Is Failed due to unknown reason (%d)\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "getSupportedSurface() : vkCreateWin32SurfaceKHR() Is Successful\n\n");
		fflush(gpFile);
	}

	return vkResult;

}

VkResult getPhysicalDevice()
{

	VkResult vkResult = VK_SUCCESS;

	//Enumerates the physical devices accessible to a Vulkan instance

	/*
	* // Provided by VK_VERSION_1_0
	*	VkResult vkEnumeratePhysicalDevices(
	*	VkInstance                                  instance,
	*	uint32_t*                                   pPhysicalDeviceCount,
	*	VkPhysicalDevice*                           pPhysicalDevices);
	*/
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevice() : first call to vkEnumaratePhysicalDevices() Is Failed due to reason %d\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else if (physicalDeviceCount == 0)
	{
		fprintf(gpFile, "getPhysicalDevice() : vkEnumaratePhysicalDevices resulted in physical device count = 0\n\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevice() : first call to vkEnumaratePhysicalDevices() successfull \n\n");
		fflush(gpFile);
	}

	//allocate memory

	vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

	//step 4
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevice() : second call to vkEnumaratePhysicalDevices() Is Failed due to reason %d\n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevice() : second call to vkEnumaratePhysicalDevices() successfull \n\n");
		fflush(gpFile);
	}

	fprintf(gpFile, "Physical Device Count = %d \n", physicalDeviceCount);
	fflush(gpFile);
	//step 5
	VkBool32 bFound = VK_FALSE;
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		uint32_t queueCount = UINT32_MAX;

		//Reports properties of the queues of the specified physical device
		/*
		* // Provided by VK_VERSION_1_0
		* void vkGetPhysicalDeviceQueueFamilyProperties(
		* VkPhysicalDevice                            physicalDevice,
		* uint32_t*                                   pQueueFamilyPropertyCount,
		* VkQueueFamilyProperties*                    pQueueFamilyProperties);
		*/
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);

		//step c :
		//Structure providing information about a queue family
		/*
		* // Provided by VK_VERSION_1_0
		* typedef struct VkQueueFamilyProperties {
		* VkQueueFlags    queueFlags;
		* uint32_t        queueCount;
		* uint32_t        timestampValidBits;
		* VkExtent3D      minImageTransferGranularity;
		* } VkQueueFamilyProperties;
		*/

		VkQueueFamilyProperties* vkQueueFamilyProperties_array = NULL;
		vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);

		//step 5 : d : 
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

		VkBool32* isQueueSurfaceSupported_array = NULL;
		isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);

		fprintf(gpFile, "Queue FamilyCOunt = %d \n", queueCount);
		fflush(gpFile);
		//f
		for (uint32_t j = 0; j < queueCount; j++)
		{
			//Query if presentation is supported
			/*
			* // Provided by VK_KHR_surface
			* VkResult vkGetPhysicalDeviceSurfaceSupportKHR(
			* VkPhysicalDevice                            physicalDevice,
			* uint32_t                                    queueFamilyIndex,
			* VkSurfaceKHR                                surface,
			* VkBool32*                                   pSupported);
			*/
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);

		}

		for (uint32_t j = 0; j < queueCount; j++)
		{
			if ((vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_COMPUTE_BIT))
			{

				if (isQueueSurfaceSupported_array[j] == VK_TRUE)
				{
					fprintf(gpFile, "getPhysicalDevice() : congrats selected %d\n\n", j);
					fflush(gpFile);
					vkPhysicalDevice_Selected = vkPhysicalDevice_array[i];
					graphicsQueueFamilyIndex_Selected = j;
					bFound = VK_TRUE;
				}

			}
		}

		if (isQueueSurfaceSupported_array)
		{
			free(isQueueSurfaceSupported_array);
			isQueueSurfaceSupported_array = NULL;
			fprintf(gpFile, "getPhysicalDevice() : succedded to free isQueueSurfaceSupported_array \n\n");
			fflush(gpFile);
		}

		if (vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;
			fprintf(gpFile, "getPhysicalDevice() : succedded to free vkQueueFamilyProperties_array \n\n");
			fflush(gpFile);
		}

		if (bFound == VK_TRUE)
			break;
	}

	if (bFound == VK_TRUE)
	{
		fprintf(gpFile, "getPhysicalDevice() : getPhysicalDevice() is succedded to select required physical device with graphics ennabled \n\n");
		fflush(gpFile);
		/*if (vkPhysicalDevice_array)
		{
			free(vkPhysicalDevice_array);
			vkPhysicalDevice_array = NULL;
			fprintf(gpFile, "getPhysicalDevice() : succedded to free vkPhysicalDevice_array \n\n");
		}*/
	}
	else
	{
		if (vkPhysicalDevice_array)
		{
			free(vkPhysicalDevice_array);
			vkPhysicalDevice_array = NULL;
			fprintf(gpFile, "getPhysicalDevice() : succedded to free vkPhysicalDevice_array \n\n");
			fflush(gpFile);
		}
		fprintf(gpFile, "getPhysicalDevice() : getPhysicalDevice() is Failed to select required physical device with graphics ennabled \n\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//Reports memory information for the specified physical device
	/*	// Provided by VK_VERSION_1_0
	*	void vkGetPhysicalDeviceMemoryProperties(
	*	VkPhysicalDevice                            physicalDevice,
	*	VkPhysicalDeviceMemoryProperties* pMemoryProperties);
	*/
	memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_Selected, &vkPhysicalDeviceMemoryProperties);

	//Structure describing the fine-grained features that can be supported by an implementation
	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

	//Reports capabilities of a physical device
	/*
	* // Provided by VK_VERSION_1_0
	* void vkGetPhysicalDeviceFeatures(
	* VkPhysicalDevice                            physicalDevice,
	* VkPhysicalDeviceFeatures*                   pFeatures);
	*/
	vkGetPhysicalDeviceFeatures(vkPhysicalDevice_Selected, &vkPhysicalDeviceFeatures);
	if (vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gpFile, "getPhysicalDevice() : Supported Tessellation Shader  \n\n");
		fflush(gpFile);
	}
	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpFile, "getPhysicalDevice() : Supported Geoemtry Shader  \n\n");
		fflush(gpFile);
	}
	return vkResult;

}

VkResult printVkInfo(void)
{
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "printVkInfo : ********************** Print Vulkan Information **************************  \n\n");
	fflush(gpFile);
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		//Structure specifying physical device properties

		/*// Provided by VK_VERSION_1_0
		typedef struct VkPhysicalDeviceProperties {
		uint32_t                            apiVersion;
		uint32_t                            driverVersion;
		uint32_t                            vendorID;
		uint32_t                            deviceID;
		VkPhysicalDeviceType                deviceType;
		char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
		uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
		VkPhysicalDeviceLimits              limits;
		VkPhysicalDeviceSparseProperties    sparseProperties;
		} VkPhysicalDeviceProperties;*/

		/*// Provided by VK_VERSION_1_0
		* void vkGetPhysicalDeviceProperties(
		* VkPhysicalDevice                            physicalDevice,
		* VkPhysicalDeviceProperties*                 pProperties);*/
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

		//Returns properties of a physical device
		/*
		* // Provided by VK_VERSION_1_0
		* void vkGetPhysicalDeviceProperties(
		* VkPhysicalDevice                            physicalDevice,
		* VkPhysicalDeviceProperties*                 pProperties);
		*/
		vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

		uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

		//api version
		fprintf(gpFile, "printVkInfo : apiVersion = %d.%d.%d  \n\n", majorVersion, minorVersion, patchVersion);
		fflush(gpFile);

		//device name
		fprintf(gpFile, "printVkInfo : Device Name = %s  \n\n", vkPhysicalDeviceProperties.deviceName);
		fflush(gpFile);

		//device type
		switch (vkPhysicalDeviceProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			fprintf(gpFile, "printVkInfo : Device type = Integrated GPU(iGPU)  \n\n");
			fflush(gpFile);
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			fprintf(gpFile, "printVkInfo : Device type = Discrete GPU(dGPU)  \n\n");
			fflush(gpFile);
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			fprintf(gpFile, "printVkInfo : Device type = Virtual GPU  \n\n");
			fflush(gpFile);
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			fprintf(gpFile, "printVkInfo : Device type = CPU  \n\n");
			fflush(gpFile);
			break;
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			fprintf(gpFile, "printVkInfo : Device type = Other  \n\n");
			fflush(gpFile);
			break;
		default:
			fprintf(gpFile, "printVkInfo : Device type = UNKNOWN  \n\n");
			fflush(gpFile);
			break;
		}

		//vendor ID
		fprintf(gpFile, "printVkInfo : vendorId = 0x%04x  \n\n", vkPhysicalDeviceProperties.vendorID);
		fflush(gpFile);


		//device ID
		fprintf(gpFile, "printVkInfo : deviceId = 0x%04x  \n\n", vkPhysicalDeviceProperties.deviceID);
		fflush(gpFile);


	}
	//free global physical device array
	if (vkPhysicalDevice_array)
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "printVkInfo() : succedded to free vkPhysicalDevice_array \n\n");
		fflush(gpFile);
	}
	return vkResult;
}

VkResult fillDeviceExtensionNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	/* STEP 1 :
	*/

	uint32_t deviceExtensionCount = 0; //local variable

	//Returns properties of available physical device extensions
	/*
	* // Provided by VK_VERSION_1_0
	* VkResult vkEnumerateDeviceExtensionProperties(
	* VkPhysicalDevice                            physicalDevice,
	* const char*                                 pLayerName,
	* uint32_t*                                   pPropertyCount,
	* VkExtensionProperties*                      pProperties);
	*/
	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_Selected, NULL, &deviceExtensionCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillDeviceExtensionNames() : first call to vkEnumerateDeviceExtensionProperties() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "fillDeviceExtensionNames() : first call to vkEnumerateDeviceExtensionProperties() successfull \n\n");
		fflush(gpFile);
	}

	/* STEP 2 :
	*  Allocate and fill struct VkExtensionProperties array corresponding to above count
	*/

	/*
	*	Structure specifying an extension properties				// Provided by VK_VERSION_1_0
	*
	*	typedef struct VkExtensionProperties {
	*	char        extensionName[VK_MAX_EXTENSION_NAME_SIZE];
	*	uint32_t    specVersion;
	*	} VkExtensionProperties;
	*/
	VkExtensionProperties* vkExtensionProperties_array = NULL;

	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);


	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_Selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillDeviceExtensionNames() : second call to vkEnumerateDeviceExtensionProperties() Is Failed \n\n");
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "fillDeviceExtensionNames() : second call to vkEnumerateDeviceExtensionProperties() successfull with deviceExtensionCount = %d\n\n", deviceExtensionCount);
		fflush(gpFile);
	}

	/* STEP 3 :
	*  Fill and display local string array of extension names obtained from VkExtensionProperties array
	*/
	//we dont know the size of array, so here we will use dynamic allocation
	char** deviceExtensionNames_array = NULL;
	deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
	for (uint32_t i = 0; i < deviceExtensionCount; ++i)
	{
		deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1); //+1 for Null terminated char
		memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		fprintf(gpFile, "fillDeviceExtensionNames() : Vulkan device extension name = %s : %d\n\n", deviceExtensionNames_array[i], i);
		fflush(gpFile);
	}

	/* STEP 4 :
	*  Free VkExtensionProperties array as it is not required henceforth
	*/
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;


	/* STEP 5 :
	*  Find whether above extension names contain our required extension
	*/
	// Provided by VK_VERSION_1_0
	//typedef uint32_t VkBool32;
	VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapChainExtensionFound = VK_TRUE;

			enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}

	}

	/* STEP 6 :
	*  Free local string array "deviceExtensionNames_array" as it is not required henceforth
	*/
	for (uint32_t i = 0; i < deviceExtensionCount; ++i)
	{
		free(deviceExtensionNames_array[i]);
		deviceExtensionNames_array[i] = NULL;
	}
	free(deviceExtensionNames_array);
	deviceExtensionNames_array = NULL;

	/* STEP 7 :
	*  Print whether required extensions supported by our vulkan driver or not
	*/
	if (vulkanSwapChainExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;

		fprintf(gpFile, "fillDeviceExtensionNames() : VK_KHR_SWAPCHAIN_EXTENSION_NAME Not Found\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "fillDeviceExtensionNames() : VK_KHR_SWAPCHAIN_EXTENSION_NAME Found\n\n");
		fflush(gpFile);
	}


	/* STEP 8 :
	*  Print Only Enabled extensions name supported by our vulkan driver
	*/
	for (uint32_t i = 0; i < enabledDeviceExtensionCount; ++i)
	{
		fprintf(gpFile, "fillDeviceExtensionNames() : Enabled Vulkan Device Extension Name = %s\n\n", enabledDeviceExtensionNames_array[i]);
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createVulkanDevice(void)
{

	// Function declarations
	VkResult fillDeviceExtensionNames();


	// Variable declarations
	VkResult vkResult = VK_SUCCESS;
	vkResult = fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanDevice() : fillDeviceExtensionNames() is Failed %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVulkanDevice() : fillDeviceExtensionNames() is Successful\n");
		fflush(gpFile);
	}


	//newly added code
	float queuePriorities[1] = { 1.0f };

	//Structure specifying parameters of a newly created device queue
	/*
	* // Provided by VK_VERSION_1_0
	* typedef struct VkDeviceQueueCreateInfo {
	* VkStructureType             sType;
	* const void*                 pNext;
	* VkDeviceQueueCreateFlags    flags;
	* uint32_t                    queueFamilyIndex;
	* uint32_t                    queueCount;
	* const float*                pQueuePriorities;
	* } VkDeviceQueueCreateInfo;
	*/
	// Create queue on selected queue family at the point of device creation
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;
	vkDeviceQueueCreateInfo.pNext = NULL;

	//3. Initialize VkDeviceCreateInfo structure

	//Structure specifying parameters of a newly created device
	/*
	* // Provided by VK_VERSION_1_0
	* typedef struct VkDeviceCreateInfo {
	* VkStructureType                    sType;
	* const void*                        pNext;
	* VkDeviceCreateFlags                flags;
	* uint32_t                           queueCreateInfoCount;
	* const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
	* // enabledLayerCount is deprecated and should not be used
	* uint32_t                           enabledLayerCount;
	* // ppEnabledLayerNames is deprecated and should not be used
	* const char* const*                 ppEnabledLayerNames;
	* uint32_t                           enabledExtensionCount;
	* const char* const*                 ppEnabledExtensionNames;
	* const VkPhysicalDeviceFeatures*    pEnabledFeatures;
	* } VkDeviceCreateInfo;
	*/
	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));


	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;


	//5 Now call vkCreateDevice() vulkan api to create actually vulkan device

	//Create a new device instance
	/*
	* // Provided by VK_VERSION_1_0
	* VkResult vkCreateDevice(
	* VkPhysicalDevice                            physicalDevice,
	* const VkDeviceCreateInfo*                   pCreateInfo,
	* const VkAllocationCallbacks*                pAllocator,
	* VkDevice*                                   pDevice);
	*/
	vkResult = vkCreateDevice(vkPhysicalDevice_Selected, &vkDeviceCreateInfo, NULL, &vkDevice);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanDevice() : vkCreateDevice() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVulkanDevice() : vkCreateDevice() is Successful\n");
		fflush(gpFile);
	}




	return vkResult;

}

void getDeviceQueue()
{
	//code
	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_Selected, 0, &vkQueue);
	if (vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "getDeviceQueue() : vkGetDeviceQueue() is failed\n");
		fflush(gpFile);
	}
	else
	{
		fprintf(gpFile, "getDeviceQueue() : vkGetDeviceQueue() is Succeded\n");
		fflush(gpFile);
	}

	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_Selected, 0, &vkComputeQueue);
	if (vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "getDeviceQueue() : vkGetDeviceQueue() is failed\n");
		fflush(gpFile);
	}
	else
	{
		fprintf(gpFile, "getDeviceQueue() : vkGetDeviceQueue() is Succeded\n");
		fflush(gpFile);
	}
}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//get the count of supported surface color formats
	uint32_t formatCount = 0;

	//Query color formats supported by surface
	/*// Provided by VK_KHR_surface
	* VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(
	* VkPhysicalDevice                            physicalDevice,
	* VkSurfaceKHR                                surface,
	* uint32_t*                                   pSurfaceFormatCount,
	* VkSurfaceFormatKHR*                         pSurfaceFormats);*/
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &formatCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() : 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else if (formatCount == 0)
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR() is failed due to formatCount = 0\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR() is Successful\n");
		fflush(gpFile);
	}

	VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

	//filling the array
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() : 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR() is Successful\n");
		fflush(gpFile);
	}

	//decide surface color format first
	if (formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
	{
		vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		vkFormat_color = vkSurfaceFormatKHR_array[0].format;
	}

	//decide the color space
	vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

	//free the array
	if (vkSurfaceFormatKHR_array)
	{
		free(vkSurfaceFormatKHR_array);
		vkSurfaceFormatKHR_array = NULL;
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkSurfaceFormatKHR_array is Freed\n");
		fflush(gpFile);
	}

	return vkResult;

}

VkResult getPhysicalDevicePresentMode(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//get the count of supported surface color formats
	uint32_t presentModeCount = 0;


	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &presentModeCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode() : 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else if (presentModeCount == 0)
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR() is failed due to formatCount = 0\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR() is Successful\n");
		fflush(gpFile);
	}

	//enum
	VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));

	//filling the array
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode() : 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR() is Successful\n");
		fflush(gpFile);
	}

	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			fprintf(gpFile, "getPhysicalDevicePresentMode() : vkPresentModeKHR is VK_PRESENT_MODE_MAILBOX_KHR.\n");
			fflush(gpFile);
			break;
		}
	}

	if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
		fprintf(gpFile, "getPhysicalDevicePresentMode() : vkPresentModeKHR is VK_PRESENT_MODE_FIFO_KHR.\n");
		fflush(gpFile);
	}
	//free the array
	if (vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;
		fprintf(gpFile, "getPhysicalDevicePresentMode() : vkPresentModeKHR_array is Freed\n");
		fflush(gpFile);

	}

	return vkResult;
}

VkResult createSwapchainHere(VkBool32 vSync)
{
	//function declarations
	VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
	VkResult getPhysicalDevicePresentMode(void);

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code

	//Step 1 - color format and color space
	vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createSwapchain() : getPhysicalDeviceSurfaceFormatAndColorSpace() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createSwapchain() : getPhysicalDeviceSurfaceFormatAndColorSpace() successfull \n\n");
		fflush(gpFile);
	}

	//Step 2 - Get PhysicalDeviceSurfaceCapabilities
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createSwapchain() : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createSwapchain() : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() successfull \n\n");
		fflush(gpFile);
	}

	//step 3 - Find out desired number of swapchain image count
	uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t desiredNumberOfSwapchainImages = 0;

	if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
	{
		fprintf(gpFile, "createSwapchain() : maxImageCount selected\n");
		fflush(gpFile);
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
	}
	else
	{
		fprintf(gpFile, "createSwapchain() : minImageCount Selected\n");
		fflush(gpFile);
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
	}

		fprintf(gpFile, "createSwapchain() : MinImageCount = %d\n maxImageCount = %d\n desiredNumberOfSwapchainImages = %d\n", vkSurfaceCapabilitiesKHR.minImageCount,
		vkSurfaceCapabilitiesKHR.maxImageCount, desiredNumberOfSwapchainImages);
		fflush(gpFile);

	//step 4 - Choose Size of SwapChain image
	memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
	if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
	{

		vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

		fprintf(gpFile, "createSwapchain() : swapchain image width X Height %d X %d\n\n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
		fflush(gpFile);
	}
	else
	{
		//if surface size already defined then swapchain image size must match with it
		VkExtent2D vkExtent2D;
		memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

		vkExtent2D.width = (uint32_t)winWidth;
		vkExtent2D.height = (uint32_t)winHeight;

		vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
	}

	//step Number 5 - set swapchain image usage flag(enum)
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	//step No 6 - whether to consider pre transform/flipping or not (enum)
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
	if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
	}

	//step 7 - presentation Mode
	vkResult = getPhysicalDevicePresentMode();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createSwapchain() : getPhysicalDevicePresentMode() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createSwapchain() : getPhysicalDevicePresentMode() successfull \n\n");
		fflush(gpFile);
	}

	//step 8 - initialize vkCreateSwapChainInfo structure
	VkSwapchainCreateInfoKHR vkSwapChainCreateInfoKHR;
	memset((void*)&vkSwapChainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
	vkSwapChainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapChainCreateInfoKHR.pNext = NULL;
	vkSwapChainCreateInfoKHR.flags = 0;
	vkSwapChainCreateInfoKHR.surface = vkSurfaceKHR;
	vkSwapChainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
	vkSwapChainCreateInfoKHR.imageFormat = vkFormat_color;
	vkSwapChainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
	vkSwapChainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
	vkSwapChainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
	vkSwapChainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapChainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	vkSwapChainCreateInfoKHR.imageArrayLayers = 1;
	vkSwapChainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //swapchain multiple queues madhe shared karaychya ki nai?
	vkSwapChainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapChainCreateInfoKHR.presentMode = vkPresentModeKHR;
	vkSwapChainCreateInfoKHR.clipped = VK_TRUE;

	vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapChainCreateInfoKHR, NULL, &vkSwapchainKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createSwapchain() : vkCreateSwapchainKHR() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createSwapchain() : vkCreateSwapchainKHR() successfull \n\n");
		fflush(gpFile);
	}
	return vkResult;

}

VkResult createImagesAndImageViews()
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapChainImageCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createImagesAndImageViews() : 1st call to vkGetSwapChainImagesKHR() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else if (swapChainImageCount == 0)
	{
		fprintf(gpFile, "createImagesAndImageViews() : vkGetSwapChainImagesKHR() is failed due to formatCount = 0\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createImagesAndImageViews() : vkGetSwapChainImagesKHR() gives swapchainImageCount  = %d \n", swapChainImageCount);
		fflush(gpFile);
	}

	//step 2
	//allocate swapChainImageArray
	swapChainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapChainImageCount);

	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapChainImageCount, swapChainImage_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createImagesAndImageViews() : 2nd call to vkGetSwapChainImagesKHR() is failed with result = %d\n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createImagesAndImageViews() : vkGetSwapChainImagesKHR() gives swapchainImageCount  = %d \n", swapChainImageCount);
		fflush(gpFile);
	}

	//step 4
	//Allocate swapChainImageView_array
	swapChainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapChainImageCount);

	//step 5
	//Init VkImageViewCreateInfo Struct
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = vkFormat_color;
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R; // VkComponentSwizzle
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //VkImageSubresourceRange 
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0; //kuthlya mip level pasun chalu karu
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	//step 6
	//fill imageViewArray by using above array

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkImageViewCreateInfo.image = swapChainImage_array[i];

		vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapChainImageView_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createImagesAndImageViews() : vkCreateImageView() is failed with result = %d of i = %d\n", vkResult, i);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createImagesAndImageViews() : vkGetSwapChainImagesKHR() gives swapchainImageCount  = %d of i = %d\n", swapChainImageCount, i);
			fflush(gpFile);
		}
	}


	return vkResult;
}

VkResult createCommandPool(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
	memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

	vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext = NULL;
	vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//can be resetted and restarted (others are transfer)
	vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;

	vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createCommandPool() : createCommandPool() Is failed due to %d \n\n", vkResult);
		fflush(gpFile);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "createCommandPool() : createCommandPool() successfull \n\n");
		fflush(gpFile);
	}


	return vkResult;
}

VkResult createComputeCommandBuffers(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	//step no 1
	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //secondary commandBuffer can be called from primary command buffers
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	//2
	vkComputeCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapChainImageCount);

	//3
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkComputeCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createComputeCommandBuffers() : vkAllocateCommandBuffers() is failed with result = %d of i = %d\n", vkResult, i);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createComputeCommandBuffers() : vkAllocateCommandBuffers() gives swapchainImageCount  = %d of i = %d\n", swapChainImageCount, i);
			fflush(gpFile);
		}
	}
	return vkResult;
}

VkResult createCommandBuffers(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	//step no 1
	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //secondary commandBuffer can be called from primary command buffers
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	//2
	vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapChainImageCount);

	//3
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createCommandBuffers() : vkAllocateCommandBuffers() is failed with result = %d of i = %d\n", vkResult, i);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createCommandBuffers() : vkAllocateCommandBuffers() gives swapchainImageCount  = %d of i = %d\n", swapChainImageCount, i);
			fflush(gpFile);
		}
	}
	return vkResult;
}

VkResult createShaderStorageBuffers()
{
	VkResult vkResult = VK_SUCCESS;
	VertexData vertexData_stagingBuffer_position;

	fprintf(gpFile, "createShaderStorageBuffers() : createShaderStorageBuffers() is successfull\n");
	fflush(gpFile);
	// Initialize particles
	std::default_random_engine rndEngine((unsigned)time(nullptr));
	std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

	 // Initial particle positions on a circle
	

	for (auto& particle : particles) {
		float r = 0.25f * sqrt(rndDist(rndEngine));
		float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
		float x = r * cos(theta) * winHeight / winWidth;
		float y = r * sin(theta);

		particle.position = glm::vec2(x, y);
		particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
		particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
		fprintf(gpFile, "particle.position : (%f, %f, particle.velocity = (%f, %f), color=(%f,%f,%f,%f) \n", particle.position.x, particle.position.y, particle.velocity.x, particle.velocity.y, 
			particle.color.x, particle.color.y, particle.color.z, particle.color.a);
		fflush(gpFile);
	}

	/*for (auto& particle : particles) {

		particle.position = glm::vec2(1, 1);
		particle.velocity = glm::normalize(glm::vec2(1, 0)) * 0.00025f;
		particle.color = glm::vec4(1.0f);
	}*/
	VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

	memset((void*)&vertexData_Position_SSBO, 0, sizeof(VertexData) * _ARRAYSIZE(vertexData_Position_SSBO));
	//vertexData_Position_SSBO.resize(swapChainImageCount);
	for (size_t i = 0; i < swapChainImageCount; i++)
	{
		VkBufferCreateInfo vkBufferCreateInfo;
		memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

		vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vkBufferCreateInfo.pNext = NULL;
		vkBufferCreateInfo.flags = 0; //valid flags used in scatter buffer(sparse Buffer)
		vkBufferCreateInfo.size = bufferSize;
		vkBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		//for staging buffer =>usage = transfer_SRC

		vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_Position_SSBO[i].vkBuffer);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkCreateBuffer() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkCreateBuffer() is successfull\n");
			fflush(gpFile);
		}

		VkMemoryRequirements vkMemoryRequirements;
		memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

		vkGetBufferMemoryRequirements(vkDevice, vertexData_Position_SSBO[i].vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
			&vkMemoryRequirements);

		VkMemoryAllocateInfo vkMemoryAllocateInfo;
		memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

		vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkMemoryAllocateInfo.pNext = NULL;
		vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //allocate zalelya chunk chi size
		vkMemoryAllocateInfo.memoryTypeIndex = 0; //initial value before entering into loop
		for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
			{
				if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)  //staging buffer | COHERENT
				{
					vkMemoryAllocateInfo.memoryTypeIndex = i;
					break;
				}
			}
			vkMemoryRequirements.memoryTypeBits >>= 1;
		}

		vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_Position_SSBO[i].vkDeviceMemory);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkAllocateMemory() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkAllocateMemory() is successfull\n");
			fflush(gpFile);
		}

		vkResult = vkBindBufferMemory(vkDevice, vertexData_Position_SSBO[i].vkBuffer, // kunala bind karaychi
			vertexData_Position_SSBO[i].vkDeviceMemory, 0 //kon bind karaychi
			//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
		);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkBindBufferMemory() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkBindBufferMemory() is successfull\n");
			fflush(gpFile);
		}

		void* data = NULL;
		vkResult = vkMapMemory(vkDevice, vertexData_Position_SSBO[i].vkDeviceMemory, //konati map karaychi
			0, //starting offset
			vkMemoryAllocateInfo.allocationSize,//till where should I map
			0,// reserved
			&data);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkMapMemory() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createShaderStorageBuffers() : vkMapMemory() is successfull\n");
			fflush(gpFile);
		}
		//data ha deviceMemory la map ahe
		memcpy(data, particles.data(), bufferSize);
		//this memory is integral memory (padding kel jat vulkan driver kadun)//allocation region madhe milte

		vkUnmapMemory(vkDevice, vertexData_Position_SSBO[i].vkDeviceMemory //konati map karaychi
		);
	}
	// Create a staging buffer used to upload data to the gpu

//createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	//for (size_t i = 0; i < swapChainImageCount; i++)
	//{
	//	VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
	//	memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

	//	vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//	vkBufferCreateInfo_stagingBuffer.pNext = NULL;
	//	vkBufferCreateInfo_stagingBuffer.flags = 0;
	//	vkBufferCreateInfo_stagingBuffer.size = bufferSize;
	//	vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; ///ha data transfer karaycha source Buffer aahe
	//	vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//	//you can use this buffer for concurrent (multi threading sathi) vapru shakta

	//	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBuffer_position.vkBuffer);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : staging Buffer vkCreateBuffer() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : staging Buffer vkCreateBuffer() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
	//	memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));

	//	vkGetBufferMemoryRequirements(vkDevice, vertexData_stagingBuffer_position.vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
	//		&vkMemoryRequirements_stagingBuffer);

	//	VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
	//	memset((void*)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));

	//	vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	//	vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
	//	vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size; //allocate zalelya chunk chi size
	//	vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0; //initial value before entering into loop

	//	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	//	{
	//		if ((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
	//		{
	//			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //staging buffer | COHERENT
	//			{
	//				vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
	//				break;
	//			}
	//		}
	//		vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
	//	}

	//	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vertexData_stagingBuffer_position.vkDeviceMemory);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer vkAllocateMemory() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer vkAllocateMemory() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBuffer_position.vkBuffer, // kunala bind karaychi
	//		vertexData_stagingBuffer_position.vkDeviceMemory, 0);//kon bind karaychi
	//	//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer vkBindBufferMemory() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer vkBindBufferMemory() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	void* data = NULL;
	//	vkResult = vkMapMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory, //konati map karaychi
	//		0, //starting offset
	//		vkMemoryAllocateInfo_stagingBuffer.allocationSize,//till where should I map
	//		0,// reserved
	//		&data);

	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer vkMapMemory() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer vkMapMemory() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	//data ha deviceMemory la map ahe
	//	memcpy(data, particles.data(), bufferSize);
	//	//this memory is integral memory (padding kel jat vulkan driver kadun)//allocation region madhe milte

	//	vkUnmapMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory //konati map karaychi
	//	);
	//	fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer ********************** is successfull\n");
	//	fflush(gpFile);
	//	//***********************************
	//		//STEP 2 : Device Visible
	//	//memset((void*)&vertexData_Position_SSBO, 0, sizeof(VertexData));
	//	vertexData_Position_SSBO.resize(swapChainImageCount);

	//	fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer !!!!!!!!!!!!!!!!!######################### is successfull\n");
	//	fflush(gpFile);  
	//	std::vector<VkCommandBuffer> vkCommandBuffer;
	//	vkCommandBuffer.resize(swapChainImageCount);

	//	fprintf(gpFile, "createShaderStorageBuffers() : stagingBuffer ######################### is successfull\n");
	//	fflush(gpFile);
	//	
	//	VkBufferCreateInfo vkBufferCreateInfo;
	//	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	//	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//	vkBufferCreateInfo.pNext = NULL;
	//	vkBufferCreateInfo.flags = 0; //valid flags used in scatter buffer(sparse Buffer)
	//	vkBufferCreateInfo.size = bufferSize;
	//	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; //(Used for starage buffer also)
	//	vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	//	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_Position_SSBO[i].vkBuffer);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : vkCreateBuffer() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : vkCreateBuffer() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	VkMemoryRequirements vkMemoryRequirements;
	//	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	//	vkGetBufferMemoryRequirements(vkDevice, vertexData_Position_SSBO[i].vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
	//		&vkMemoryRequirements);

	//	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	//	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	//	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	//	vkMemoryAllocateInfo.pNext = NULL;
	//	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //allocate zalelya chunk chi size
	//	vkMemoryAllocateInfo.memoryTypeIndex = 0; //initial value before entering into loop

	//	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	//	{
	//		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
	//		{
	//			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //staging buffer | COHERENT
	//			{
	//				vkMemoryAllocateInfo.memoryTypeIndex = i;
	//				break;
	//			}
	//		}
	//		vkMemoryRequirements.memoryTypeBits >>= 1;
	//	}
	//	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_Position_SSBO[i].vkDeviceMemory);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : vkAllocateMemory() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : vkAllocateMemory() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	vkResult = vkBindBufferMemory(vkDevice, vertexData_Position_SSBO[i].vkBuffer, // kunala bind karaychi
	//		vertexData_Position_SSBO[i].vkDeviceMemory, 0 //kon bind karaychi
	//		//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
	//	);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : vkBindBufferMemory() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : vkBindBufferMemory() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	//no need of map memory, memcpy, unmp memory for this device visible buffer
	//	//******************************************************
	//	//STEP 3
	//	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	//	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	//	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//	vkCommandBufferAllocateInfo.pNext = NULL;
	//	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	//	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //secondary commandBuffer can be called from primary command buffers
	//	vkCommandBufferAllocateInfo.commandBufferCount = 1;



	//	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer[i]);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY : vkAllocateCommandBuffers() is failed with result = %d\n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY : vkAllocateCommandBuffers() gives swapchainImageCount  = %d \n", swapChainImageCount);
	//		fflush(gpFile);
	//	}

	//	//buildCommandBuffer
	//	VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
	//	memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

	//	vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//	vkCommandBufferBeginInfo.pNext = NULL;
	//	//why we havent done reset command Buffer?
	//	vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //will only primary command buffer, not going to use simultaneously between multiple threads

	//	vkResult = vkBeginCommandBuffer(vkCommandBuffer[i], &vkCommandBufferBeginInfo);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY : vkBeginCommandBuffer() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY : vkBeginCommandBuffer() is successfull\n");
	//		fflush(gpFile);
	//	}


	//	VkBufferCopy vkBufferCopy;
	//	memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));

	//	vkBufferCopy.srcOffset = 0;
	//	vkBufferCopy.dstOffset = 0;
	//	vkBufferCopy.size = sizeof(bufferSize);


	//	vkCmdCopyBuffer(vkCommandBuffer[i], vertexData_stagingBuffer_position.vkBuffer, vertexData_Position_SSBO[i].vkBuffer, 1 //number of regions
	//		, &vkBufferCopy); //3,4,6,9 chapter of BAK
	//	//End command Buffer Record
	//	vkResult = vkEndCommandBuffer(vkCommandBuffer[i]);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY : vkEndCommandBuffer() is failed with result = %d \n", vkResult);
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY : vkEndCommandBuffer() is successfull\n");
	//		fflush(gpFile);
	//	}

	//	//Queue madhe submit karaycha ahe, submit above command buffer
	//	//declare , memset and initialize VkSubmitInfo Structure
	//	VkSubmitInfo vkSubmitInfo;
	//	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	//	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//	vkSubmitInfo.pNext = NULL;
	//	vkSubmitInfo.commandBufferCount = 1;
	//	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer[i];

	//	//waitDstStageMask chi garaj nai karan, ithe ek ch ahe, wait karaychi garaj naiye

	//	//now submit above work to the queue
	//	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY :vkQueueSubmit() failed\n\n");
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY :vkQueueSubmit() Success\n\n");
	//		fflush(gpFile);
	//	}

	//	vkResult = vkQueueWaitIdle(vkQueue);
	//	if (vkResult != VK_SUCCESS)
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY :vkQueueWaitIdle() failed\n\n");
	//		fflush(gpFile);
	//		return vkResult;
	//	}
	//	else
	//	{
	//		fprintf(gpFile, "createShaderStorageBuffers() : BufferCOPY :vkQueueWaitIdle() Success\n\n");
	//		fflush(gpFile);
	//	}

	//	
	//	if (vkCommandBuffer[i])
	//	{
	//		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer[i]);
	//		fprintf(gpFile, "\n createShaderStorageBuffers() : vkFreeCommandBuffers() of is Done\n");
	//		fflush(gpFile);
	//	}
	//	
	//
	//	if (vertexData_stagingBuffer_position.vkDeviceMemory)
	//	{
	//		vkFreeMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory, NULL);
	//		vertexData_stagingBuffer_position.vkDeviceMemory = VK_NULL_HANDLE;
	//		fprintf(gpFile, "\n createShaderStorageBuffers() : vkFreeMemory() is Done\n");
	//		fflush(gpFile);
	//	}
	//	if (vertexData_stagingBuffer_position.vkBuffer)
	//	{
	//		vkDestroyBuffer(vkDevice, vertexData_stagingBuffer_position.vkBuffer, NULL);
	//		vertexData_stagingBuffer_position.vkBuffer = VK_NULL_HANDLE;
	//		fprintf(gpFile, "\n createShaderStorageBuffers() : vkDestroyBuffer() is Done\n");
	//		fflush(gpFile);
	//	}
	//}
	//

	
	return vkResult;
}

VkResult createVertexBuffer()
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//staging buffer code

	//step1
	VertexData vertexData_stagingBuffer_position;

	float triangle_Position[] =
	{
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};
	float triangle_Color[] =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	//**************************************************Vertex PositionBuffer****************************

	memset((void*)&vertexData_stagingBuffer_position, 0, sizeof(VertexData));

	VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
	memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo_stagingBuffer.pNext = NULL;
	vkBufferCreateInfo_stagingBuffer.flags = 0;
	vkBufferCreateInfo_stagingBuffer.size = sizeof(triangle_Position);
	vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; ///ha data transfer karaycha source Buffer aahe
	vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//you can use this buffer for concurrent (multi threading sathi) vapru shakta

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBuffer_position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : staging Buffer vkCreateBuffer() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : staging Buffer vkCreateBuffer() is successfull\n");
	}

	VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
	memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_stagingBuffer_position.vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
		&vkMemoryRequirements_stagingBuffer);

	VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
	memset((void*)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
	vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size; //allocate zalelya chunk chi size
	vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0; //initial value before entering into loop

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //staging buffer | COHERENT
			{
				vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vertexData_stagingBuffer_position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : stagingBuffer vkAllocateMemory() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : stagingBuffer vkAllocateMemory() is successfull\n");
	}

	vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBuffer_position.vkBuffer, // kunala bind karaychi
		vertexData_stagingBuffer_position.vkDeviceMemory, 0);//kon bind karaychi
	//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : stagingBuffer vkBindBufferMemory() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : stagingBuffer vkBindBufferMemory() is successfull\n");
	}

	void* data = NULL;
	vkResult = vkMapMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory, //konati map karaychi
		0, //starting offset
		vkMemoryAllocateInfo_stagingBuffer.allocationSize,//till where should I map
		0,// reserved
		&data);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : stagingBuffer vkMapMemory() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : stagingBuffer vkMapMemory() is successfull\n");
	}

	//data ha deviceMemory la map ahe
	memcpy(data, triangle_Position, sizeof(triangle_Position));
	//this memory is integral memory (padding kel jat vulkan driver kadun)//allocation region madhe milte

	vkUnmapMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory //konati map karaychi
	);

	//***********************************
	//STEP 2 : Device Visible
	memset((void*)&vertexData_Position, 0, sizeof(VertexData));

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags used in scatter buffer(sparse Buffer)
	vkBufferCreateInfo.size = sizeof(triangle_Position);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_Position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkCreateBuffer() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkCreateBuffer() is successfull\n");
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_Position.vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
		&vkMemoryRequirements);

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //allocate zalelya chunk chi size
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //initial value before entering into loop

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //staging buffer | COHERENT
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_Position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkAllocateMemory() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkAllocateMemory() is successfull\n");
	}

	vkResult = vkBindBufferMemory(vkDevice, vertexData_Position.vkBuffer, // kunala bind karaychi
		vertexData_Position.vkDeviceMemory, 0 //kon bind karaychi
		//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
	);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkBindBufferMemory() is failed with result = %d \n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkBindBufferMemory() is successfull\n");
	}

	//no need of map memory, memcpy, unmp memory for this device visible buffer

	//******************************************************
	//STEP 3
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //secondary commandBuffer can be called from primary command buffers
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY : vkAllocateCommandBuffers() is failed with result = %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY : vkAllocateCommandBuffers() gives swapchainImageCount  = %d \n", swapChainImageCount);
	}

	//buildCommandBuffer
	VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
	memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

	vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo.pNext = NULL;
	//why we havent done reset command Buffer?
	vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //will only primary command buffer, not going to use simultaneously between multiple threads

	vkResult = vkBeginCommandBuffer(vkCommandBuffer, &vkCommandBufferBeginInfo);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY : vkBeginCommandBuffer() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY : vkBeginCommandBuffer() is successfull\n");
		fflush(gpFile);
	}

	/*// Provided by VK_VERSION_1_0
		typedef struct VkBufferCopy {
		VkDeviceSize    srcOffset;
		VkDeviceSize    dstOffset;
		VkDeviceSize    size;
		} VkBufferCopy;*/
	VkBufferCopy vkBufferCopy;
	memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));

	vkBufferCopy.srcOffset = 0;
	vkBufferCopy.dstOffset = 0;
	vkBufferCopy.size = sizeof(triangle_Position);

	/*// Provided by VK_VERSION_1_0
void vkCmdCopyBuffer(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    srcBuffer,
	VkBuffer                                    dstBuffer,
	uint32_t                                    regionCount,
	const VkBufferCopy*                         pRegions);*/
	vkCmdCopyBuffer(vkCommandBuffer, vertexData_stagingBuffer_position.vkBuffer, vertexData_Position.vkBuffer, 1 //number of regions
		, &vkBufferCopy); //3,4,6,9 chapter of BAK
	//End command Buffer Record
	vkResult = vkEndCommandBuffer(vkCommandBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY : vkEndCommandBuffer() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY : vkEndCommandBuffer() is successfull\n");
		fflush(gpFile);
	}

	//Queue madhe submit karaycha ahe, submit above command buffer
	//declare , memset and initialize VkSubmitInfo Structure
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	//vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask; //stage to stage wait -> barrier
	//vkSubmitInfo.waitSemaphoreCount = 1;
	//vkSubmitInfo.pWaitSemaphores = &vkSamaphore_backBuffer; //kunasathi thambaych
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer;
	//vkSubmitInfo.signalSemaphoreCount = 1;
	//vkSubmitInfo.pSignalSemaphores = &vkSamaphore_renderComplete;
	//No synchronization 
	//waitDstStageMask chi garaj nai karan, ithe ek ch ahe, wait karaychi garaj naiye

	//now submit above work to the queue
	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY :vkQueueSubmit() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY :vkQueueSubmit() Success\n\n");
		fflush(gpFile);
	}

	vkResult = vkQueueWaitIdle(vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY :vkQueueWaitIdle() failed\n\n");
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : BufferCOPY :vkQueueWaitIdle() Success\n\n");
		fflush(gpFile);
	}

	if (vkCommandBuffer)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer);
		fprintf(gpFile, "\n createVertexBuffer() : vkFreeCommandBuffers() of is Done\n");
		fflush(gpFile);
	}

	if (vertexData_stagingBuffer_position.vkDeviceMemory)
	{
		vkFreeMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory, NULL);
		vertexData_stagingBuffer_position.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(gpFile, "\n createVertexBuffer() : vkFreeMemory() is Done\n");
		fflush(gpFile);
	}
	if (vertexData_stagingBuffer_position.vkBuffer)
	{
		vkDestroyBuffer(vkDevice, vertexData_stagingBuffer_position.vkBuffer, NULL);
		vertexData_stagingBuffer_position.vkBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "\n createVertexBuffer() : vkDestroyBuffer() is Done\n");
		fflush(gpFile);
	}

	//**************************************************Position Color Buffer****************************
	memset((void*)&vertexData_Color, 0, sizeof(VertexData));


	//VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags used in scatter buffer(sparse Buffer)
	vkBufferCreateInfo.size = sizeof(triangle_Color);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; //Most Important change for index Buffer
	//for staging buffer =>usage = transfer_SRC

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_Color.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkCreateBuffer() for colorBuffer is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkCreateBuffer() for colorBuffer is successfull\n");
		fflush(gpFile);
	}

	//VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_Color.vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
		&vkMemoryRequirements);

	//VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //allocate zalelya chunk chi size
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //initial value before entering into loop
	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) //staging buffer | COHERENT
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_Color.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkAllocateMemory() for colorBuffer is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkAllocateMemory() for colorBuffer is successfull\n");
		fflush(gpFile);
	}

	vkResult = vkBindBufferMemory(vkDevice, vertexData_Color.vkBuffer, // kunala bind karaychi
		vertexData_Color.vkDeviceMemory, 0 //kon bind karaychi
		//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
	);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkBindBufferMemory() for colorBuffer is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkBindBufferMemory() for colorBuffer is successfull\n");
		fflush(gpFile);
	}


	vkResult = vkMapMemory(vkDevice, vertexData_Color.vkDeviceMemory, //konati map karaychi
		0, //starting offset
		vkMemoryAllocateInfo.allocationSize,//till where should I map
		0,// reserved
		&data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() : vkMapMemory() for colorBuffer is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() : vkMapMemory() for colorBuffer is successfull\n");
		fflush(gpFile);
	}
	//data ha deviceMemory la map ahe
	memcpy(data, triangle_Color, sizeof(triangle_Color));
	//this memory is integral memory (padding kel jat vulkan driver kadun)//allocation region madhe milte

	vkUnmapMemory(vkDevice, vertexData_Color.vkDeviceMemory //konati map karaychi
	);

	return vkResult;
}

VkResult createUniformBuffer()
{
	//function declaration
	VkResult updateUniformBuffer(void);


	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code

	memset((void*)&uniformData, 0, sizeof(UniformData));

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags used in scatter buffer(sparse Buffer)
	vkBufferCreateInfo.size = sizeof(MyUniformData);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() : vkCreateBuffer() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() : vkCreateBuffer() is successfull\n");
		fflush(gpFile);
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
		&vkMemoryRequirements);

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //allocate zalelya chunk chi size
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //initial value before entering into loop
	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() : vkAllocateMemory() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() : vkAllocateMemory() is successfull\n");
		fflush(gpFile);
	}

	vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, // kunala bind karaychi
		uniformData.vkDeviceMemory, 0 //kon bind karaychi
		//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
	);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() : vkBindBufferMemory() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() : vkBindBufferMemory() is successfull\n");
		fflush(gpFile);
	}

	//call updateUniformBuffer
	vkResult = updateUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() : updateUniformBuffer() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() : updateUniformBuffer() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createComputeUniformBuffer()
{
	//function declaration
	VkResult updateComputeUniformBuffer(void);


	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	computeUniformData.resize(swapChainImageCount);

	//code
	for (size_t i = 0; i < swapChainImageCount; i++)
	{
		memset((void*)&computeUniformData[i], 0, sizeof(UniformData));

		VkBufferCreateInfo vkBufferCreateInfo;
		memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

		vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vkBufferCreateInfo.pNext = NULL;
		vkBufferCreateInfo.flags = 0; //valid flags used in scatter buffer(sparse Buffer)
		vkBufferCreateInfo.size = sizeof(MyComputeUniformData);
		vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &computeUniformData[i].vkBuffer);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createComputeUniformBuffer() : vkCreateBuffer() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createComputeUniformBuffer() : vkCreateBuffer() is successfull\n");
			fflush(gpFile);
		}

		VkMemoryRequirements vkMemoryRequirements;
		memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

		vkGetBufferMemoryRequirements(vkDevice, computeUniformData[i].vkBuffer, //kunakde mahiti ahe kiti memory lagnar ti
			&vkMemoryRequirements);

		VkMemoryAllocateInfo vkMemoryAllocateInfo;
		memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

		vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkMemoryAllocateInfo.pNext = NULL;
		vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //allocate zalelya chunk chi size
		vkMemoryAllocateInfo.memoryTypeIndex = 0; //initial value before entering into loop
		for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
			{
				if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
				{
					vkMemoryAllocateInfo.memoryTypeIndex = i;
					break;
				}
			}
			vkMemoryRequirements.memoryTypeBits >>= 1;
		}

		vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &computeUniformData[i].vkDeviceMemory);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createComputeUniformBuffer() : vkAllocateMemory() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createComputeUniformBuffer() : vkAllocateMemory() is successfull\n");
			fflush(gpFile);
		}

		vkResult = vkBindBufferMemory(vkDevice, computeUniformData[i].vkBuffer, // kunala bind karaychi
			computeUniformData[i].vkDeviceMemory, 0 //kon bind karaychi
			//it binds vulkan device Memory Object handle with vulkan Buffer Object handle
		);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createComputeUniformBuffer() : vkBindBufferMemory() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createComputeUniformBuffer() : vkBindBufferMemory() is successfull\n");
			fflush(gpFile);
		}
	}
	//call updateUniformBuffer
	vkResult = updateComputeUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createComputeUniformBuffer() : updateUniformBuffer() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createComputeUniformBuffer() : updateUniformBuffer() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult updateComputeUniformBuffer(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//myUniformData
	MyComputeUniformData myComputeUniformData;
	memset((void*)&myComputeUniformData, 0, sizeof(MyComputeUniformData));

	//update matrices

	myComputeUniformData.deltaTime = lastFrameTime * 2.0f;
	/*UniformBufferObject ubo{};
	ubo.deltaTime = lastFrameTime * 2.0f;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));*/


	//map uniform buffer
	std::vector<void*> data;
	data.resize(swapChainImageCount);

	for (size_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkMapMemory(vkDevice, computeUniformData[i].vkDeviceMemory, //konati map karaychi
			0, //starting offset
			sizeof(MyComputeUniformData),//till where should I map
			0,// reserved
			&data[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "updateComputeUniformBuffer() : vkMapMemory() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}

		//data ha deviceMemory la map ahe
		memcpy(data[i], &myComputeUniformData, sizeof(MyComputeUniformData));
		//this memory is integral memory (padding kel jat vulkan driver kadun)//allocation region madhe milte

		vkUnmapMemory(vkDevice, computeUniformData[i].vkDeviceMemory //konati map karaychi
		);
	}

	return vkResult;
}

VkResult updateUniformBuffer(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//myUniformData
	MyUniformData myUniformData;
	memset((void*)&myUniformData, 0, sizeof(MyUniformData));

	//update matrices

	myUniformData.modelMatrix = glm::mat4(1.0);
	myUniformData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

	myUniformData.viewMatrix = glm::mat4(1.0);


	glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0);
	perspectiveProjectionMatrix = glm::perspective(45.0f, (float)winWidth / (float)winHeight, 0.1f, 100.0f);
	perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * (-1.0f); //corrected way
	myUniformData.projectionMatrix = perspectiveProjectionMatrix;

	//map uniform buffer
	void* data = NULL;
	vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, //konati map karaychi
		0, //starting offset
		sizeof(MyUniformData),//till where should I map
		0,// reserved
		&data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "updateUniformBuffer() : vkMapMemory() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}

	//data ha deviceMemory la map ahe
	memcpy(data, &myUniformData, sizeof(MyUniformData));
	//this memory is integral memory (padding kel jat vulkan driver kadun)//allocation region madhe milte

	vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory //konati map karaychi
	);

	return vkResult;
}

VkResult createComputeDescriptorSetLayout(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//initialize DescriptorSetBinding

	/*VkAttachmentDescription vkAttachmentDescription_array[1];
	memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));*/

	VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[3];
	memset((void*)vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));
	vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkDescriptorSetLayoutBinding_array[0].binding = 0; //binding point
	vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1;
	vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL;//samplers across the shaders thevayche astat

	vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vkDescriptorSetLayoutBinding_array[1].binding = 1; //binding point
	vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1;
	vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL;//samplers across the shaders thevayche astat

	vkDescriptorSetLayoutBinding_array[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vkDescriptorSetLayoutBinding_array[2].binding = 2; //binding point
	vkDescriptorSetLayoutBinding_array[2].descriptorCount = 1;
	vkDescriptorSetLayoutBinding_array[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vkDescriptorSetLayoutBinding_array[2].pImmutableSamplers = NULL;//samplers across the shaders thevayche astat

	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0;
	vkDescriptorSetLayoutCreateInfo.bindingCount = 3; //integer value where you want to bind descriptor sets
	vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;	//to give uniform

	vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkComputeDescriptorSetLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorSetLayout() : vkCreateDescriptorSetLayout() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createDescriptorSetLayout() : vkCreateDescriptorSetLayout() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createDescriptorSetLayout(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0;
	vkDescriptorSetLayoutCreateInfo.bindingCount = 0; //integer value where you want to bind descriptor sets
	vkDescriptorSetLayoutCreateInfo.pBindings = NULL;

	vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorSetLayout() : vkCreateDescriptorSetLayout() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createDescriptorSetLayout() : vkCreateDescriptorSetLayout() is successfull\n");
		fflush(gpFile);
	}

	//initialize DescriptorSetBinding
	/*// Provided by VK_VERSION_1_0
typedef struct VkDescriptorSetLayoutBinding {
	uint32_t              binding;
	VkDescriptorType      descriptorType;
	uint32_t              descriptorCount;
	VkShaderStageFlags    stageFlags;
	const VkSampler*      pImmutableSamplers;
} VkDescriptorSetLayoutBinding;*/
//VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
//memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
//vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//vkDescriptorSetLayoutBinding.binding = 0; //binding point
//vkDescriptorSetLayoutBinding.descriptorCount = 1;
//vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;//samplers across the shaders thevayche astat

//VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
//memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

//vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//vkDescriptorSetLayoutCreateInfo.pNext = NULL;
//vkDescriptorSetLayoutCreateInfo.flags = 0;
//vkDescriptorSetLayoutCreateInfo.bindingCount = 1; //integer value where you want to bind descriptor sets
//vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;	//to give uniform
//
//vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
//if (vkResult != VK_SUCCESS)
//{
//	fprintf(gpFile, "createDescriptorSetLayout() : vkCreateDescriptorSetLayout() is failed with result = %d \n", vkResult);
//	return vkResult;
//}
//else
//{
//	fprintf(gpFile, "createDescriptorSetLayout() : vkCreateDescriptorSetLayout() is successfull\n");
//}

	return vkResult;
}

VkResult createComputePipelineLayout(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.flags = 0;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.setLayoutCount = 1;
	vkPipelineLayoutCreateInfo.pSetLayouts = &vkComputeDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkComputePipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createPipelineLayout() : vkCreatePipelineLayout() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createPipelineLayout() : vkCreatePipelineLayout() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createPipelineLayout(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.flags = 0;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.setLayoutCount = 0;
	vkPipelineLayoutCreateInfo.pSetLayouts = nullptr;
	//vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createPipelineLayout() : vkCreatePipelineLayout() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createPipelineLayout() : vkCreatePipelineLayout() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createDescriptorPool(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//before creating actual descriptor pool, vulkan expect descriptor Pool Size
	VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
	memset((void*)&vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));

	/*// Provided by VK_VERSION_1_0
typedef struct VkDescriptorPoolSize {
	VkDescriptorType    type;
	uint32_t            descriptorCount;
} VkDescriptorPoolSize;*/
	vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkDescriptorPoolSize_array[0].descriptorCount = static_cast<uint32_t>(swapChainImageCount);

	vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vkDescriptorPoolSize_array[1].descriptorCount = static_cast<uint32_t>(swapChainImageCount) * 2;
	//no of descripors sathi chi mahit ahe

	/*// Provided by VK_VERSION_1_0
typedef struct VkDescriptorPoolCreateInfo {
	VkStructureType                sType;
	const void*                    pNext;
	VkDescriptorPoolCreateFlags    flags;
	uint32_t                       maxSets;
	uint32_t                       poolSizeCount;
	const VkDescriptorPoolSize*    pPoolSizes;
} VkDescriptorPoolCreateInfo;*/
	VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
	memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

	vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	vkDescriptorPoolCreateInfo.flags = 0;
	vkDescriptorPoolCreateInfo.pNext = NULL;
	vkDescriptorPoolCreateInfo.poolSizeCount = 2;
	vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;
	vkDescriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(swapChainImageCount);//Important (kiti sets pahijet)

	vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorPool() : vkCreateDescriptorPool() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createDescriptorPool() : vkCreateDescriptorPool() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createComputeDescriptorSet(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	std::vector<VkDescriptorSetLayout> layouts(swapChainImageCount, vkComputeDescriptorSetLayout);

	//initialize descriptorSetAllocationInfo
	VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
	memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

	vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	vkDescriptorSetAllocateInfo.pNext = NULL; //dynamic karanyasathi
	vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
	vkDescriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImageCount);
	vkDescriptorSetAllocateInfo.pSetLayouts = layouts.data();

	//fix function sarkh asel tar flags nasto
	vkComputeDescriptorSets.resize(swapChainImageCount);
	vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, vkComputeDescriptorSets.data());
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createComputeDescriptorSet() : vkAllocateDescriptorSets() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createComputeDescriptorSet() : vkAllocateDescriptorSets() is successfull\n");
		fflush(gpFile);
	}

	computeUniformData.resize(swapChainImageCount);
	//describe whether we want buffer as uniform or image as uniform
	for (size_t i = 0; i < swapChainImageCount; i++)
	{

		VkDescriptorBufferInfo vkDescriptorBufferInfo;
		memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
		vkDescriptorBufferInfo.buffer = computeUniformData[i].vkBuffer;
		vkDescriptorBufferInfo.offset = 0;
		vkDescriptorBufferInfo.range = sizeof(MyComputeUniformData);

		//we will do directly writing to shader
		VkWriteDescriptorSet vkWriteDescriptorSet_array[3];
		memset((void*)&vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));


		vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkWriteDescriptorSet_array[0].pNext = NULL;
		vkWriteDescriptorSet_array[0].dstSet = vkComputeDescriptorSets[i];//konta set write karaycha ahe
		vkWriteDescriptorSet_array[0].dstArrayElement = 0; //array nai takat mhanun 0
		vkWriteDescriptorSet_array[0].descriptorCount = 1;//kiti write karnar
		vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
		vkWriteDescriptorSet_array[0].pImageInfo = NULL; //no image here
		vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;//tile sathi
		vkWriteDescriptorSet_array[0].dstBinding = 0; //me binding kuthe karu? (binding Index*)

		VkDescriptorBufferInfo storageBufferInfoLastFrame;
		memset((void*)&storageBufferInfoLastFrame, 0, sizeof(VkDescriptorBufferInfo));
		storageBufferInfoLastFrame.buffer = vertexData_Position_SSBO[(i - 1) % swapChainImageCount].vkBuffer;
		storageBufferInfoLastFrame.offset = 0;
		storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;


		vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkWriteDescriptorSet_array[1].pNext = NULL;
		vkWriteDescriptorSet_array[1].dstSet = vkComputeDescriptorSets[i];//konta set write karaycha ahe
		vkWriteDescriptorSet_array[1].dstArrayElement = 0; //array nai takat mhanun 0
		vkWriteDescriptorSet_array[1].descriptorCount = 1;//kiti write karnar
		vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vkWriteDescriptorSet_array[1].pBufferInfo = &storageBufferInfoLastFrame;
		vkWriteDescriptorSet_array[1].pImageInfo = NULL; //no image here
		vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;//tile sathi
		vkWriteDescriptorSet_array[1].dstBinding = 1; //me binding kuthe karu? (binding Index*)

		VkDescriptorBufferInfo storageBufferInfoCurrentFrame;
		memset((void*)&storageBufferInfoCurrentFrame, 0, sizeof(VkDescriptorBufferInfo));
		storageBufferInfoCurrentFrame.buffer = vertexData_Position_SSBO[i].vkBuffer;
		storageBufferInfoCurrentFrame.offset = 0;
		storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;

		vkWriteDescriptorSet_array[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkWriteDescriptorSet_array[2].pNext = NULL;
		vkWriteDescriptorSet_array[2].dstSet = vkComputeDescriptorSets[i];//konta set write karaycha ahe
		vkWriteDescriptorSet_array[2].dstArrayElement = 0; //array nai takat mhanun 0
		vkWriteDescriptorSet_array[2].descriptorCount = 1;//kiti write karnar
		vkWriteDescriptorSet_array[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vkWriteDescriptorSet_array[2].pBufferInfo = &storageBufferInfoCurrentFrame;
		vkWriteDescriptorSet_array[2].pImageInfo = NULL; //no image here
		vkWriteDescriptorSet_array[2].pTexelBufferView = NULL;//tile sathi
		vkWriteDescriptorSet_array[2].dstBinding = 2; //me binding kuthe karu? (binding Index*)

		vkUpdateDescriptorSets(vkDevice, 3, vkWriteDescriptorSet_array, 0, NULL);
		fprintf(gpFile, "updateDescriptorSets() : vkUpdateDescriptorSets() is successfull\n");
		fflush(gpFile);
	}

	return vkResult;
}

VkResult createDescriptorSet(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	/*// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorSetAllocateInfo {
	VkStructureType                 sType;
	const void*                     pNext;
	VkDescriptorPool                descriptorPool;
	uint32_t                        descriptorSetCount;
	const VkDescriptorSetLayout*    pSetLayouts;
	} VkDescriptorSetAllocateInfo;*/

	//initialize descriptorSetAllocationInfo
	VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
	memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

	vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	vkDescriptorSetAllocateInfo.pNext = NULL; //dynamic karanyasathi
	vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
	vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
	vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

	//fix function sarkh asel tar flags nasto

	vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorSet() : vkAllocateDescriptorSets() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createDescriptorSet() : vkAllocateDescriptorSets() is successfull\n");
		fflush(gpFile);
	}

	//describe whether we want buffer as uniform or image as uniform
	/*// Provided by VK_VERSION_1_0
typedef struct VkDescriptorBufferInfo {
	VkBuffer        buffer;
	VkDeviceSize    offset;
	VkDeviceSize    range;
} VkDescriptorBufferInfo;*/
	VkDescriptorBufferInfo vkDescriptorBufferInfo;
	memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
	vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
	vkDescriptorBufferInfo.offset = 0;
	vkDescriptorBufferInfo.range = sizeof(MyUniformData);

	//now update the descriptor set directly to the shader
	//there are two ways to update
	//1. writing directly to shader
	//2. copying from one shader to another shader 

	//we will do directly writing to shader
	VkWriteDescriptorSet vkWriteDescriptorSet;
	memset((void*)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));

	/*// Provided by VK_VERSION_1_0
	typedef struct VkWriteDescriptorSet {
	VkStructureType                  sType;
	const void*                      pNext;
	VkDescriptorSet                  dstSet;
	uint32_t                         dstBinding;
	uint32_t                         dstArrayElement;
	uint32_t                         descriptorCount;
	VkDescriptorType                 descriptorType;
	const VkDescriptorImageInfo*     pImageInfo;
	const VkDescriptorBufferInfo*    pBufferInfo;
	const VkBufferView*              pTexelBufferView;
	} VkWriteDescriptorSet;*/

	vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkWriteDescriptorSet.pNext = NULL;
	vkWriteDescriptorSet.dstSet = vkDescriptorSet;//konta set write karaycha ahe
	vkWriteDescriptorSet.dstArrayElement = 0; //array nai takat mhanun 0
	vkWriteDescriptorSet.descriptorCount = 1;//kiti write karnar
	vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
	vkWriteDescriptorSet.pImageInfo = NULL; //no image here
	vkWriteDescriptorSet.pTexelBufferView = NULL;//tile sathi
	vkWriteDescriptorSet.dstBinding = 0; //me binding kuthe karu? (binding Index*)

	/*// Provided by VK_VERSION_1_0
	void vkUpdateDescriptorSets(
	VkDevice                                    device,
	uint32_t                                    descriptorWriteCount,
	const VkWriteDescriptorSet*                 pDescriptorWrites,
	uint32_t                                    descriptorCopyCount, //these are for copy operation
	const VkCopyDescriptorSet*                  pDescriptorCopies);*/

	vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);
	fprintf(gpFile, "updateDescriptorSets() : vkUpdateDescriptorSets() is successfull\n");
	fflush(gpFile);
	return vkResult;
}

VkResult createShaders(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code for vertex shaders
	const char* szFileName = "Shader.vert.spv";
	FILE* fp = NULL;
	size_t size;

	fp = fopen(szFileName, "rb");
	if (fp == NULL)
	{
		fprintf(gpFile, "CreateShaders() : failed to open spirv file of vertex shader\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : Succeded to open spirv file of vertex shader\n");
		fflush(gpFile);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "CreateShaders() : ftell failed.. for spirv vertex shader file size is zero\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	fseek(fp, 0L, SEEK_SET);

	char* shaderData = (char*)malloc(sizeof(char) * size);

	size_t retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "CreateShaders() : fread failed.. for spirv vertex shader file\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : Succeded to fread spirv file of vertex shader\n");
		fflush(gpFile);
	}

	fclose(fp);

	VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; //reserved for future use , hence must be zero
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "CreateShaders() : vkCreateShaderModule() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : vkCreateShaderModule() is successfull\n");
		fflush(gpFile);
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gpFile, "CreateShaders() : vertex Shader module completed!");
	fflush(gpFile);

	//for fragment shaders

	szFileName = "Shader.frag.spv";
	fp = NULL;
	size = 0;

	fp = fopen(szFileName, "rb");
	if (fp == NULL)
	{
		fprintf(gpFile, "CreateShaders() : failed to open spirv file of fragment shader\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : Succeded to open spirv file of fragment shader\n");
		fflush(gpFile);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "CreateShaders() : ftell failed.. for spirv fragment shader file size is zero\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	fseek(fp, 0L, SEEK_SET);

	shaderData = (char*)malloc(sizeof(char) * size);

	retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "CreateShaders() : fread failed.. for spirv fragment shader file\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : Succeded to fread spirv file of vertex shader\n");
		fflush(gpFile);
	}

	fclose(fp);


	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; //reserved for future use , hence must be zero
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "CreateShaders() : vkCreateShaderModule() is failed for fragment shader with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : vkCreateShaderModule() is successfull for fragment shader\n");
		fflush(gpFile);
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gpFile, "CreateShaders() : fragment Shader module completed!");
	fflush(gpFile);

	//compute

	szFileName = "Shader.comp.spv";
	fp = NULL;
	size = 0;

	fp = fopen(szFileName, "rb");
	if (fp == NULL)
	{
		fprintf(gpFile, "CreateShaders() : failed to open spirv file of compute shader\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : Succeded to open spirv file of compute shader\n");
		fflush(gpFile);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "CreateShaders() : ftell failed.. for spirv compute shader file size is zero\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	fseek(fp, 0L, SEEK_SET);

	shaderData = (char*)malloc(sizeof(char) * size);

	retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "CreateShaders() : fread failed.. for spirv compute shader file\n");
		fflush(gpFile);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : Succeded to fread spirv file of compute shader\n");
		fflush(gpFile);
	}

	fclose(fp);


	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; //reserved for future use , hence must be zero
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_compute_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "CreateShaders() : vkCreateShaderModule() is failed for compute shader with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "CreateShaders() : vkCreateShaderModule() is successfull for compute shader\n");
		fflush(gpFile);
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gpFile, "CreateShaders() : compute Shader module completed!");
	fflush(gpFile);
	return vkResult;
}

VkResult createRenderPass(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	/*// Provided by VK_VERSION_1_0
typedef struct VkAttachmentDescription {
	VkAttachmentDescriptionFlags    flags;
	VkFormat                        format;
	VkSampleCountFlagBits           samples;
	VkAttachmentLoadOp              loadOp;
	VkAttachmentStoreOp             storeOp;
	VkAttachmentLoadOp              stencilLoadOp;
	VkAttachmentStoreOp             stencilStoreOp;
	VkImageLayout                   initialLayout;
	VkImageLayout                   finalLayout;
} VkAttachmentDescription;*/
//declare and initialise VkAttachmentDescription structure
	VkAttachmentDescription vkAttachmentDescription_array[1];
	memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
	vkAttachmentDescription_array[0].flags = 0;
	vkAttachmentDescription_array[0].format = vkFormat_color;
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;//eka image la kas nivadanar : specifies an image with one sample per pixel.
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //contents within the render area will be cleared to a uniform value
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//it is for both depth and stensil
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//it is for both depth and stensil
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//data arrangement ch kai karu? (Unpacking)
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //source hoti tashi present karaychi

	//declare and init VkAttachmentReference
	VkAttachmentReference vkAttachmentReference;
	memset((void*)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

	vkAttachmentReference.attachment = 0;//0th index number (color attachment)
	vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//layout kasa thevaycha, vapraycha kasa? (color attachment mhanun vapar)

	/*// Provided by VK_VERSION_1_0
typedef struct VkSubpassDescription {
	VkSubpassDescriptionFlags       flags;
	VkPipelineBindPoint             pipelineBindPoint;
	uint32_t                        inputAttachmentCount;
	const VkAttachmentReference*    pInputAttachments;
	uint32_t                        colorAttachmentCount;
	const VkAttachmentReference*    pColorAttachments;
	const VkAttachmentReference*    pResolveAttachments;
	const VkAttachmentReference*    pDepthStencilAttachment;
	uint32_t                        preserveAttachmentCount;
	const uint32_t*                 pPreserveAttachments;
} VkSubpassDescription;*/
//declare and initialize VkSubPassDescription
	VkSubpassDescription vkSubpassDescription;
	memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //binding as a graphics pipeline.
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(&vkAttachmentReference);
	vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = NULL;
	vkSubpassDescription.preserveAttachmentCount = 0;
	vkSubpassDescription.pPreserveAttachments = NULL;


	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	/*// Provided by VK_VERSION_1_0
typedef struct VkRenderPassCreateInfo {
	VkStructureType                   sType;
	const void*                       pNext;
	VkRenderPassCreateFlags           flags;
	uint32_t                          attachmentCount;
	const VkAttachmentDescription*    pAttachments;
	uint32_t                          subpassCount;
	const VkSubpassDescription*       pSubpasses;
	uint32_t                          dependencyCount;
	const VkSubpassDependency*        pDependencies;
} VkRenderPassCreateInfo;*/
	VkRenderPassCreateInfo vkRenderPassCreateInfo;
	memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
	vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.pNext = NULL;
	vkRenderPassCreateInfo.flags = 0;
	vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
	vkRenderPassCreateInfo.subpassCount = 1;
	vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
	/*vkRenderPassCreateInfo.dependencyCount = 0;
	vkRenderPassCreateInfo.pDependencies = NULL;*/
	vkRenderPassCreateInfo.dependencyCount = 1;
	vkRenderPassCreateInfo.pDependencies = &dependency;

	vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createRenderPass() : vkCreateRenderPass() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createRenderPass() : vkCreateRenderPass() is successfull\n");
		fflush(gpFile);
	}


	return vkResult;
}

VkResult createComputePipeline(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//As pipelines are created from pipelineCache
	//Now we will create PipelineCache

	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;//reserved

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createComputePipeline() : vkCreatePipelineCache() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createComputePipeline() : vkCreatePipelineCache() is successfull\n");
		fflush(gpFile);
	}


	/*// Provided by VK_VERSION_1_0
	typedef struct VkComputePipelineCreateInfo {
	VkStructureType                    sType;
	const void*                        pNext;
	VkPipelineCreateFlags              flags;
	VkPipelineShaderStageCreateInfo    stage;
	VkPipelineLayout                   layout;
	VkPipeline                         basePipelineHandle;
	int32_t                            basePipelineIndex;
	} VkComputePipelineCreateInfo;*/

	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[1];
	memset((void*)vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));

	//Vertex Shader
	vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo[0].pNext = NULL; //if not specified, validation error yeto; (jyala extensions ahet, tyana pNext = NULL dyayla pahije
	vkPipelineShaderStageCreateInfo[0].flags = 0;//reserved
	vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
	vkPipelineShaderStageCreateInfo[0].module = vkShaderModule_compute_shader;
	vkPipelineShaderStageCreateInfo[0].pName = "main";//entry point ch function cha adress(d3dcompile)
	vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL;

	VkComputePipelineCreateInfo vkComputePipelineCreateInfo;
	memset((void*)&vkComputePipelineCreateInfo, 0, sizeof(VkComputePipelineCreateInfo));

	vkComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	vkComputePipelineCreateInfo.pNext = NULL;
	vkComputePipelineCreateInfo.flags = 0;
	vkComputePipelineCreateInfo.layout = vkComputePipelineLayout;
	vkComputePipelineCreateInfo.stage = vkPipelineShaderStageCreateInfo[0];

	//Now create the pipeline
	vkResult = vkCreateComputePipelines(vkDevice, vkPipelineCache, 1, &vkComputePipelineCreateInfo, NULL, &vkComputePipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createComputePipeline() : vkCreateComputePipelines() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createComputePipeline() : vkCreateComputePipelines() is successfull\n");
		fflush(gpFile);
	}

	//we have done with pipelineCache
	vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
	vkPipelineCache = VK_NULL_HANDLE;
	return vkResult;
}

VkResult createPipeline(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//********************************************************************************1. vertex Inpout State ********************************************************************

	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1]; //position and color Buffer

	memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));


	//for position
	vkVertexInputBindingDescription_array[0].binding = 0; //GL_ARRAY_BUFFER chya buffer chya array madhe 0 the index (layout location = 0)
	vkVertexInputBindingDescription_array[0].stride = sizeof(Particle);
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //(dx11 ->D3D11_INPUT_ELEMENT_DESC ->InstanceDataStepRate) (vertices ki indices)

	//vkVertexInputBindingDescription_array[1].binding = 1; //GL_ARRAY_BUFFER chya buffer chya array madhe 0 the index (layout location = 0)
	//vkVertexInputBindingDescription_array[1].stride = sizeof(Particle);
	//vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //(dx11 ->D3D11_INPUT_ELEMENT_DESC ->InstanceDataStepRate) (vertices ki indices)

	/*// Provided by VK_VERSION_1_0
	typedef struct VkVertexInputAttributeDescription {
	uint32_t    location;
	uint32_t    binding;
	VkFormat    format;
	uint32_t    offset;
	} VkVertexInputAttributeDescription;*/
	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2]; //position and color Buffer
	memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

	//shader madhlya attribute baddal
	//for Position
	vkVertexInputAttributeDescription_array[0].location = 0; //layout location = 0 in shader (should be same)
	vkVertexInputAttributeDescription_array[0].binding = 0;//(vegla honar binding doghanch, interleave chya veli VertexInputBindingDescription = 0, VertexInputAttributeDescription = change honar)
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32_SFLOAT;
	vkVertexInputAttributeDescription_array[0].offset = offsetof(Particle, position); //interleave chya veli vaprat yeil
	fprintf(gpFile, "createPipeline() : vkVertexInputAttributeDescription_array[0].offset = %d is successfull\n", vkVertexInputAttributeDescription_array[0].offset);
	fflush(gpFile);
	

	//for Color
	vkVertexInputAttributeDescription_array[1].location = 1; //layout location = 1 in shader (should be same)
	vkVertexInputAttributeDescription_array[1].binding = 0;//(vegla honar binding doghanch, interleave chya veli VertexInputBindingDescription = 0, VertexInputAttributeDescription = change honar)
	vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vkVertexInputAttributeDescription_array[1].offset = offsetof(Particle, color); //interleave chya veli vaprat yeil
	fprintf(gpFile, "createPipeline() : vkVertexInputAttributeDescription_array[1].offset = %d is successfull\n", vkVertexInputAttributeDescription_array[1].offset);
	fprintf(gpFile, "createPipeline() : vkCreatePipelines() is successfull\n");
	fflush(gpFile);

	/*// Provided by VK_VERSION_1_0
typedef struct VkPipelineVertexInputStateCreateInfo {
	VkStructureType                             sType;
	const void*                                 pNext;
	VkPipelineVertexInputStateCreateFlags       flags;
	uint32_t                                    vertexBindingDescriptionCount;
	const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
	uint32_t                                    vertexAttributeDescriptionCount;
	const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
} VkPipelineVertexInputStateCreateInfo;*/
	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

	//******************************************************************************** 2. InputAssemblyState ********************************************************************************
		/*// Provided by VK_VERSION_1_0
	typedef struct VkPipelineInputAssemblyStateCreateInfo {
		VkStructureType                            sType;
		const void*                                pNext;
		VkPipelineInputAssemblyStateCreateFlags    flags;
		VkPrimitiveTopology                        topology;
		VkBool32                                   primitiveRestartEnable;
	} VkPipelineInputAssemblyStateCreateInfo;
	*/
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; //11
	/*// Provided by VK_VERSION_1_0
	typedef enum VkPrimitiveTopology {
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 6, //Geometry shader
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 7, //Geometry shader
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 8, //Geometry shader
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 9, //Geometry shader
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST = 10, //tesselation
	} VkPrimitiveTopology;*/

	//primitiveRestartEnable => In Geometry shader, and indexDrawing (strip and Fan chya) tithe 1 (true), ata ithe 0 (false)

	/*// Provided by VK_VERSION_1_0
typedef struct VkPipelineRasterizationStateCreateInfo {
	VkStructureType                            sType;
	const void*                                pNext;
	VkPipelineRasterizationStateCreateFlags    flags;
	VkBool32                                   depthClampEnable;
	VkBool32                                   rasterizerDiscardEnable;
	VkPolygonMode                              polygonMode;
	VkCullModeFlags                            cullMode;
	VkFrontFace                                frontFace;
	VkBool32                                   depthBiasEnable;
	float                                      depthBiasConstantFactor;
	float                                      depthBiasClamp;
	float                                      depthBiasSlopeFactor;
	float                                      lineWidth;
} VkPipelineRasterizationStateCreateInfo;
*/
//************************************************************************************** 3.Rasterizer State ***********************************************************************
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; //GL_POLYGON_MODE in ffp
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; //for point and line drawing this will not needed,so it will help for optimization
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //triangle winding order(in opengl, left bottom, AntiClockWise) *************IMP*************
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0;//validation error la err yete if not given (it is Implementation dependent, min 1.0 dya)

	/*// Provided by VK_VERSION_1_0
	typedef struct VkPipelineColorBlendAttachmentState {
	VkBool32                 blendEnable;
	VkBlendFactor            srcColorBlendFactor;
	VkBlendFactor            dstColorBlendFactor;
	VkBlendOp                colorBlendOp;
	VkBlendFactor            srcAlphaBlendFactor;
	VkBlendFactor            dstAlphaBlendFactor;
	VkBlendOp                alphaBlendOp;
	VkColorComponentFlags    colorWriteMask;
	} VkPipelineColorBlendAttachmentState;
	*/
	//In case where fragment shader not there => In redbook (only vertex shader ) => so this is null in Redbook

	//*********************************************************************************************************** 4. Color Blend State *****************************************************
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xF;//VK_COLOR_COMPONENT_R_BIT; //Important
	/*vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_TRUE;
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD;
	vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD;
	vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;*/

	/*// Provided by VK_VERSION_1_0
typedef struct VkPipelineColorBlendStateCreateInfo {
	VkStructureType                               sType;
	const void*                                   pNext;
	VkPipelineColorBlendStateCreateFlags          flags;
	VkBool32                                      logicOpEnable;
	VkLogicOp                                     logicOp;
	uint32_t                                      attachmentCount;
	const VkPipelineColorBlendAttachmentState*    pAttachments;
	float                                         blendConstants[4];
} VkPipelineColorBlendStateCreateInfo;*/
	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
	vkPipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	vkPipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	vkPipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	vkPipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
	vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	vkPipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	//******************************************************************************************************* 5 ViewPort scissor state ********************************************************

	/*// Provided by VK_VERSION_1_0
typedef struct VkPipelineViewportStateCreateInfo {
	VkStructureType                       sType;
	const void*                           pNext;
	VkPipelineViewportStateCreateFlags    flags;
	uint32_t                              viewportCount;
	const VkViewport*                     pViewports;
	uint32_t                              scissorCount;
	const VkRect2D*                       pScissors;
} VkPipelineViewportStateCreateInfo;*/
	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0;
	vkPipelineViewportStateCreateInfo.viewportCount = 1; //we can give multiple viewport here ********

	/*// Provided by VK_VERSION_1_0
	typedef struct VkViewport {
	float    x;
	float    y;
	float    width;
	float    height;
	float    minDepth;
	float    maxDepth;
	} VkViewport;*/
	memset((void*)&vkViewport, 0, sizeof(VkViewport));
	vkViewport.x = 0;
	vkViewport.y = 0;
	vkViewport.width = (float)vkExtent2D_swapchain.width;
	vkViewport.height = (float)vkExtent2D_swapchain.height;
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;

	vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;

	vkPipelineViewportStateCreateInfo.scissorCount = 1;

	/*// Provided by VK_VERSION_1_0
typedef struct VkRect2D {
	VkOffset2D    offset;
	VkExtent2D    extent;
} VkRect2D;*/
	memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
	vkRect2D_scissor.offset.x = 0;
	vkRect2D_scissor.offset.y = 0;
	vkRect2D_scissor.extent.width = (float)vkExtent2D_swapchain.width;
	vkRect2D_scissor.extent.height = (float)vkExtent2D_swapchain.height;

	vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;



	std::vector<VkDynamicState> dynamicStates = {
		   VK_DYNAMIC_STATE_VIEWPORT,
		   VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	//we can create multiple pipelines
	//viewport ani scissor count members must be same, for multiple pipelines

	//******************************************************************************************************* 5 ViewPort scissor state ********************************************************

	//as we dont have depth yet, we can omit this state

	//******************************************************************************************************* 6 Dynamic state ********************************************************

	//we dont have any dynamic state

	//******************************************************************************************************* 7 Multi Sample state ********************************************************
	//in redbook it is null, we have fragment shader so we need it
	/*// Provided by VK_VERSION_1_0
	typedef struct VkPipelineMultisampleStateCreateInfo {
	VkStructureType                          sType;
	const void*                              pNext;
	VkPipelineMultisampleStateCreateFlags    flags;
	VkSampleCountFlagBits                    rasterizationSamples;
	VkBool32                                 sampleShadingEnable;
	float                                    minSampleShading;
	const VkSampleMask*                      pSampleMask;
	VkBool32                                 alphaToCoverageEnable;
	VkBool32                                 alphaToOneEnable;
	} VkPipelineMultisampleStateCreateInfo;*/

	//required for MSAA
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0;//reserved
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//if not specified, validation error yeto;

	//******************************************************************************************************* 8 Shader state ********************************************************
	/*// Provided by VK_VERSION_1_0
typedef struct VkPipelineShaderStageCreateInfo {
	VkStructureType                     sType;
	const void*                         pNext;
	VkPipelineShaderStageCreateFlags    flags;
	VkShaderStageFlagBits               stage;
	VkShaderModule                      module;
	const char*                         pName;
	const VkSpecializationInfo*         pSpecializationInfo;
} VkPipelineShaderStageCreateInfo;*/
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

	//Vertex Shader
	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL; //if not specified, validation error yeto; (jyala extensions ahet, tyana pNext = NULL dyayla pahije
	vkPipelineShaderStageCreateInfo_array[0].flags = 0;//reserved
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
	vkPipelineShaderStageCreateInfo_array[0].pName = "main";//entry point ch function cha adress(d3dcompile)
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

	//fragment shader
	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[1].flags = 0;//reserved
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
	vkPipelineShaderStageCreateInfo_array[1].pName = "main";//entry point ch function cha adress(d3dcompile)
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;

	//******************************************************************************************************* 9 tesselator state ********************************************************
	//we dont have tesellation yet, we can omit this state

	//As pipelines are created from pipelineCache
	//Now we will create PipelineCache

	/*// Provided by VK_VERSION_1_0
typedef struct VkPipelineCacheCreateInfo {
	VkStructureType               sType;
	const void*                   pNext;
	VkPipelineCacheCreateFlags    flags;
	size_t                        initialDataSize;
	const void*                   pInitialData;
} VkPipelineCacheCreateInfo;
*/
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;//reserved

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createPipeline() : vkCreatePipelineCache() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createPipeline() : vkCreatePipelineCache() is successfull\n");
		fflush(gpFile);
	}



	/*// Provided by VK_VERSION_1_0
typedef struct VkGraphicsPipelineCreateInfo {
	VkStructureType                                  sType;
	const void*                                      pNext;
	VkPipelineCreateFlags                            flags;
	uint32_t                                         stageCount;
	const VkPipelineShaderStageCreateInfo*           pStages;
	const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
	const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
	const VkPipelineTessellationStateCreateInfo*     pTessellationState;
	const VkPipelineViewportStateCreateInfo*         pViewportState;
	const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
	const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
	const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
	const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
	const VkPipelineDynamicStateCreateInfo*          pDynamicState;
	VkPipelineLayout                                 layout;
	VkRenderPass                                     renderPass;
	uint32_t                                         subpass;
	VkPipeline                                       basePipelineHandle; //existing pipeline la pakdun navin pipeline asel tar
	int32_t                                          basePipelineIndex;
} VkGraphicsPipelineCreateInfo;*/
//create actual graphics pipeline
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;//reserved
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
	//vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
	vkGraphicsPipelineCreateInfo.pDynamicState = &dynamicState;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL;

	vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
	vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
	vkGraphicsPipelineCreateInfo.subpass = 0;

	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

	//Now create the pipeline

	/*// Provided by VK_VERSION_1_0
	VkResult vkCreateGraphicsPipelines(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkGraphicsPipelineCreateInfo*         pCreateInfos,
	const VkAllocationCallbacks*                pAllocator,
	VkPipeline*                                 pPipelines);*/
	vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createPipeline() : vkCreateGraphicsPipelines() is failed with result = %d \n", vkResult);
		fflush(gpFile);
		vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createPipeline() : vkCreateGraphicsPipelines() is successfull\n");
		fflush(gpFile);
	}

	//we have done with pipelineCache

	/*// Provided by VK_VERSION_1_0
void vkDestroyPipelineCache(
	VkDevice                                    device,
	VkPipelineCache                             pipelineCache,
	const VkAllocationCallbacks*                pAllocator);*/
	vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
	vkPipelineCache = VK_NULL_HANDLE;
	return vkResult;
}
VkResult createFramebuffers(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//declare array of VkImageView
	VkImageView vkImageView_attachments_array[1];
	memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

	/*// Provided by VK_VERSION_1_0
typedef struct VkFramebufferCreateInfo {
	VkStructureType             sType;
	const void*                 pNext;
	VkFramebufferCreateFlags    flags;
	VkRenderPass                renderPass;
	uint32_t                    attachmentCount;
	const VkImageView*          pAttachments;
	uint32_t                    width;
	uint32_t                    height;
	uint32_t                    layers;
} VkFramebufferCreateInfo;*/
//declare and initialize VkFramebufferCreateInfo
	VkFramebufferCreateInfo vkFramebufferCreateInfo;
	memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

	vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	vkFramebufferCreateInfo.pNext = NULL;
	vkFramebufferCreateInfo.flags = 0;
	vkFramebufferCreateInfo.renderPass = vkRenderPass;
	vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
	vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
	vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
	vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
	vkFramebufferCreateInfo.layers = 1; //layered rendering  //Usecase Number 2, comment layers, you will get black screen and Validation err : 
	/*MPD_Validation : debugReportCallback() : Validation (0) = Validation Error: [ VUID-VkFramebufferCreateInfo-layers-00889 ] | MessageID = 0x3976be90 | vkCreateFramebuffer(): pCreateInfo->layers is zero.
	The Vulkan spec states: layers must be greater than 0*/

	vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapChainImageCount);

	//loop
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkImageView_attachments_array[0] = swapChainImageView_array[i]; //for multipass rendering, now its 0
		/*// Provided by VK_VERSION_1_0
VkResult vkCreateFramebuffer(
	VkDevice                                    device,
	const VkFramebufferCreateInfo*              pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkFramebuffer*                              pFramebuffer);*/
		vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createFramebuffers() : vkCreateFramebuffer() is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "createFramebuffers() : vkCreateFramebuffer() is successfull\n");
			fflush(gpFile);
		}
	}
	return vkResult;
}

VkResult CreateSemaphores(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
	memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

	vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.pNext = NULL;//const void* (binary)
	vkSemaphoreCreateInfo.flags = 0; //must be zero
	//vkSamaphore_backBuffer.resize(swapChainImageCount);
	vkSamaphore_renderComplete.resize(swapChainImageCount);
	vkComputeSemaphore_renderComplete.resize(swapChainImageCount);

	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSamaphore_backBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "vkCreateSemaphores() : vkCreateSemaphore() for backBuffer is failed with result = %d \n", vkResult);
		fflush(gpFile);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "vkCreateSemaphores() : vkCreateSemaphore() for backBuffer is successfull\n");
		fflush(gpFile);
	}
	for (size_t i = 0; i < swapChainImageCount; i++)
	{
		

		vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSamaphore_renderComplete[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "vkCreateSemaphores() : vkCreateSemaphore() for renderComplete is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "vkCreateSemaphores() : vkCreateSemaphore() for renderComplete is successfull\n");
			fflush(gpFile);
		}

		vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkComputeSemaphore_renderComplete[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "vkCreateSemaphores() : vkCreateSemaphore() for vkComputeSemaphore_renderComplete is failed with result = %d \n", vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "vkCreateSemaphores() : vkCreateSemaphore() for vkComputeSemaphore_renderComplete is successfull\n");
			fflush(gpFile);
		}
	}
	return vkResult;
}

VkResult createFences(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	VkFenceCreateInfo vkFenceCreateInfo1;
	memset((void*)&vkFenceCreateInfo1, 0, sizeof(VkFenceCreateInfo));

	vkFenceCreateInfo1.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo1.pNext = NULL;
	vkFenceCreateInfo1.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkComputeFence_array = (VkFence*)malloc(sizeof(VkFence) * swapChainImageCount);
	//vkComputeFence_array.resize(swapChainImageCount);
	//loop
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo1, NULL, &vkComputeFence_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "vkCreateFences() : vkCreateFence1() vkComputeFence_array failed %d with result = %d \n", i, vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "vkCreateFences() : vkCreateFence1() vkComputeFence_array %d is successfull\n", i);
		}
	}
	
	//declare
	VkFenceCreateInfo vkFenceCreateInfo;
	memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkFence_array = (VkFence*)malloc(sizeof(VkFence) * swapChainImageCount);
	//loop
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "vkCreateFences() : vkCreateFence() %d is failed with result = %d\n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "vkCreateFences() : vkCreateFence() %d is successfull and swapChainImageCount = %d\n", i, swapChainImageCount);
			fflush(gpFile);
		}
	}


	return vkResult;
}

VkResult buildComputeCommandBuffers(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		//reset commandBuffers
		vkResult = vkResetCommandBuffer(vkComputeCommandBuffer_array[i], 0);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildComputeCommandBuffers() : vkResetCommandBuffer() %d is failed with result = %d \n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "buildComputeCommandBuffers() : vkResetCommandBuffer() %d is successfull\n", i);
			fflush(gpFile);
		}

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; //will only primary command buffer, not going to use simultaneously between multiple threads

		vkResult = vkBeginCommandBuffer(vkComputeCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildComputeCommandBuffers() : vkBeginCommandBuffer() %d is failed with result = %d \n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "buildComputeCommandBuffers() : vkBeginCommandBuffer() %d is successfull\n", i);
			fflush(gpFile);
		}

		vkCmdBindPipeline(vkComputeCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_COMPUTE, vkComputePipeline);

		vkCmdBindDescriptorSets(vkComputeCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_COMPUTE, vkComputePipelineLayout,
			0,//multiple descripotr set tar index
			1,//kiti descriptor set
			&vkComputeDescriptorSets[i],
			0,
			NULL);

		vkCmdDispatch(vkComputeCommandBuffer_array[i], PARTICLE_COUNT / 256, 1, 1);

		//End command Buffer Record
		vkResult = vkEndCommandBuffer(vkComputeCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildComputeCommandBuffers() : vkEndCommandBuffer() %d is failed with result = %d \n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "buildComputeCommandBuffers() : vkEndCommandBuffer() %d is successfull\n", i);
			fflush(gpFile);
		}
	}
	return vkResult;
}

VkResult recordComputeCommandBuffers(uint32_t currentImageIndex)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		//reset commandBuffers
		vkResult = vkResetCommandBuffer(vkComputeCommandBuffer_array[currentImageIndex], 0);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "recordComputeCommandBuffers() : vkResetCommandBuffer() %d is failed with result = %d \n", currentImageIndex, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			/*fprintf(gpFile, "recordComputeCommandBuffers() : vkResetCommandBuffer() %d is successfull\n", currentImageIndex);
			fflush(gpFile);*/
		}

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; //will only primary command buffer, not going to use simultaneously between multiple threads

		vkResult = vkBeginCommandBuffer(vkComputeCommandBuffer_array[currentImageIndex], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "recordComputeCommandBuffers() : vkBeginCommandBuffer() %d is failed with result = %d \n", currentImageIndex, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			/*fprintf(gpFile, "recordComputeCommandBuffers() : vkBeginCommandBuffer() %d is successfull\n", currentImageIndex);
			fflush(gpFile);*/
		}

		vkCmdBindPipeline(vkComputeCommandBuffer_array[currentImageIndex], VK_PIPELINE_BIND_POINT_COMPUTE, vkComputePipeline);

		vkCmdBindDescriptorSets(vkComputeCommandBuffer_array[currentImageIndex], VK_PIPELINE_BIND_POINT_COMPUTE, vkComputePipelineLayout,
			0,//multiple descripotr set tar index
			1,//kiti descriptor set
			&vkComputeDescriptorSets[currentImageIndex],
			0,
			NULL);

		vkCmdDispatch(vkComputeCommandBuffer_array[currentImageIndex], PARTICLE_COUNT / 256, 1, 1);

		//End command Buffer Record
		vkResult = vkEndCommandBuffer(vkComputeCommandBuffer_array[currentImageIndex]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "recordComputeCommandBuffers() : vkEndCommandBuffer() %d is failed with result = %d \n", currentImageIndex, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			/*fprintf(gpFile, "recordComputeCommandBuffers() : vkEndCommandBuffer() %d is successfull\n", currentImageIndex);
			fflush(gpFile);*/
		}
	}
	return vkResult;
}

VkResult buildCommandBuffers(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//loop per swapChainImage
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		//reset commandBuffers
		vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildCommandBuffers() : vkResetCommandBuffer() %d is failed with result = %d \n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "buildCommandBuffers() : vkResetCommandBuffer() %d is successfull\n", i);
			fflush(gpFile);
		}

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; //will only primary command buffer, not going to use simultaneously between multiple threads

		vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildCommandBuffers() : vkBeginCommandBuffer() %d is failed with result = %d \n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "buildCommandBuffers() : vkBeginCommandBuffer() %d is successfull\n", i);
			fflush(gpFile);
		}

		VkClearValue vkClearValue_array[1];
		memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

		vkClearValue_array[0].color = vkClearColorValue;

		//renderPassBegin
		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = vkRenderPass;
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
		vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height; //D3DViewPort

		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_array;

		vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[i];
		/*fprintf(gpFile, "buildCommandBuffers() : Here %d I Am\n", i);
		fflush(gpFile);*/
		vkCmdBeginRenderPass(vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //every process has main thread
		//content of renderPass are contents of subpass and part of primary command Buffers

		//Bind with the pipeline

		/*// Provided by VK_VERSION_1_0
		void vkCmdBindPipeline(
		VkCommandBuffer                             commandBuffer,
		VkPipelineBindPoint                         pipelineBindPoint,
		VkPipeline                                  pipeline);*/
		vkCmdBindPipeline(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
		/*fprintf(gpFile, "buildCommandBuffers() : Here........... %d I Am\n", i);
		fflush(gpFile);*/
		/*// Provided by VK_VERSION_1_0
		void vkCmdBindDescriptorSets(
		VkCommandBuffer                             commandBuffer,
		VkPipelineBindPoint                         pipelineBindPoint,
		VkPipelineLayout                            layout,
		uint32_t                                    firstSet,
		uint32_t                                    descriptorSetCount,
		const VkDescriptorSet*                      pDescriptorSets,
		uint32_t                                    dynamicOffsetCount,
		const uint32_t*                             pDynamicOffsets);*/
		//bind our descriptorSet to the pipeline
		//vkCmdBindDescriptorSets(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout,
		//	0,//multiple descripotr set tar index
		//	1,//kiti descriptor set
		//	&vkDescriptorSet,
		//	0,
		//	NULL);
		/*fprintf(gpFile, "buildCommandBuffers() : Here ************* %d I Am\n", i);
		fflush(gpFile);*/
		//bind with Vertex Buffer
		/*// Provided by VK_VERSION_1_0
		typedef uint64_t VkDeviceSize;*/
		//bind Vertex Position Buffer

		vkViewport.x = 0;
		vkViewport.y = 0;
		vkViewport.width = (float)vkExtent2D_swapchain.width;
		vkViewport.height = (float)vkExtent2D_swapchain.height;
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;
		vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);
		/*vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;

		vkPipelineViewportStateCreateInfo.scissorCount = 1;*/

		/*// Provided by VK_VERSION_1_0
	typedef struct VkRect2D {
		VkOffset2D    offset;
		VkExtent2D    extent;
	} VkRect2D;*/
	//memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
		vkRect2D_scissor.offset.x = 0;
		vkRect2D_scissor.offset.y = 0;
		vkRect2D_scissor.extent.width = (float)vkExtent2D_swapchain.width;
		vkRect2D_scissor.extent.height = (float)vkExtent2D_swapchain.height;
		vkCmdSetScissor(vkCommandBuffer_array[i], 0, 1, &vkRect2D_scissor);

		VkDeviceSize vkDeviceSize_offset_position[] = { 0 };

		//memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkCommandBuffer_array));


		/*// Provided by VK_VERSION_1_0
		void vkCmdBindVertexBuffers(
		VkCommandBuffer                             commandBuffer,
		uint32_t                                    firstBinding,
		uint32_t                                    bindingCount,
		const VkBuffer*                             pBuffers,
		const VkDeviceSize*                         pOffsets);*/ //offset is if multiple buffer che binding point astil, 
		//vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_Position.vkBuffer, vkDeviceSize_offset_position);
		vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_Position_SSBO[i].vkBuffer, vkDeviceSize_offset_position);

		//bind Color Buffer
		/*VkDeviceSize vkDeviceSize_offset_color[1];
		memset((void*)vkDeviceSize_offset_color, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkCommandBuffer_array));*/

		//vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 1, 1, &vertexData_Color.vkBuffer, vkDeviceSize_offset_color);

		//**************Here we should call Vulkan Drawing functions******************
		/*// Provided by VK_VERSION_1_0
		void vkCmdDraw(
		VkCommandBuffer                             commandBuffer,
		uint32_t                                    vertexCount,
		uint32_t                                    instanceCount,
		uint32_t                                    firstVertex,
		uint32_t                                    firstInstance);*/
		vkCmdDraw(vkCommandBuffer_array[i], PARTICLE_COUNT, 1, 0, 0);

		vkCmdEndRenderPass(vkCommandBuffer_array[i]);

		//End command Buffer Record
		vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildCommandBuffers() : vkEndCommandBuffer() %d is failed with result = %d \n", i, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "buildCommandBuffers() : vkEndCommandBuffer() %d is successfull\n", i);
			fflush(gpFile);
		}
	}

	return vkResult;
}

VkResult recordCommandBuffers(uint32_t currentImageIndex)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//loop per swapChainImage
	//for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		//reset commandBuffers
		vkResult = vkResetCommandBuffer(vkCommandBuffer_array[currentImageIndex], 0);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "recordCommandBuffers() : vkResetCommandBuffer() %d is failed with result = %d \n", currentImageIndex, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			/*fprintf(gpFile, "recordCommandBuffers() : vkResetCommandBuffer() %d is successfull\n", currentImageIndex);
			fflush(gpFile);*/
		}

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; //will only primary command buffer, not going to use simultaneously between multiple threads

		vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[currentImageIndex], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "recordCommandBuffers() : vkBeginCommandBuffer() %d is failed with result = %d \n", currentImageIndex, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			/*fprintf(gpFile, "recordCommandBuffers() : vkBeginCommandBuffer() %d is successfull\n", currentImageIndex);
			fflush(gpFile);*/
		}

		VkClearValue vkClearValue_array[1];
		memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

		vkClearValue_array[0].color = vkClearColorValue;

		//renderPassBegin
		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = vkRenderPass;
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
		vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height; //D3DViewPort

		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_array;

		vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[currentImageIndex];
		/*fprintf(gpFile, "recordCommandBuffers() : Here %d I Am\n", currentImageIndex);
		fflush(gpFile);*/
		vkCmdBeginRenderPass(vkCommandBuffer_array[currentImageIndex], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //every process has main thread
		//content of renderPass are contents of subpass and part of primary command Buffers

		//Bind with the pipeline

		vkCmdBindPipeline(vkCommandBuffer_array[currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
		/*fprintf(gpFile, "recordCommandBuffers() : Here........... %d I Am\n", currentImageIndex);
		fflush(gpFile);*/
		

		//memset((void*)&vkViewport, 0, sizeof(VkViewport));
		vkViewport.x = 0;
		vkViewport.y = 0;
		vkViewport.width = (float)vkExtent2D_swapchain.width;
		vkViewport.height = (float)vkExtent2D_swapchain.height;
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;
		vkCmdSetViewport(vkCommandBuffer_array[currentImageIndex], 0, 1, &vkViewport);
		/*vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;

		vkPipelineViewportStateCreateInfo.scissorCount = 1;*/

		/*// Provided by VK_VERSION_1_0
	typedef struct VkRect2D {
		VkOffset2D    offset;
		VkExtent2D    extent;
	} VkRect2D;*/
		//memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
		vkRect2D_scissor.offset.x = 0;
		vkRect2D_scissor.offset.y = 0;
		vkRect2D_scissor.extent.width = (float)vkExtent2D_swapchain.width;
		vkRect2D_scissor.extent.height = (float)vkExtent2D_swapchain.height;
		vkCmdSetScissor(vkCommandBuffer_array[currentImageIndex], 0, 1, &vkRect2D_scissor);

		//bind with Vertex Buffer
		/*// Provided by VK_VERSION_1_0
		typedef uint64_t VkDeviceSize;*/
		//bind Vertex Position Buffer
		VkDeviceSize vkDeviceSize_offset_position[] = { 0 };

		//memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkCommandBuffer_array));	
		//vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_Position.vkBuffer, vkDeviceSize_offset_position);
		vkCmdBindVertexBuffers(vkCommandBuffer_array[currentImageIndex], 0, 1, &vertexData_Position_SSBO[currentImageIndex].vkBuffer, vkDeviceSize_offset_position);

		//bind Color Buffer
		/*VkDeviceSize vkDeviceSize_offset_color[1];
		memset((void*)vkDeviceSize_offset_color, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkCommandBuffer_array));*/

		vkCmdDraw(vkCommandBuffer_array[currentImageIndex], PARTICLE_COUNT, 1, 0, 0);

		vkCmdEndRenderPass(vkCommandBuffer_array[currentImageIndex]);

		//End command Buffer Record
		vkResult = vkEndCommandBuffer(vkCommandBuffer_array[currentImageIndex]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "recordCommandBuffers() : vkEndCommandBuffer() %d is failed with result = %d \n", currentImageIndex, vkResult);
			fflush(gpFile);
			return vkResult;
		}
		else
		{
			//fprintf(gpFile, "recordCommandBuffers() : vkEndCommandBuffer() %d is successfull\n", currentImageIndex);
			//fflush(gpFile);
		}
	}

	return vkResult;
}


//this is the last function everytime in our code
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
	VkDebugReportFlagsEXT vkDebugReportFlagsEXT, //kontya flag ni err gen keli, tya flag cha ID
	VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, //jyane ha callback trigger kela tya object cha type
	uint64_t object, //Object (OS/Vulkan)
	size_t location, //warning or error kuthan ali tyach location
	int32_t messageCode, //message ID
	const char* pLayerPrefix, //kontya layer ni he error ali
	const char* pMessage, //Actual Error message
	void* pUserData) //parameter dila asel tar(CreateWindow cha last param/ COM madhe FormatMessage madhe)
{
	fprintf(gpFile, "MPD_Validation : debugReportCallback() : %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);
	fflush(gpFile);
	return FALSE;
}