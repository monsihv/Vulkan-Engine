#include "headers/base.h"

#include <ranges>
#include <algorithm>
#include <cassert>

void createInstance(Renderer *renderer) {
    constexpr vk::ApplicationInfo appInfo {
        .pApplicationName = "type type",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "engine",
        .engineVersion = VK_MAKE_VERSION(1, 0 , 0),
        .apiVersion = vk::ApiVersion13
    };

    uint32_t glfwExtensionCount {};
    auto glfwExtensions {glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};
    std::vector<const char*> enabledExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    enabledExtensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
    enabledExtensions.push_back(vk::EXTDebugUtilsExtensionName);

    const std::vector<char const*> validationLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    vk::InstanceCreateInfo instanceInfo {
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data()
    };

    renderer->instance = vk::raii::Instance(renderer->context, instanceInfo);
}

void createSurface(Renderer *renderer) {
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(*renderer->instance, renderer->window, nullptr, &surface);

    renderer->surface = vk::raii::SurfaceKHR(renderer->instance, surface);
}

void selectGPU(Renderer *renderer) {
    auto physicalDevices = renderer->instance.enumeratePhysicalDevices();
    renderer->GPU = std::move(physicalDevices[1]);
}

void createLogicalDevice(Renderer *renderer) {
    auto queueFamilyProperties = renderer->GPU.getQueueFamilyProperties();

    auto qfpLength = queueFamilyProperties.size();
    uint32_t queueIndex {};
    for (uint32_t i {}; i < qfpLength; ++i) {
        if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) && 
                renderer->GPU.getSurfaceSupportKHR(i, *renderer->surface)) {
            queueIndex = i;
            break;
        }
    }

    float queuePriority {0.5f};

    vk::DeviceQueueCreateInfo queueInfo {
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    //vk::PhysicalDeviceFeatures deviceFeatures;

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain {
                           {},
                           {.shaderDrawParameters = true},
                           {.dynamicRendering = true},
                           {.extendedDynamicState = true}
                       };

    std::vector<const char*> requiredDeviceExtensions {vk::KHRSwapchainExtensionName};

    vk::DeviceCreateInfo deviceInfo {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = requiredDeviceExtensions.data()
    };

    renderer->device = vk::raii::Device(renderer->GPU, deviceInfo);
    renderer->graphicsQueue = vk::raii::Queue(renderer->device, queueIndex, 0);
}

void createSwapchain(Renderer *renderer) {
    auto surfaceCapabilities = renderer->GPU.getSurfaceCapabilitiesKHR(*renderer->surface);
    auto availableFormats = renderer->GPU.getSurfaceFormatsKHR(*renderer->surface);
    auto availablePresentModes = renderer->GPU.getSurfacePresentModesKHR(*renderer->surface);

    const auto format = std::ranges::find_if(availableFormats,
            [](const auto& format) {
                return format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            });

    assert(std::ranges::any_of(availablePresentModes, 
                [](const auto& presentMode) {
                    return presentMode == vk::PresentModeKHR::eFifo;
                }));
    const auto presentMode = std::ranges::any_of(availablePresentModes,
            [](const auto& presentMode) {
                return presentMode == vk::PresentModeKHR::eMailbox;
            }) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;

    auto minImageExtentWidth = surfaceCapabilities.minImageExtent.width;
    auto maxImageExtentWidth = surfaceCapabilities.maxImageExtent.width;
    auto minImageExtentHeight = surfaceCapabilities.minImageExtent.height;
    auto maxImageExtentHeight = surfaceCapabilities.maxImageExtent.height;
    renderer->swapchainExtent = {std::clamp<uint32_t>(renderer->width, minImageExtentWidth, maxImageExtentWidth),
                                 std::clamp<uint32_t>(renderer->height, minImageExtentHeight, maxImageExtentHeight)};

    uint32_t minImageCount = surfaceCapabilities.minImageCount;
    auto maxImageCount = surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0) {
        if (minImageCount + 1 > maxImageCount) {
            minImageCount = maxImageCount;
        }
        else {
            minImageCount++;
        }
    }

    vk::SwapchainCreateInfoKHR swapchainInfo {
        .surface = *renderer->surface,
        .minImageCount = minImageCount,
        .imageFormat = format->format,
        .imageColorSpace = format->colorSpace,
        .imageExtent = renderer->swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true
    };

    swapchainInfo.oldSwapchain = nullptr;

    renderer->swapchain = vk::raii::SwapchainKHR(renderer->device, swapchainInfo);
    renderer->swapchainImages = renderer->swapchain.getImages();
    renderer->swapchainSurfaceFormat = {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};
}

void createSwapchainImageViews(Renderer *renderer) {
    vk::ImageViewCreateInfo imageViewInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = renderer->swapchainSurfaceFormat.format,
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    imageViewInfo.components = {vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity};

    for (auto& image : renderer->swapchainImages) {
        imageViewInfo.image = image;
        renderer->swapchainImageViews.emplace_back(renderer->device, imageViewInfo);
    }
}
