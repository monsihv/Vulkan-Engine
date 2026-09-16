#include "headers/base.h"
#include "headers/buffer.h"

#include <stdio.h>

uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memoryProperties = gpu.getMemoryProperties();

    for (uint32_t i {}; i < memoryProperties.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
    }

    fprintf(stderr, "failed to find suitable memory type!");
    exit(1);
}

Buffer createBuffer(Renderer *renderer, const void *data, vk::DeviceSize size,
                    vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags) {
    Buffer output;

    vk::BufferCreateInfo bufferInfo {
        .size = size,
        .usage = usageFlags,
        .sharingMode = vk::SharingMode::eExclusive
    };

    output.buffer = vk::raii::Buffer(renderer->device, bufferInfo);

    auto memoryRequirements = output.buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memoryInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(*renderer->GPU, memoryRequirements.memoryTypeBits, memoryFlags)
    };

    output.memory = vk::raii::DeviceMemory(renderer->device, memoryInfo);
    output.buffer.bindMemory(output.memory, 0);
    output.map = output.memory.mapMemory(0, bufferInfo.size);
    if (data && (memoryFlags & vk::MemoryPropertyFlagBits::eHostCoherent)) memcpy(output.map, data, bufferInfo.size);

    return output;
}

void copyBuffer(Renderer *renderer, vk::raii::Buffer& src, vk::raii::Buffer& dst,
                vk::DeviceSize size, vk::raii::CommandBuffer& commandBuffer) {
    commandBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    vk::BufferCopy bufferCopy {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    commandBuffer.copyBuffer(src, dst, bufferCopy);

    commandBuffer.end();

    vk::SubmitInfo submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer
    };
    renderer->graphicsQueue.submit(submitInfo, nullptr);
    renderer->device.waitIdle();
}

void createDescriptorPools(Renderer *renderer) {
    vk::DescriptorPoolSize descriptorPoolSize {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = MaxFramesInFlight
    };

    auto deviceProperties = renderer->GPU.getProperties();
    vk::DescriptorPoolCreateInfo descriptorPoolInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = deviceProperties.limits.maxDescriptorSetUniformBuffers,
        .poolSizeCount = 1,
        .pPoolSizes = &descriptorPoolSize
    };

    renderer->uniformBufferDescriptorPool = vk::raii::DescriptorPool(renderer->device, descriptorPoolInfo);
}

void createUniformBufferDescriptorSets(Renderer *renderer) {
    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
    };

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {
        .bindingCount = 1,
        .pBindings = &descriptorSetLayoutBinding
    };

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.reserve(MaxFramesInFlight);
    renderer->cameraBufferDescriptorSetLayouts.reserve(MaxFramesInFlight);
    for (uint32_t i{}; i < MaxFramesInFlight; ++i) {
        renderer->cameraBufferDescriptorSetLayouts.emplace_back(renderer->device, descriptorSetLayoutInfo, nullptr);
        descriptorSetLayouts.push_back(*renderer->cameraBufferDescriptorSetLayouts[i]);
    }

    vk::DescriptorSetAllocateInfo descriptorSetInfo {
        .descriptorPool = renderer->uniformBufferDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data()
    };

    renderer->cameraBufferDescriptorSets = std::move(vk::raii::DescriptorSets(renderer->device, descriptorSetInfo));
}
