#include "headers/base.h"

#include <ranges>
#include <algorithm>

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
    auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties,
            [](auto const& qfp) {
                return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlagBits{};
            });
    auto graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(),
                                                             graphicsQueueFamilyProperty));
    float queuePriority {0.5f};

    vk::DeviceQueueCreateInfo queueInfo {
        .queueFamilyIndex = graphicsIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::PhysicalDeviceFeatures deviceFeatures;

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
    renderer->graphicsQueue = vk::raii::Queue(renderer->device, graphicsIndex, 0);
}
