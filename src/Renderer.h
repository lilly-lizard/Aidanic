#pragma once

/*
    todo: replace error checks with macro or something?
    write rtx code
    write descriptor set allocation system
*/

#include "tools/Log.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include <optional>
#include <string>

// Check vulkan result macro
#define VK_CHECK_RESULT(result, ...) if (result != VK_SUCCESS) { AID_TRACE("vkResult = {}", result); AID_ERROR(__VA_ARGS__); }

#define VK_ALLOCATOR nullptr

struct Vertex { glm::vec3 pos; };
struct Model {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class Aidanic;

class Renderer {
public:
    void init(Aidanic* app, Model model);
    void drawFrame(bool framebufferResized);
    void cleanUp();

private:
#pragma region STRUCT_DEFINITIONS

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct Buffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDeviceSize size = static_cast<VkDeviceSize>(0);
    };

    struct StorageImage {
        VkDeviceMemory memory;
        VkImage image;
        VkImageView view;
        VkFormat format;
    };

    struct AccelerationStructure {
        VkDeviceMemory memory;
        VkAccelerationStructureNV accelerationStructure;
        uint64_t handle;
    };

    struct BLASInstance {
        glm::mat3x4 transform;
        uint32_t instanceId : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

#pragma endregion
#pragma region VARIABLES

    Aidanic* aidanicApp;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    struct {
        VkQueue graphics;
        VkQueue present;
    } queues;

    struct {
        VkSwapchainKHR swapchain;
        std::vector<VkImage> images;
        int numImages = 0;
        VkFormat imageFormat;
        VkExtent2D extent;
        std::vector<VkImageView> imageViews;
    } swapchain;

    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};
    StorageImage renderImage;
    AccelerationStructure bottomLevelAS, topLevelAS;
    Buffer shaderBindingTable;
    std::vector<VkShaderModule> shaderModules;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffersDraw;

    Buffer vertexBuffer, indexBuffer, uboBuffer;

    struct {
        glm::mat4 viewInverse = glm::mat4(1.f, 0.f, 0.f, 0.f,
                                          0.f, 1.f, 0.f, 0.f,
                                          0.f, 0.f, 1.f, 0.f,
                                          0.f, 0.f, 2.5f, 1.f);
        glm::mat4 projInverse = glm::mat4(1.0264f, 0.f, 0.f, 0.f,
                                          0.f, 0.5774f, 0.f, 0.f,
                                          0.f, 0.f, 0.f, -9.9980f,
                                          0.f, 0.f, -1.f, 10.f);
    } uniformData;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    const std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::array<const char*, 3> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_NV_RAY_TRACING_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
    };
    const std::array<const char*, 1> instanceExtensions = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    };

#pragma endregion
#pragma region FUNCTIONS

    // initialization

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createCommandPool();
    void createImageViews();
    void createSyncObjects();

    void createScene(Model model);
    void createRenderImage();
    void createRayTracingPipeline();
    void createShaderBindingTable();
    void createDescriptorSets();
    void createCommandBuffers();

    void createBottomLevelAccelerationStructure(const VkGeometryNV* geometries);
    void createTopLevelAccelerationStructure();

    // main loop

    void recreateSwapChain();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    // clean up

    void cleanupSwapChain();

    // helper

    bool checkValidationLayerSupport();
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void testDebugMessenger();
    
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    static std::vector<char> readFile(const std::string filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkPipelineShaderStageCreateInfo loadShader(const std::string filename, VkShaderStageFlagBits stage);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void recordImageLayoutTransition(VkCommandBuffer& commandBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void createBuffer(Buffer& buffer, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size);
    void uploadBufferDeviceLocal(Buffer& buffer, void* data, VkDeviceSize size);
    void uploadBufferHostVisible(Buffer& buffer, void* data, VkDeviceSize size);
    void destroyBuffer(Buffer& buffer);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // vulkan pointers

    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV;
    PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV;
    PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV;
    PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV;
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV;
    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV;
    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV;
    PFN_vkGetRayTracingShaderGroupHandlesNV vkGetRayTracingShaderGroupHandlesNV;
    PFN_vkCmdTraceRaysNV vkCmdTraceRaysNV;

#pragma endregion
};
