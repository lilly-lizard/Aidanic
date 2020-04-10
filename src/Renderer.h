#pragma once

#include "tools/VkHelper.h"
#include "ImguiVk.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include <string>

class Aidanic;
class ImGuiVk;

class Renderer {
    friend ImGuiVk;

public:
    void init(Aidanic* app, std::vector<const char*>& requiredExtensions, Model model, glm::mat4 viewInverse, glm::mat4 projInverse, glm::vec3 cameraPos);
    void drawFrame(bool framebufferResized, glm::mat4 viewInverse, glm::mat4 projInverse, glm::vec3 cameraPos, bool renderImGui = false, ImGuiVk* imGuiRenderer = nullptr);
    void cleanUp();

private:
#pragma region VARIABLES

    Aidanic* aidanicApp;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    VkMemoryRequirements memoryRequirements{};

    struct {
        VkQueue graphics;
        VkQueue present;
    } queues;

    struct {
        VkSwapchainKHR swapchain;
        std::vector<VkImage> images;
        int numImages = 0;
        VkFormat format;
        VkExtent2D extent;
        std::vector<VkImageView> imageViews;
    } swapchain;

    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};
    Vk::StorageImage renderImage;
    Vk::AccelerationStructure bottomLevelAS, topLevelAS;
    Vk::Buffer shaderBindingTable;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffersRender;
    std::vector<VkCommandBuffer> commandBuffersImageCopy;

    Vk::Buffer vertexBuffer, indexBuffer, uboBuffer;

    struct UniformData {
        glm::mat4 viewInverse = glm::mat4(1.0f);
        glm::mat4 projInverse = glm::mat4(1.0f);
        glm::vec4 cameraPos = glm::vec4(0.0f);
    } uniformData;

    std::vector<VkSemaphore> semaphoresImageAvailable, semaphoresRenderFinished, semaphoresImGuiFinished, semaphoresImageCopyFinished;
    std::vector<VkFence> inFlightFences, imagesInFlight;
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

    void createInstance(std::vector<const char*>& requiredExtensions);
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
    void createCommandBuffersRender();
    void createCommandBuffersImageCopy();
    void createBottomLevelAccelerationStructure(const VkGeometryNV* geometries);
    void createTopLevelAccelerationStructure();

    // main loop

    void updateUniformBuffer(glm::mat4 viewInverse, glm::mat4 projInverse, glm::vec4 cameraPos, uint32_t swapchainIndex);
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
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void recordImageLayoutTransition(VkCommandBuffer& commandBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void createBuffer(Vk::Buffer& buffer, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size);
    void uploadBufferDeviceLocal(Vk::Buffer& buffer, void* data, VkDeviceSize size, VkDeviceSize bufferOffset = 0);
    void uploadBufferHostVisible(Vk::Buffer& buffer, void* data, VkDeviceSize size, VkDeviceSize bufferOffset = 0);
    void destroyBuffer(Vk::Buffer& buffer);

    VkDeviceSize getUBOOffsetAligned(VkDeviceSize stride);

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

