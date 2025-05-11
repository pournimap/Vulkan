/*Hardware Ray Tracing*/

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, // Ray tracing related extensions required by this sample
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, //if !RayQuery

    // Required by VK_KHR_acceleration_structure
    //VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    //VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,

    //VK_KHR_SPIRV_1_4_EXTENSION_NAME, // Required for VK_KHR_ray_tracing_pipeline
    //// Required by VK_KHR_spirv_1_4
    //VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};


struct VertexRT {
    glm::vec3 pos;
   // float pos[3];
};

const std::vector<VertexRT> verticesRT = {
     { {  1.0f,  1.0f, 0.0f } },
     { { -1.0f,  1.0f, 0.0f } },
     { {  0.0f, -1.0f, 0.0f } }
};
const std::vector<uint32_t> indicesRT = {
    0, 1, 2
};

//vertex buffer related variables
struct MappedData
{
    VkBuffer vkBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkBufferMemory = VK_NULL_HANDLE;
    uint64_t vkBufferAdress = 0;
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanupRT();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue queue;


    VkCommandPool commandPool;

    MappedData vertexDataRT;

    MappedData indexDataRT;

    MappedData bottomLevelAccelerationStructureMemoryRT;
    VkAccelerationStructureKHR bottomLevelAS = VK_NULL_HANDLE;
    uint64_t bottomLevelASHandle = 0;

    MappedData scratchMemoryRT;

    MappedData instanceBufferRT;

    MappedData topLevelAccelerationStructureMemoryRT;
    VkAccelerationStructureKHR topLevelAS = VK_NULL_HANDLE;
    uint64_t topLevelASHandle = 0;


    VkSemaphore semaphoreImageAvailable = VK_NULL_HANDLE;
    VkSemaphore semaphoreRenderingAvailable = VK_NULL_HANDLE;

    bool framebufferResized = false;

    //raytracing
    VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};

   // void* deviceCreatepNextChain = nullptr;
    uint32_t desiredWindowWidth = 800;
    uint32_t desiredWindowHeight = 600;
    VkFormat desiredSurfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;

    VkImage offScreenBuffer;
    VkImageView offScreenBufferView;
    VkDeviceMemory offScreenBufferMemory;

    VkDescriptorSetLayout descriptorSetLayoutRT;
    VkDescriptorPool descriptorPoolRT;
    VkDescriptorSet descriptorSetRT;

    VkPipelineLayout raytracingPipelineLayout;
    VkPipeline raytracingPipeline;

    uint32_t sbtGroupCount = 3;
    uint32_t sbtHandleSize = 0;
    uint32_t sbtHandleAlignment = 0;
    uint32_t sbtHandleSizeAligned = 0;
    uint32_t sbtSize = 0;

    MappedData sbtRayGenBuffer;
    MappedData sbtRayHitBuffer;
    MappedData sbtRayMissBuffer;

    std::vector<VkCommandBuffer> commandBuffersRT;

    VkCommandPool commandPoolRT;
    VkSwapchainKHR swapChainRT;
    std::vector<VkImage> swapChainImagesRT;
    VkFormat swapChainImageFormatRT;
    VkExtent2D swapChainExtentRT;
    std::vector<VkImageView> swapChainImageViewsRT;
    

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDeviceRT();
      

        createCommandPoolRT();

        //raytracing
       
        prepareForRaytracing();

        createAccelerationStructures();
        createOffscreenBuffer();
        createRTDescriptorSetLayout();
        createRTDescriptorSets();
        createRTPipelineLayout();
        createRTPipeline();

       
        createShaderBindingTable();
        createSwapChainRT();
        createTraceRayCommandBuffer();

        createSyncObjects();

       
    }

    void prepareForRaytracing()
    {
        
        rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        rayTracingPipelineProperties.pNext = NULL;

        // acquire RT properties
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &rayTracingPipelineProperties;

        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

       // VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
       enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
       enabledAccelerationStructureFeatures.pNext = NULL;
       VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &enabledAccelerationStructureFeatures;
        
        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);


    }

   

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            //drawFrame();
            drawFrameRT();
        }

        vkDeviceWaitIdle(device);
    }


    void cleanupSwapChainRT() {
       

        for (auto imageView : swapChainImageViewsRT) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChainRT, nullptr);
    }


    void cleanupRT() {

        PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR_fnptr = NULL;

        vkDestroyAccelerationStructureKHR_fnptr = (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
        if (vkDestroyAccelerationStructureKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkDestroyAccelerationStructureKHR!");
        }
        /*createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDeviceRT();
        createCommandPoolRT();
        //raytracing
        prepareForRaytracing();
        createAccelerationStructures();
        createOffscreenBuffer();
        createRTDescriptorSetLayout();
        createRTDescriptorSets();
        createRTPipelineLayout();
        createRTPipeline();

       
        createShaderBindingTable();
        createSwapChainRT();
        createTraceRayCommandBuffer();

        createSyncObjects();*/
        cleanupSwapChainRT();

        vkDestroySemaphore(device, semaphoreImageAvailable, nullptr);
        vkDestroySemaphore(device, semaphoreRenderingAvailable, nullptr);


        vkDestroyBuffer(device, sbtRayHitBuffer.vkBuffer, nullptr);
        vkFreeMemory(device, sbtRayHitBuffer.vkBufferMemory, nullptr);

        vkDestroyBuffer(device, sbtRayMissBuffer.vkBuffer, nullptr);
        vkFreeMemory(device, sbtRayMissBuffer.vkBufferMemory, nullptr);
   
        vkDestroyBuffer(device, sbtRayGenBuffer.vkBuffer, nullptr);
        vkFreeMemory(device, sbtRayGenBuffer.vkBufferMemory, nullptr);


        vkDestroyPipeline(device, raytracingPipeline, nullptr);
        vkDestroyPipelineLayout(device, raytracingPipelineLayout, nullptr);


        vkDestroyDescriptorPool(device, descriptorPoolRT, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptorSetLayoutRT, nullptr);

        vkDestroyImage(device, offScreenBuffer, nullptr);
        vkDestroyImageView(device, offScreenBufferView, nullptr);
        vkFreeMemory(device, offScreenBufferMemory, nullptr);

        vkDestroyAccelerationStructureKHR_fnptr(device, topLevelAS, nullptr);

        vkDestroyBuffer(device, topLevelAccelerationStructureMemoryRT.vkBuffer, nullptr);
        vkFreeMemory(device, topLevelAccelerationStructureMemoryRT.vkBufferMemory, nullptr);

        vkDestroyBuffer(device, instanceBufferRT.vkBuffer, nullptr);
        vkFreeMemory(device, instanceBufferRT.vkBufferMemory, nullptr);

        vkDestroyAccelerationStructureKHR_fnptr(device, bottomLevelAS, nullptr);

        vkDestroyBuffer(device, bottomLevelAccelerationStructureMemoryRT.vkBuffer, nullptr);
        vkFreeMemory(device, bottomLevelAccelerationStructureMemoryRT.vkBufferMemory, nullptr);


        vkDestroyBuffer(device, scratchMemoryRT.vkBuffer, nullptr);
        vkFreeMemory(device, scratchMemoryRT.vkBufferMemory, nullptr);


        vkDestroyBuffer(device, indexDataRT.vkBuffer, nullptr);
        vkFreeMemory(device, indexDataRT.vkBufferMemory, nullptr);


        vkDestroyBuffer(device, vertexDataRT.vkBuffer, nullptr);
        vkFreeMemory(device, vertexDataRT.vkBufferMemory, nullptr);

        vkDestroyCommandPool(device, commandPoolRT, nullptr);
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
    

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createLogicalDeviceRT()
    {
        const float queuePriority = 0.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueCreateInfo;

        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

        VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
        enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;
        enabledBufferDeviceAddresFeatures.pNext = nullptr;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
        enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
        enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;

        enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
        enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;

        deviceInfo.pNext = &enabledAccelerationStructureFeatures;

        if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        vkGetDeviceQueue(device, 0, 0, &queue);
    }
   
    void createCommandPoolRT() {
        //QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        //poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPoolRT) != VK_SUCCESS) {
            throw std::runtime_error("failed to create RT command pool!");
        }
    }

    void createAccelerationStructures()
    {
        //VkResult vkResult = VK_SUCCESS;
        PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR_fnptr = NULL;

        vkCreateAccelerationStructureKHR_fnptr = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
        if (vkCreateAccelerationStructureKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkCreateAccelerationStructureKHR!");
        }
        
        PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR_fnptr = NULL;

        vkGetAccelerationStructureBuildSizesKHR_fnptr = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR");
        if (vkCreateAccelerationStructureKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkGetAccelerationStructureBuildSizesKHR!");
        }

        PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR_fnptr = NULL;

        vkCmdBuildAccelerationStructuresKHR_fnptr = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
        if (vkCmdBuildAccelerationStructuresKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkCmdBuildAccelerationStructuresKHR!");
        }


        PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR_fnptr = NULL;

        vkGetAccelerationStructureDeviceAddressKHR_fnptr = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR");
        if (vkGetAccelerationStructureDeviceAddressKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkGetAccelerationStructureDeviceAddressKHR!");
        }

        void* dstData = nullptr;
        /****************************************/
        //Creating Bottom - Level Acceleration Structure..
        {
            VkDeviceSize vertexbufferSize = sizeof(verticesRT[0]) * verticesRT.size();

            createBuffer(vertexbufferSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexDataRT.vkBuffer, vertexDataRT.vkBufferMemory);

            VkBufferDeviceAddressInfo vbufferDeviceAddressInfo = {};
            vbufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            vbufferDeviceAddressInfo.buffer = vertexDataRT.vkBuffer;
            vbufferDeviceAddressInfo.pNext = NULL;
            vertexDataRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &vbufferDeviceAddressInfo);
           
            if (verticesRT.data() != nullptr) {
                if (vkMapMemory(device, vertexDataRT.vkBufferMemory, 0, vertexbufferSize, 0, &dstData) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed vkMapMemory!");
                }
                memcpy(dstData, verticesRT.data(), vertexbufferSize);
                vkUnmapMemory(device, vertexDataRT.vkBufferMemory);
            }

            VkDeviceSize indexBufferSize = sizeof(indicesRT[0]) * indicesRT.size();

            createBuffer(indexBufferSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexDataRT.vkBuffer, indexDataRT.vkBufferMemory);

            VkBufferDeviceAddressInfo ibufferDeviceAddressInfo = {};
            ibufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            ibufferDeviceAddressInfo.buffer = indexDataRT.vkBuffer;
            ibufferDeviceAddressInfo.pNext = NULL;
            indexDataRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &ibufferDeviceAddressInfo);

            if (indicesRT.data() != nullptr) {
                if (vkMapMemory(device, indexDataRT.vkBufferMemory, 0, indexBufferSize, 0, &dstData) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed vkMapMemory!");
                }
                memcpy(dstData, indicesRT.data(), indexBufferSize);
                vkUnmapMemory(device, indexDataRT.vkBufferMemory);
            }

            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress = {};
            vertexBufferDeviceAddress.deviceAddress = vertexDataRT.vkBufferAdress;

            VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress = {};
            indexBufferDeviceAddress.deviceAddress = indexDataRT.vkBufferAdress;;

            //create Bottom
            VkAccelerationStructureGeometryKHR asGeometryInfo = {};
            asGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            asGeometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            asGeometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            asGeometryInfo.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            asGeometryInfo.geometry.triangles.vertexData = vertexBufferDeviceAddress;
            asGeometryInfo.geometry.triangles.indexData = indexBufferDeviceAddress;

            asGeometryInfo.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            asGeometryInfo.geometry.triangles.maxVertex = verticesRT.size();
            asGeometryInfo.geometry.triangles.vertexStride = sizeof(VertexRT);
            asGeometryInfo.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;

            VkAccelerationStructureBuildGeometryInfoKHR asBuildSizeGeometryInfo = {};
            asBuildSizeGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
            asBuildSizeGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            asBuildSizeGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            asBuildSizeGeometryInfo.geometryCount = 1;
            asBuildSizeGeometryInfo.pGeometries = &asGeometryInfo;

            // aquire size to build acceleration structure
            const uint32_t primitiveCount = verticesRT.size() / 3;

            VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo = {};
            asBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

            vkGetAccelerationStructureBuildSizesKHR_fnptr(
                device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &asBuildSizeGeometryInfo,
                &primitiveCount, &asBuildSizesInfo);


            // reserve memory to hold the acceleration structure
            createBuffer(asBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bottomLevelAccelerationStructureMemoryRT.vkBuffer, bottomLevelAccelerationStructureMemoryRT.vkBufferMemory);

            VkBufferDeviceAddressInfo abufferDeviceAddressInfo = {};
            abufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            abufferDeviceAddressInfo.buffer = bottomLevelAccelerationStructureMemoryRT.vkBuffer;
            abufferDeviceAddressInfo.pNext = NULL;
            bottomLevelAccelerationStructureMemoryRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &abufferDeviceAddressInfo);

            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {};
            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            accelerationStructureCreateInfo.buffer = bottomLevelAccelerationStructureMemoryRT.vkBuffer;
            accelerationStructureCreateInfo.size = asBuildSizesInfo.accelerationStructureSize;
            accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

            if (vkCreateAccelerationStructureKHR_fnptr(device, &accelerationStructureCreateInfo, nullptr, &bottomLevelAS) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create bottom Level Acceleration Structure!");
            }


            // reserve memory to hold the acceleration structure
            createBuffer(asBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchMemoryRT.vkBuffer, scratchMemoryRT.vkBufferMemory);

            VkBufferDeviceAddressInfo scratchbufferDeviceAddressInfo = {};
            scratchbufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            scratchbufferDeviceAddressInfo.buffer = scratchMemoryRT.vkBuffer;
            scratchbufferDeviceAddressInfo.pNext = NULL;
            scratchMemoryRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &scratchbufferDeviceAddressInfo);

            //buildAccelerationStrcture
            VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo = {};
            asBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
            asBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            asBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            asBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
            asBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS;
            asBuildGeometryInfo.geometryCount = 1;
            asBuildGeometryInfo.pGeometries = &asGeometryInfo;
            asBuildGeometryInfo.scratchData.deviceAddress = scratchMemoryRT.vkBufferAdress;

            VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo = {};
            asBuildRangeInfo.primitiveCount = primitiveCount;
            asBuildRangeInfo.primitiveOffset = 0;
            asBuildRangeInfo.firstVertex = 0;
            asBuildRangeInfo.transformOffset = 0;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> asBuildRangeInfos =
            {
                &asBuildRangeInfo
            };

            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = commandPoolRT;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate command Buffers!");

            VkCommandBufferBeginInfo commandBufferBeginInfo = {};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
                throw std::runtime_error("failed to begin command Buffers!");

            // build the bottom-level acceleration structure
            vkCmdBuildAccelerationStructuresKHR_fnptr(commandBuffer, 1, &asBuildGeometryInfo, asBuildRangeInfos.data());

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to end command Buffers!");

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            VkFence fence = VK_NULL_HANDLE;

            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            vkCreateFence(device, &fenceInfo, nullptr, &fence);
            vkQueueSubmit(queue, 1, &submitInfo, fence);
            vkWaitForFences(device, 1, &fence, true, UINT64_MAX);

            vkDestroyFence(device, fence, nullptr);
            vkFreeCommandBuffers(device, commandPoolRT, 1, &commandBuffer);

            // Get bottom level acceleration structure handle for use in top level instances
            VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo = {};
            asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
            asDeviceAddressInfo.accelerationStructure = bottomLevelAS;

            bottomLevelASHandle = vkGetAccelerationStructureDeviceAddressKHR_fnptr(device, &asDeviceAddressInfo);

            // make sure bottom AS handle is valid
            if (bottomLevelASHandle == 0) {
                throw std::runtime_error("Invalid Handle to BLAS");
            }
        }

        /**************************/
        // create top-level container
        
        {
            VkTransformMatrixKHR instanceTransform = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f
            };

            VkAccelerationStructureInstanceKHR instance = {};
            instance.transform = instanceTransform;
            instance.instanceCustomIndex = 0;
            instance.mask = 0xFF;
            instance.instanceShaderBindingTableRecordOffset = 0;
            instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
            instance.accelerationStructureReference = bottomLevelASHandle;
            std::vector<VkAccelerationStructureInstanceKHR> instances = { instance };

            createBuffer(sizeof(VkAccelerationStructureInstanceKHR), 
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBufferRT.vkBuffer, instanceBufferRT.vkBufferMemory);

            VkBufferDeviceAddressInfo instanceBufferDeviceAddressInfo = {};
            instanceBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            instanceBufferDeviceAddressInfo.buffer = instanceBufferRT.vkBuffer;
            instanceBufferDeviceAddressInfo.pNext = NULL;
            instanceBufferRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &instanceBufferDeviceAddressInfo);

            if (instances.data() != nullptr) {
                if (vkMapMemory(device, instanceBufferRT.vkBufferMemory, 0, sizeof(VkAccelerationStructureInstanceKHR), 0, &dstData) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed vkMapMemory!");
                }
                memcpy(dstData, instances.data(), sizeof(VkAccelerationStructureInstanceKHR));
                vkUnmapMemory(device, instanceBufferRT.vkBufferMemory);
            }

            VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress = {};
            instanceDataDeviceAddress.deviceAddress = instanceBufferRT.vkBufferAdress;

            VkAccelerationStructureGeometryKHR asGeometryInfo = {};
            asGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            asGeometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            asGeometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            asGeometryInfo.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
            asGeometryInfo.geometry.instances.arrayOfPointers = VK_FALSE;
            asGeometryInfo.geometry.instances.data = instanceDataDeviceAddress;

            VkAccelerationStructureBuildGeometryInfoKHR asBuildSizeGeometryInfo = {};
            asBuildSizeGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
            asBuildSizeGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            asBuildSizeGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            asBuildSizeGeometryInfo.geometryCount = 1;
            asBuildSizeGeometryInfo.pGeometries = &asGeometryInfo;

            // aquire size to build acceleration structure
            const uint32_t primitiveCount = 1;

            VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo = {};
            asBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

            vkGetAccelerationStructureBuildSizesKHR_fnptr(
                device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &asBuildSizeGeometryInfo,
                &primitiveCount, &asBuildSizesInfo);


            // reserve memory to hold the acceleration structure
            createBuffer(asBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, topLevelAccelerationStructureMemoryRT.vkBuffer, topLevelAccelerationStructureMemoryRT.vkBufferMemory);

            VkBufferDeviceAddressInfo abufferDeviceAddressInfo = {};
            abufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            abufferDeviceAddressInfo.buffer = topLevelAccelerationStructureMemoryRT.vkBuffer;
            abufferDeviceAddressInfo.pNext = NULL;
            topLevelAccelerationStructureMemoryRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &abufferDeviceAddressInfo);

            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {};
            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            accelerationStructureCreateInfo.buffer = topLevelAccelerationStructureMemoryRT.vkBuffer;
            accelerationStructureCreateInfo.size = asBuildSizesInfo.accelerationStructureSize;
            accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

            if (vkCreateAccelerationStructureKHR_fnptr(device, &accelerationStructureCreateInfo, nullptr, &topLevelAS) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create top Level Acceleration Structure!");
            }

            // reserve memory to hold the acceleration structure
            createBuffer(asBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchMemoryRT.vkBuffer, scratchMemoryRT.vkBufferMemory);

            VkBufferDeviceAddressInfo scratchbufferDeviceAddressInfo = {};
            scratchbufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            scratchbufferDeviceAddressInfo.buffer = scratchMemoryRT.vkBuffer;
            scratchbufferDeviceAddressInfo.pNext = NULL;
            scratchMemoryRT.vkBufferAdress = vkGetBufferDeviceAddress(device, &scratchbufferDeviceAddressInfo);

            //buildAccelerationStrcture
            VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo = {};
            asBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
            asBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            asBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            asBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
            asBuildGeometryInfo.dstAccelerationStructure = topLevelAS;
            asBuildGeometryInfo.geometryCount = 1;
            asBuildGeometryInfo.pGeometries = &asGeometryInfo;
            asBuildGeometryInfo.scratchData.deviceAddress = scratchMemoryRT.vkBufferAdress;

            VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo = {};
            asBuildRangeInfo.primitiveCount = primitiveCount;
            asBuildRangeInfo.primitiveOffset = 0;
            asBuildRangeInfo.firstVertex = 0;
            asBuildRangeInfo.transformOffset = 0;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> asBuildRangeInfos =
            {
                &asBuildRangeInfo
            };

            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = commandPoolRT;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate command Buffers!");

            VkCommandBufferBeginInfo commandBufferBeginInfo = {};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
                throw std::runtime_error("failed to begin command Buffers!");

            // build the bottom-level acceleration structure
            vkCmdBuildAccelerationStructuresKHR_fnptr(commandBuffer, 1, &asBuildGeometryInfo, asBuildRangeInfos.data());

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to end command Buffers!");

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            VkFence fence = VK_NULL_HANDLE;

            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            vkCreateFence(device, &fenceInfo, nullptr, &fence);
            vkQueueSubmit(queue, 1, &submitInfo, fence);
            vkWaitForFences(device, 1, &fence, true, UINT64_MAX);

            vkDestroyFence(device, fence, nullptr);
            vkFreeCommandBuffers(device, commandPoolRT, 1, &commandBuffer);

            // Get bottom level acceleration structure handle for use in top level instances
            VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo = {};
            asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
            asDeviceAddressInfo.accelerationStructure = topLevelAS;

            topLevelASHandle = vkGetAccelerationStructureDeviceAddressKHR_fnptr(device, &asDeviceAddressInfo);

            // make sure bottom AS handle is valid
            if (topLevelASHandle == 0) {
                throw std::runtime_error("Invalid Handle to TLAS");
            }
        }

    }

    void createOffscreenBuffer()
    {
        //Creating Offsceen Buffer..
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = desiredSurfaceFormat;
        imageInfo.extent = { desiredWindowWidth, desiredWindowHeight, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if (vkCreateImage(device, &imageInfo, nullptr, &offScreenBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Create image Failed!");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, offScreenBuffer, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &offScreenBufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("AllocateMemory Failed!");
        }
        if (vkBindImageMemory(device, offScreenBuffer, offScreenBufferMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("AllocateMemory Failed!");
        }

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = desiredSurfaceFormat;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.image = offScreenBuffer;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &offScreenBufferView) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateImageView Failed!");
        }
    }

    void createRTDescriptorSetLayout()
    {
        //"Creating RT Descriptor Set Layout.."
        VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
        accelerationStructureLayoutBinding.binding = 0;
        accelerationStructureLayoutBinding.descriptorCount = 1;
        accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

        VkDescriptorSetLayoutBinding storageImageLayoutBinding = {};
        storageImageLayoutBinding.binding = 1;
        storageImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        storageImageLayoutBinding.descriptorCount = 1;
        storageImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

        std::vector<VkDescriptorSetLayoutBinding> bindings(
            { accelerationStructureLayoutBinding, storageImageLayoutBinding }
        );

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = (uint32_t)bindings.size();
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayoutRT) != VK_SUCCESS) {
            throw std::runtime_error("failed to create RT descriptor set layout!");
        }
    }

    void createRTDescriptorSets()
    {
        //"Creating RT Descriptor Set.."

        std::vector<VkDescriptorPoolSize> poolSizes(
            {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}});

        VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.maxSets = 1;
        descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        descriptorPoolInfo.pPoolSizes = poolSizes.data();

        if (vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPoolRT) != VK_SUCCESS)
        {
            throw std::runtime_error("CreateDescriptorPool failed!");
        }

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPoolRT;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayoutRT;

        if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSetRT) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        VkWriteDescriptorSetAccelerationStructureKHR descriptorSetAccelerationStructureInfo = {};
        descriptorSetAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        descriptorSetAccelerationStructureInfo.accelerationStructureCount = 1;
        descriptorSetAccelerationStructureInfo.pAccelerationStructures = &topLevelAS;


        VkWriteDescriptorSet accelerationStructureWrite = {};
        accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        accelerationStructureWrite.pNext = &descriptorSetAccelerationStructureInfo;
        accelerationStructureWrite.dstSet = descriptorSetRT;
        accelerationStructureWrite.dstBinding = 0;
        accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        accelerationStructureWrite.descriptorCount = 1;

        VkDescriptorImageInfo storageImageInfo = {};
        storageImageInfo.sampler = VK_NULL_HANDLE;
        storageImageInfo.imageView = offScreenBufferView;
        storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet outputImageWrite = {};
        outputImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        outputImageWrite.pNext = nullptr;
        outputImageWrite.dstSet = descriptorSetRT;
        outputImageWrite.dstBinding = 1;
        outputImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        outputImageWrite.descriptorCount = 1;
        outputImageWrite.pImageInfo = &storageImageInfo;

        std::vector<VkWriteDescriptorSet> descriptorWrites(
            { accelerationStructureWrite, outputImageWrite });

        vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        
    }

    void createRTPipelineLayout()
    {
        //"Creating RT Pipeline Layout.."
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayoutRT;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &raytracingPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create RT pipeline layout!");
        }
    }

    void createRTPipeline()
    {
        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR_fnptr = NULL;

        vkCreateRayTracingPipelinesKHR_fnptr = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
        if (vkCreateRayTracingPipelinesKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkCreateRayTracingPipelinesKHR!");
        }

        //"Creating RT Pipeline.."
        auto rgenShaderSrcCode = readFile("shaders/ray-generation.spv");
        auto rchitShaderSrcCode = readFile("shaders/ray-closest-hit.spv");
        auto rmissShaderSrcCode = readFile("shaders/ray-miss.spv");

        VkShaderModule rgenShaderModule = createShaderModule(rgenShaderSrcCode);
        VkShaderModule rchitShaderModule = createShaderModule(rchitShaderSrcCode);
        VkShaderModule rmissShaderModule = createShaderModule(rmissShaderSrcCode);

        VkPipelineShaderStageCreateInfo rayGenShaderStageInfo{};
        rayGenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        rayGenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        rayGenShaderStageInfo.module = rgenShaderModule;
        rayGenShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo rayChitShaderStageInfo{};
        rayChitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        rayChitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        rayChitShaderStageInfo.module = rchitShaderModule;
        rayChitShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo rayMissShaderStageInfo{};
        rayMissShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        rayMissShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        rayMissShaderStageInfo.module = rmissShaderModule;
        rayMissShaderStageInfo.pName = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            rayGenShaderStageInfo, rayMissShaderStageInfo, rayChitShaderStageInfo };


        VkRayTracingShaderGroupCreateInfoKHR rayGenGroup = {};
        rayGenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        rayGenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        rayGenGroup.generalShader = 0;
        rayGenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        rayGenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        rayGenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingShaderGroupCreateInfoKHR rayMissGroup = {};
        rayMissGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        rayMissGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        rayMissGroup.generalShader = 1;
        rayMissGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        rayMissGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        rayMissGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingShaderGroupCreateInfoKHR rayHitGroup = {};
        rayHitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        rayHitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        rayHitGroup.generalShader = 2;
        rayHitGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        rayHitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        rayHitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups = { rayGenGroup, rayMissGroup,
                                                                          rayHitGroup };

        VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipelineInfo.stageCount = (uint32_t)shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.groupCount = (uint32_t)shaderGroups.size();
        pipelineInfo.pGroups = shaderGroups.data();
        pipelineInfo.maxPipelineRayRecursionDepth = 1;
        pipelineInfo.layout = raytracingPipelineLayout;

        sbtGroupCount = shaderGroups.size();

        if (vkCreateRayTracingPipelinesKHR_fnptr(device, nullptr, nullptr, 1, &pipelineInfo, nullptr, &raytracingPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("CreateRayTracingPipelines failed!");
        }

        vkDestroyShaderModule(device, rgenShaderModule, nullptr);
        vkDestroyShaderModule(device, rchitShaderModule, nullptr);
        vkDestroyShaderModule(device, rmissShaderModule, nullptr);

    }

    uint32_t alignTo(uint32_t value, uint32_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    void createShaderBindingTable()
    {
        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR_fnptr = NULL;

        vkGetRayTracingShaderGroupHandlesKHR_fnptr = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR");
        if (vkGetRayTracingShaderGroupHandlesKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkGetRayTracingShaderGroupHandlesKHR!");
        }

        //"Creating Shader Binding Table.."
        sbtHandleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
        sbtHandleAlignment = rayTracingPipelineProperties.shaderGroupHandleAlignment;
        sbtHandleSizeAligned = alignTo(sbtHandleSize, sbtHandleAlignment);
        sbtSize = sbtGroupCount * sbtHandleSizeAligned;

        std::vector<uint8_t> sbtResults(sbtSize);

        if (vkGetRayTracingShaderGroupHandlesKHR_fnptr(device, raytracingPipeline, 0, sbtGroupCount, sbtSize, sbtResults.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("GetRayTracingShaderGroupHandles failed!");
        }

        void* dstData = nullptr;
        // create 3 separate buffers for each ray type
        createBuffer(sbtHandleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbtRayGenBuffer.vkBuffer, sbtRayGenBuffer.vkBufferMemory);
        VkBufferDeviceAddressInfo sbtRayGenBufferAddressInfo = {};
        sbtRayGenBufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        sbtRayGenBufferAddressInfo.buffer = sbtRayGenBuffer.vkBuffer;
        sbtRayGenBufferAddressInfo.pNext = NULL;
        sbtRayGenBuffer.vkBufferAdress = vkGetBufferDeviceAddress(device, &sbtRayGenBufferAddressInfo);
        if (sbtResults.data() != nullptr) {
            if (vkMapMemory(device, sbtRayGenBuffer.vkBufferMemory, 0, sbtHandleSize, 0, &dstData) != VK_SUCCESS)
            {
                throw std::runtime_error("failed vkMapMemory!");
            }
            memcpy(dstData, sbtResults.data(), sbtHandleSize);
            vkUnmapMemory(device, sbtRayGenBuffer.vkBufferMemory);
        }

        createBuffer(sbtHandleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbtRayMissBuffer.vkBuffer, sbtRayMissBuffer.vkBufferMemory);
        
        sbtRayGenBufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        sbtRayGenBufferAddressInfo.buffer = sbtRayMissBuffer.vkBuffer;
        sbtRayGenBufferAddressInfo.pNext = NULL;
        sbtRayMissBuffer.vkBufferAdress = vkGetBufferDeviceAddress(device, &sbtRayGenBufferAddressInfo);
        if (sbtResults.data() + sbtHandleSizeAligned != nullptr) {
            if (vkMapMemory(device, sbtRayMissBuffer.vkBufferMemory, 0, sbtHandleSize, 0, &dstData) != VK_SUCCESS)
            {
                throw std::runtime_error("failed vkMapMemory!");
            }
            memcpy(dstData, sbtResults.data() + sbtHandleSizeAligned, sbtHandleSize);
            vkUnmapMemory(device, sbtRayMissBuffer.vkBufferMemory);
        }

        createBuffer(sbtHandleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbtRayHitBuffer.vkBuffer, sbtRayHitBuffer.vkBufferMemory);
        
        sbtRayGenBufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        sbtRayGenBufferAddressInfo.buffer = sbtRayHitBuffer.vkBuffer;
        sbtRayGenBufferAddressInfo.pNext = NULL;
        sbtRayHitBuffer.vkBufferAdress = vkGetBufferDeviceAddress(device, &sbtRayGenBufferAddressInfo);
    
        if (sbtResults.data() + sbtHandleSizeAligned * 2 != nullptr) {
            if (vkMapMemory(device, sbtRayHitBuffer.vkBufferMemory, 0, sbtHandleSize, 0, &dstData) != VK_SUCCESS)
            {
                throw std::runtime_error("failed vkMapMemory!");
            }
            memcpy(dstData, sbtResults.data() + sbtHandleSizeAligned * 2, sbtHandleSize);
            vkUnmapMemory(device, sbtRayHitBuffer.vkBufferMemory);
        }
    }

    void createSwapChainRT()
    {
       
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = desiredSurfaceFormat;
        
        createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        createInfo.imageExtent.width = desiredWindowWidth;
        createInfo.imageExtent.height = desiredWindowHeight;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

      
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
       // }

        // createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChainRT) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChainRT, &imageCount, nullptr);
        swapChainImagesRT.resize(imageCount);
        if (vkGetSwapchainImagesKHR(device, swapChainRT, &imageCount, swapChainImagesRT.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to Get swap chain!");
        }

        swapChainImageFormatRT = surfaceFormat.format;
        swapChainExtentRT = extent;
       

        swapChainImageViewsRT.resize(swapChainImagesRT.size());

        for (size_t i = 0; i < swapChainImagesRT.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImagesRT[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = desiredSurfaceFormat;
           
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViewsRT[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }
    void InsertCommandImageBarrier(VkCommandBuffer commandBuffer,
        VkImage image,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        const VkImageSubresourceRange& subresourceRange)
    {
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    }
    void createTraceRayCommandBuffer()
    {
        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR_fnptr = NULL;

        vkCmdTraceRaysKHR_fnptr = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
        if (vkCmdTraceRaysKHR_fnptr == NULL)
        {
            throw std::runtime_error("failed to get function pointer for vkCmdTraceRaysKHR!");
        }

        
        VkImageCopy copyRegion = {};
        copyRegion.srcOffset = { 0, 0, 0 };
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.extent.depth = 1;
        copyRegion.extent.width = desiredWindowWidth;
        copyRegion.extent.height = desiredWindowHeight;
        copyRegion.dstOffset = { 0, 0, 0 };

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkStridedDeviceAddressRegionKHR rayGenSBT = {};
        rayGenSBT.deviceAddress = sbtRayGenBuffer.vkBufferAdress;
        rayGenSBT.stride = sbtHandleSizeAligned;
        rayGenSBT.size = sbtHandleSizeAligned;

        VkStridedDeviceAddressRegionKHR rayMissSBT = {};
        rayMissSBT.deviceAddress = sbtRayMissBuffer.vkBufferAdress;
        rayMissSBT.stride = sbtHandleSizeAligned;
        rayMissSBT.size = sbtHandleSizeAligned;

        VkStridedDeviceAddressRegionKHR rayHitSBT = {};
        rayHitSBT.deviceAddress = sbtRayHitBuffer.vkBufferAdress;
        rayHitSBT.stride = sbtHandleSizeAligned;
        rayHitSBT.size = sbtHandleSizeAligned;

        VkStridedDeviceAddressRegionKHR rayCallableSBT = {};

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPoolRT;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = (uint32_t)swapChainImagesRT.size();

       // std::vector<VkCommandBuffer> icommandBuffer =
        commandBuffersRT = 
            std::vector<VkCommandBuffer>(swapChainImagesRT.size());

        if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffersRT.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
   
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;

        for (uint32_t ii = 0; ii < swapChainImagesRT.size(); ++ii)
        {
            VkCommandBuffer commandBuffer = commandBuffersRT[ii];
            VkImage swapChainImage = swapChainImagesRT[ii];

            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

         

             // transition offscreen buffer into shader writeable state
            InsertCommandImageBarrier(commandBuffer, offScreenBuffer, 0, VK_ACCESS_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                subresourceRange);

            // record ray tracing
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                raytracingPipelineLayout, 0, 1, &descriptorSetRT, 0, 0);

            vkCmdTraceRaysKHR_fnptr(commandBuffer, &rayGenSBT, &rayMissSBT, &rayHitSBT, &rayCallableSBT,
                desiredWindowWidth, desiredWindowHeight, 1);

            // transition swapchain image into copy destination state
            InsertCommandImageBarrier(commandBuffer, swapChainImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                subresourceRange);

            // transition offscreen buffer into copy source state
            InsertCommandImageBarrier(commandBuffer, offScreenBuffer, VK_ACCESS_SHADER_WRITE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

            // copy offscreen buffer into swapchain image
            vkCmdCopyImage(commandBuffer, offScreenBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                swapChainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            // transition swapchain image into presentable state
            InsertCommandImageBarrier(commandBuffer, swapChainImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresourceRange);

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }

    }
   
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        //bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        //Structure controlling how many instances of memory will be allocated (Raytracing)
        VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
        memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        allocInfo.pNext = &memoryAllocateFlagsInfo; //newly added here

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

  
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

  
    void createSyncObjects() {
      

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphoreImageAvailable) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create SemaphoreImageAvailable");
        }
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphoreRenderingAvailable) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphoreRenderingAvailable");
        }

    }

   
    void drawFrameRT() {
       // vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChainRT, UINT64_MAX, semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            //recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffersRT[imageIndex];
        submitInfo.signalSemaphoreCount = 1;

        submitInfo.pSignalSemaphores = &semaphoreRenderingAvailable;

        if (vkQueueSubmit(queue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit Queue!");
        }

    

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pWaitSemaphores = &semaphoreRenderingAvailable;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChainRT;

        presentInfo.pImageIndices = &imageIndex;


       

        result = vkQueuePresentKHR(queue, &presentInfo);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        if (vkQueueWaitIdle(queue) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to Queue Wait Idle!");
        }


       // currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == desiredSurfaceFormat && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {

        //for raytracing
        bool raytracingSupportedDevice = false;
        VkPhysicalDeviceAccelerationStructureFeaturesKHR rtAccelerationFeatures = {};
        rtAccelerationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 deviceFeatures2;
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &rtAccelerationFeatures;
        vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

        if (rtAccelerationFeatures.accelerationStructure == VK_TRUE)
        {
            raytracingSupportedDevice = true;
        }

       // QueueFamilyIndices indices = findQueueFamilies(device);

       // bool extensionsSupported = checkDeviceExtensionSupport(device);

       // bool swapChainAdequate = false;
        /*if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }*/

        return raytracingSupportedDevice;
        //return indices.isComplete() && extensionsSupported && swapChainAdequate && raytracingSupportedDevice;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}