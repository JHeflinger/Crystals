#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

struct VulkanFamilyGroup {
    uint32_t graphics;
    bool exists;
};

struct VulkanGeneral {
    VkDebugUtilsMessengerEXT messenger;
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice interface;
    char gpuname[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
};

struct VulkanCommand {
    VkCommandPool pool;
    VkCommandBuffer command;
};

struct VulkanScheduler {
    VkFence syncro;
    VulkanCommand command;
    VkQueue queue;
};

struct VulkanPipeline {
    VkPipeline* pipeline;
    VkPipelineLayout* layout;
};

struct VulkanImage {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
};

struct VulkanDescriptor {
    VkDescriptorPool pool;
    VkDescriptorSet set;
    VkDescriptorSetLayout layout;
};

struct VulkanDataBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct VulkanUBO {
    VulkanDataBuffer objects;
    void* mapped;
};

struct VulkanRenderData {
    VulkanDescriptor* descriptors;
    VulkanUBO ubo;
    VulkanDataBuffer ssbo;
};

struct VulkanRenderContext {
    VulkanPipeline pipeline;
    VulkanRenderData renderdata;
    VulkanImage target;
};

struct VulkanCore {
    VulkanGeneral general;
    VulkanScheduler scheduler;
    VulkanRenderContext context;
};

struct VulkanExtensionData {
    std::vector<const char*> required;
    std::vector<const char*> device;
};

struct VulkanMetadata {
    std::vector<const char*> validation;
    VulkanExtensionData extensions;
};

class VulkanBackend {
public:
    VulkanBackend();
public:
    std::string gpuName() { return std::string(core.general.gpuname); }
public:
    bool checkValidationLayerSupport();
    VulkanFamilyGroup findQueueFamilies(VkPhysicalDevice gpu);
    bool checkGPUExtensionSupport(VkPhysicalDevice gpu);
private:
    bool initCore();
    bool initMetadata();
    bool initShaders();
    bool initGeneral();
    bool initGeometry();
    bool initScheduler();
    bool initSyncro();
    bool initCommands();
    bool initQueue();
    bool initRenderContext();
    bool initTargets();
    bool initRenderData();
    bool initPipeline();
private:
    VulkanCore core;
    VulkanMetadata metadata;
};
