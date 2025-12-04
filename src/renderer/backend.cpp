#include "backend.h"
#include "util/log.h"
#include <cstring>

static bool ENABLE_VK_VALIDATION_LAYERS = true;

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    ASSERT(pUserData == NULL, "User data has not been set up to be handled");
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        WARN("%s[VULKAN] [%s]%s %s",
            LOGCOLOR_YELLOW,
            (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ? "GENERAL" :
                (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ? "VALIDATION" : "PERFORMANCE")),
            LOGCOLOR_RESET,
            pCallbackData->pMessage);
    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        printf("%s[FATAL] [VULKAN] [%s]%s %s",
            LOGCOLOR_RED,
            (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ? "GENERAL" :
                (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ? "VALIDATION" : "PERFORMANCE")),
            LOGCOLOR_RESET,
            pCallbackData->pMessage);
		exit(1);
    }
    return VK_FALSE;
}

VulkanBackend::VulkanBackend() {
    if (!(initMetadata() && initCore())) FATAL("Unable to intialize vulkan backend");
}

bool VulkanBackend::checkValidationLayerSupport() {
    // grab all available layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties* availableLayers = (VkLayerProperties*)calloc(layerCount, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    // check if layers in validation layers exist in the available layers
    for (size_t i = 0; i < metadata.validation.size(); i++) {
        bool layerFound = false;
        for (size_t j = 0; j < layerCount; j++) {
            if (strcmp(metadata.validation[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            free(availableLayers);
            return false;
        }
    }
    free(availableLayers);
    return true;
}

VulkanFamilyGroup VulkanBackend::findQueueFamilies(VkPhysicalDevice gpu) {
    VulkanFamilyGroup group{};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)calloc(queueFamilyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies);
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            group.graphics = i;
            group.exists = true;
            break;
        }
    }
    free(queueFamilies);
    return group;
}

bool VulkanBackend::checkGPUExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)calloc(extensionCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);
    for (size_t i = 0; i < metadata.extensions.device.size(); i++) {
        bool extensionFound = false;
        for (size_t j = 0; j < extensionCount; j++) {
            if (strcmp(metadata.extensions.device[i], availableExtensions[j].extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) {
            free(availableExtensions);
            return false;
        }
    }
    free(availableExtensions);
    return true;
}

bool VulkanBackend::initCore() {
    return 
        initShaders() &&
        initGeneral() &&
        initGeometry() &&
        initScheduler() &&
        initRenderContext();
}

bool VulkanBackend::initMetadata() {
    metadata.validation.push_back("VK_LAYER_KHRONOS_validation");
    if (ENABLE_VK_VALIDATION_LAYERS) metadata.extensions.required.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return true;
}

bool VulkanBackend::initShaders() {
    // TODO:
    return true;
}

bool VulkanBackend::initGeneral() {
    // error check for validation layer support
    if (ENABLE_VK_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		WARN("Requested validation layers are not available");
        ENABLE_VK_VALIDATION_LAYERS = false;
	}

    // create app info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Jason's Super Awesome Spectral Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Jason's Super Awesome Spectral Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    // create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32_t)(metadata.extensions.required.size());
    createInfo.ppEnabledExtensionNames = metadata.extensions.required.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (ENABLE_VK_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = metadata.validation.size();
        createInfo.ppEnabledLayerNames = metadata.validation.data();

        // set up additional debug callback for the creation of the instance
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = vulkanDebugCallback;
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    // create instance
    VkResult result = vkCreateInstance(&createInfo, NULL, &(core.general.instance));
    if (result != VK_SUCCESS) {
		FATAL("Failed to create vulkan instance");
		return false;
	}

	// create validation messenger
    if (ENABLE_VK_VALIDATION_LAYERS) {
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = vulkanDebugCallback;
		createInfo.pUserData = NULL;
		PFN_vkCreateDebugUtilsMessengerEXT messenger_extension = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(core.general.instance, "vkCreateDebugUtilsMessengerEXT");
		if (messenger_extension == NULL) {
			FATAL("Failed to set up debug messenger");
			return false;
		}
		messenger_extension(core.general.instance, &createInfo, NULL, &(core.general.messenger));
	}

	// pick gpu
	uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(core.general.instance, &deviceCount, NULL);
    if (deviceCount == 0) {
		FATAL("No devices with vulkan support were found");
		return false;
	}
    VkPhysicalDevice* devices = (VkPhysicalDevice*)calloc(deviceCount, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(core.general.instance, &deviceCount, devices);
    uint32_t score = 0;
    uint32_t ind = 0;
    for (uint32_t i = 0; i < deviceCount; i++) {
        // check if device is suitable
        VulkanFamilyGroup families = findQueueFamilies(devices[i]);
        if (!families.exists) continue;
        if (!checkGPUExtensionSupport(devices[i])) continue;

        uint32_t curr_score = 0;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) curr_score += 10000;
        curr_score += deviceProperties.limits.maxImageDimension2D;
        if (!deviceFeatures.geometryShader) curr_score = 0;
        if (!deviceFeatures.samplerAnisotropy) curr_score = 0;
        if (curr_score > score) {
            score = curr_score;
            ind = i;
        }
    }
    if (score == 0) {
		FATAL("A suitable GPU could not be found");
		return false;
	}
    VkPhysicalDeviceProperties dp;
    vkGetPhysicalDeviceProperties(devices[ind], &dp);
	strcpy(core.general.gpuname, dp.deviceName);
    core.general.gpu = devices[ind];
    free(devices);

	// create device interface
	VulkanFamilyGroup families = findQueueFamilies(core.general.gpu);
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = families.graphics;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = metadata.extensions.device.size();
    deviceCreateInfo.ppEnabledExtensionNames  = metadata.extensions.device.data();
    if (ENABLE_VK_VALIDATION_LAYERS) {
        deviceCreateInfo.enabledLayerCount = metadata.validation.size();
        deviceCreateInfo.ppEnabledLayerNames = metadata.validation.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }
    result = vkCreateDevice(core.general.gpu, &deviceCreateInfo, NULL, &(core.general.interface));
	if (result != VK_SUCCESS) {
		FATAL("Failed to create logical device");
		return false;
	}
    return true;
}

bool VulkanBackend::initGeometry() {
    // TODO:
    return true;
}

bool VulkanBackend::initScheduler() {
    return
        initSyncro() &&
        initCommands() &&
        initQueue();
}

bool VulkanBackend::initSyncro() {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult result = vkCreateFence(
		core.general.interface,
		&fenceInfo, NULL, &(core.scheduler.syncro));
    if (result != VK_SUCCESS) {
	    FATAL("Failed to create fence");
		return false;
	}
	return true;
}

bool VulkanBackend::initCommands() {
    // create command pool
    VulkanFamilyGroup queueFamilyIndices = findQueueFamilies(core.general.gpu);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    VkResult result = vkCreateCommandPool(
		core.general.interface,
		&poolInfo, NULL, &(core.scheduler.command.pool));
    if (result != VK_SUCCESS) {
		FATAL("Failed to create command pool!");
		return false;
	}

	// create command buffers
	VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = core.scheduler.command.pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    result = vkAllocateCommandBuffers(
		core.general.interface,
		&allocInfo,
		&(core.scheduler.command.command));
    if (result != VK_SUCCESS) {
		FATAL("Failed to create command buffer");
		return false;
	}

    return true;
}

bool VulkanBackend::initQueue() {
	VulkanFamilyGroup families = findQueueFamilies(core.general.gpu);
    vkGetDeviceQueue(
		core.general.interface,
		families.graphics,
		0,
		&(core.scheduler.queue));
    return true;
}

bool VulkanBackend::initRenderContext() {
    return 
        initTargets() &&
        initRenderData() &&
        initPipeline();
}

bool VulkanBackend::initTargets() {
    // TODO:
    return true;
}

bool VulkanBackend::initRenderData() {
    // TODO:
    return true;
}

bool VulkanBackend::initPipeline() {
    // TODO:
    return true;
}
