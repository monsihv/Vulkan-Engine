#define VMA_IMPLEMENTATION
#include "headers/base.h"

#include <assert.h>

static u32 clampU32(u32 value, u32 lo, u32 hi) {
    return value < lo ? lo : (value > hi ? hi : value);
}

void createInstance(Renderer *renderer) {
    constexpr VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "type type",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "engine",
        .engineVersion = VK_MAKE_VERSION(1, 0 , 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    Arena *scratch = renderer->scratch;
    u64 scratchPos = scratch->pos;

    u32 glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    u32 enabledExtensionCount = glfwExtensionCount + 2;
    const char **enabledExtensions = pushArray(scratch, const char *, enabledExtensionCount);
    for (u32 i = 0; i < glfwExtensionCount; ++i) {
        enabledExtensions[i] = glfwExtensions[i];
    }
    enabledExtensions[glfwExtensionCount + 0] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    enabledExtensions[glfwExtensionCount + 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    const char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]),
        .ppEnabledLayerNames = validationLayers,
        .enabledExtensionCount = enabledExtensionCount,
        .ppEnabledExtensionNames = enabledExtensions
    };

    VK_CHECK(vkCreateInstance(&instanceInfo, NULL, &renderer->instance));

    arenaPopTo(scratch, scratchPos);
}

void createSurface(Renderer *renderer) {
    VK_CHECK(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL, &renderer->surface));
}

void selectGPU(Renderer *renderer) {
    Arena *scratch = renderer->scratch;
    u64 scratchPos = scratch->pos;

    u32 physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(renderer->instance, &physicalDeviceCount, NULL));
    VkPhysicalDevice *physicalDevices = pushArray(scratch, VkPhysicalDevice, physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(renderer->instance, &physicalDeviceCount, physicalDevices));

    renderer->GPU = physicalDevices[1];

    arenaPopTo(scratch, scratchPos);
}

void createLogicalDevice(Renderer *renderer) {
    Arena *scratch = renderer->scratch;
    u64 scratchPos = scratch->pos;

    u32 qfpLength = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(renderer->GPU, &qfpLength, NULL);
    VkQueueFamilyProperties *queueFamilyProperties = pushArray(scratch, VkQueueFamilyProperties, qfpLength);
    vkGetPhysicalDeviceQueueFamilyProperties(renderer->GPU, &qfpLength, queueFamilyProperties);

    u32 queueIndex = 0;
    for (u32 i = 0; i < qfpLength; ++i) {
        VkBool32 presentSupport = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(renderer->GPU, i, renderer->surface, &presentSupport));

        if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
            queueIndex = i;
            break;
        }
    }

    arenaPopTo(scratch, scratchPos);

    float queuePriority = 0.5f;

    VkDeviceQueueCreateInfo queueInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    // Feature chain: features2 -> vulkan11 -> vulkan13 -> extendedDynamicState
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = NULL,
        .extendedDynamicState = VK_TRUE
    };

    VkPhysicalDeviceVulkan13Features vulkan13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &extendedDynamicStateFeatures,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE
    };

    VkPhysicalDeviceVulkan11Features vulkan11Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vulkan13Features,
        .shaderDrawParameters = VK_TRUE
    };

    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan11Features
    };

    const char *requiredDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features2,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = sizeof(requiredDeviceExtensions) / sizeof(requiredDeviceExtensions[0]),
        .ppEnabledExtensionNames = requiredDeviceExtensions
    };

    VK_CHECK(vkCreateDevice(renderer->GPU, &deviceInfo, NULL, &renderer->device));
    vkGetDeviceQueue(renderer->device, queueIndex, 0, &renderer->graphicsQueue);
    renderer->graphicsQueueIndex = queueIndex;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = renderer->GPU;
    allocatorInfo.device = renderer->device;
    allocatorInfo.instance = renderer->instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    if (vmaCreateAllocator(&allocatorInfo, &renderer->allocator) != VK_SUCCESS) {
        fprintf(stderr, "allocator not made properly\n");
        exit(1);
    }
}

void createSwapchain(Renderer *renderer) {
    Arena *scratch = renderer->scratch;
    u64 scratchPos = scratch->pos;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->GPU, renderer->surface, &surfaceCapabilities));

    u32 formatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->GPU, renderer->surface, &formatCount, NULL));
    VkSurfaceFormatKHR *availableFormats = pushArray(scratch, VkSurfaceFormatKHR, formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->GPU, renderer->surface, &formatCount, availableFormats));

    u32 presentModeCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->GPU, renderer->surface, &presentModeCount, NULL));
    VkPresentModeKHR *availablePresentModes = pushArray(scratch, VkPresentModeKHR, presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->GPU, renderer->surface, &presentModeCount, availablePresentModes));

    VkSurfaceFormatKHR *format = NULL;
    for (u32 i = 0; i < formatCount; ++i) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format = &availableFormats[i];
            break;
        }
    }
    assert(format);

    bool hasFifo = false;
    bool hasMailbox = false;
    for (u32 i = 0; i < presentModeCount; ++i) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_FIFO_KHR) hasFifo = true;
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) hasMailbox = true;
    }
    assert(hasFifo);
    (void)hasFifo;
    const VkPresentModeKHR presentMode = hasMailbox ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;

    glfwGetFramebufferSize(renderer->window, (int*)&renderer->width, (int*)&renderer->height);

    auto minImageExtentWidth = surfaceCapabilities.minImageExtent.width;
    auto maxImageExtentWidth = surfaceCapabilities.maxImageExtent.width;
    auto minImageExtentHeight = surfaceCapabilities.minImageExtent.height;
    auto maxImageExtentHeight = surfaceCapabilities.maxImageExtent.height;
    renderer->swapchainExtent = {clampU32(renderer->width, minImageExtentWidth, maxImageExtentWidth),
                                 clampU32(renderer->height, minImageExtentHeight, maxImageExtentHeight)};

    u32 minImageCount = surfaceCapabilities.minImageCount;
    auto maxImageCount = surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0) {
        if (minImageCount + 1 > maxImageCount) {
            minImageCount = maxImageCount;
        }
        else {
            minImageCount++;
        }
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = renderer->surface,
        .minImageCount = minImageCount,
        .imageFormat = format->format,
        .imageColorSpace = format->colorSpace,
        .imageExtent = renderer->swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    VK_CHECK(vkCreateSwapchainKHR(renderer->device, &swapchainInfo, NULL, &renderer->swapchain));

    u32 imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &imageCount, NULL));
    if (imageCount > MaxSwapchainImages) {
        fprintf(stderr, "swapchain has %u images, MaxSwapchainImages is %u\n", imageCount, MaxSwapchainImages);
        exit(1);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &imageCount, renderer->swapchainImages));
    renderer->swapchainImageCount = imageCount;

    renderer->swapchainSurfaceFormat = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    arenaPopTo(scratch, scratchPos);
}

void createSwapchainImageViews(Renderer *renderer) {
    VkImageViewCreateInfo imageViewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = renderer->swapchainSurfaceFormat.format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    for (u32 i = 0; i < renderer->swapchainImageCount; ++i) {
        imageViewInfo.image = renderer->swapchainImages[i];
        VK_CHECK(vkCreateImageView(renderer->device, &imageViewInfo, NULL, &renderer->swapchainImageViews[i]));
    }
}

void cleanupSwapchain(Renderer *renderer) {
    for (u32 i = 0; i < renderer->swapchainImageCount; ++i) {
        vkDestroyImageView(renderer->device, renderer->swapchainImageViews[i], NULL);
        renderer->swapchainImageViews[i] = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
}

void recreateSwapchain(Renderer *renderer) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(renderer->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(renderer->device);

    cleanupSwapchain(renderer);
    cleanupDepthImage(renderer);

    createSwapchain(renderer);
    createSwapchainImageViews(renderer);
    createDepthImage(renderer);
}

void createDepthImage(Renderer *renderer) {
    VkImageCreateInfo depthImageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .extent = {renderer->swapchainExtent.width, renderer->swapchainExtent.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocationInfo = {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    auto& depthImage = renderer->depthBuffer.image;
    auto& depthImageMemory = renderer->depthBuffer.memory;
    if (vmaCreateImage(renderer->allocator, &depthImageInfo,
                &allocationInfo, &depthImage, &depthImageMemory, NULL) != VK_SUCCESS) {
        fprintf(stderr, "depth buffer not made properly\n");
        exit(1);
    }

    VkImageViewCreateInfo imageViewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VK_CHECK(vkCreateImageView(renderer->device, &imageViewInfo, NULL, &renderer->depthBuffer.imageView));
}

void cleanupDepthImage(Renderer *renderer) {
    destroyImage(renderer, renderer->depthBuffer, true);
}
