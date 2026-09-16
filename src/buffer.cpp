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

    output.buffer = renderer->device.createBuffer(bufferInfo);

    auto memoryRequirements = renderer->device.getBufferMemoryRequirements(output.buffer);

    vk::MemoryAllocateInfo memoryInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(renderer->GPU, memoryRequirements.memoryTypeBits, memoryFlags)
    };

    output.memory = renderer->device.allocateMemory(memoryInfo);
    renderer->device.bindBufferMemory(output.buffer, output.memory, 0);
    output.map = renderer->device.mapMemory(output.memory, 0, bufferInfo.size);
    if (data && (memoryFlags & vk::MemoryPropertyFlagBits::eHostCoherent)) memcpy(output.map, data, bufferInfo.size);

    return output;
}

void destroyBuffer(Renderer *renderer, Buffer& buffer) {
    renderer->device.free(buffer.memory);
    renderer->device.destroy(buffer.buffer);
    buffer.map = nullptr;
}

void copyBuffer(Renderer *renderer, vk::Buffer& src, vk::Buffer& dst,
                vk::DeviceSize size, vk::CommandBuffer& commandBuffer) {
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
        .pCommandBuffers = &commandBuffer
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

    renderer->uniformBufferDescriptorPool = renderer->device.createDescriptorPool(descriptorPoolInfo);
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

    auto& descriptorSetLayouts = renderer->cameraBufferDescriptorSetLayouts;
    descriptorSetLayouts.reserve(MaxFramesInFlight);
    for (uint32_t i{}; i < MaxFramesInFlight; ++i) {
        auto descriptorLayout = renderer->device.createDescriptorSetLayout(descriptorSetLayoutInfo, nullptr);
        descriptorSetLayouts.push_back(descriptorLayout);
    }

    vk::DescriptorSetAllocateInfo descriptorSetInfo {
        .descriptorPool = renderer->uniformBufferDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data()
    };

    renderer->cameraBufferDescriptorSets = renderer->device.allocateDescriptorSets(descriptorSetInfo);
}
