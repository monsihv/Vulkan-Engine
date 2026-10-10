#include "headers/base.h"
#include "headers/buffer.h"

Buffer createBuffer(Renderer *renderer, const void *data, VkDeviceSize size,
                    VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocationFlags,
                    VmaPool pool) {
    Buffer output = {};

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocationInfo = {
        .flags = allocationFlags,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .pool = pool
    };

    VmaAllocationInfo allocationOutputInfo;
    if (vmaCreateBuffer(renderer->allocator, &bufferInfo, &allocationInfo,
                &output.buffer, &output.memory, &allocationOutputInfo) != VK_SUCCESS) {
        fprintf(stderr, "buffer not made via vma\n");
        exit(1);
    }

    if (allocationFlags & vmaKeepMapped) output.map = allocationOutputInfo.pMappedData;

    if (data) memcpy(output.map, data, bufferInfo.size);

    return output;
}

void destroyBuffer(Renderer *renderer, Buffer& buffer) {
    vmaDestroyBuffer(renderer->allocator, buffer.buffer, buffer.memory);
    buffer.map = NULL;
}

void copyBuffer(Renderer *renderer, VkBuffer src, VkBuffer dst,
                VkDeviceSize size, VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkBufferCopy bufferCopy = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &bufferCopy);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };
    VK_CHECK(vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    vkDeviceWaitIdle(renderer->device);
}

void createDescriptorPools(Renderer *renderer) {
    VkDescriptorPoolSize descriptorPoolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = MaxFramesInFlight
    };

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(renderer->GPU, &deviceProperties);
    VkDescriptorPoolCreateInfo descriptorPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = deviceProperties.limits.maxDescriptorSetUniformBuffers,
        .poolSizeCount = 1,
        .pPoolSizes = &descriptorPoolSize
    };

    VK_CHECK(vkCreateDescriptorPool(renderer->device, &descriptorPoolInfo, NULL, &renderer->uniformBufferDescriptorPool));
}

void createUniformBufferDescriptorSets(Renderer *renderer) {
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &descriptorSetLayoutBinding
    };

    auto& descriptorSetLayouts = renderer->cameraBufferDescriptorSetLayouts;
    for (u32 i = 0; i < MaxFramesInFlight; ++i) {
        VK_CHECK(vkCreateDescriptorSetLayout(renderer->device, &descriptorSetLayoutInfo, NULL, &descriptorSetLayouts[i]));
    }

    VkDescriptorSetAllocateInfo descriptorSetInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = renderer->uniformBufferDescriptorPool,
        .descriptorSetCount = MaxFramesInFlight,
        .pSetLayouts = descriptorSetLayouts
    };

    VK_CHECK(vkAllocateDescriptorSets(renderer->device, &descriptorSetInfo, renderer->cameraBufferDescriptorSets));
}
