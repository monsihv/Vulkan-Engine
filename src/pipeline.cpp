#include "headers/base.h"
#include "headers/pipeline.h"
#include "headers/vertex.h"

#include <stdio.h>
#include <vector>

vk::raii::ShaderModule readSpv(Renderer *renderer, const char *fileName) {
    FILE *f = fopen(fileName, "rb");
    if (!f) printf("shader file not opened properly");

    fseek(f, 0, SEEK_END);
    uint32_t size = ftell(f);
    rewind(f);

    uint32_t *buffer = static_cast<uint32_t*>(::operator new(size));

    fread(buffer, size, 1, f);
    fclose(f);

    vk::ShaderModuleCreateInfo shaderModuleInfo {
        .codeSize = size,
        .pCode = buffer
    };

    auto output = vk::raii::ShaderModule(renderer->device, shaderModuleInfo);

    ::operator delete(buffer);

    return output;
}

void createGraphicsPipeline(Renderer *renderer) {
    //Shaders
    auto shaderModule = readSpv(renderer, "shaders/shader.spv");

    vk::PipelineShaderStageCreateInfo vertexShaderInfo {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName = "vertMain"
    };

    vk::PipelineShaderStageCreateInfo fragmentShaderInfo {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName = "fragMain"
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderInfo, fragmentShaderInfo};

    //Dynamic State
    vk::DynamicState dynamicStates[] = {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo {
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(vk::DynamicState),
        .pDynamicStates = dynamicStates
    };

    //Vertex Input
    auto bindingDescription {Vertex::getBindingDescription()};
    auto attributeDescription {Vertex::getAttributeDescription()};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()),
        .pVertexAttributeDescriptions = attributeDescription.data()
    };

    //Input Assembler
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo {
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    //Viewport
    vk::PipelineViewportStateCreateInfo viewportInfo {
        .viewportCount = 1,
        .scissorCount = 1
    };

    //Rasterization
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f
    };

    //Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampleInfo {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    //Depth & Stencil
    //vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;

    //Color Blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    //Pipeline Layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
        .setLayoutCount = 0, 
        .pushConstantRangeCount = 0
    };

    renderer->graphicsPipelineLayout = vk::raii::PipelineLayout(renderer->device, pipelineLayoutInfo);

    //Pipeline Creation & Dynamic Rendering
    vk::StructureChain<vk::GraphicsPipelineCreateInfo, 
                       vk::PipelineRenderingCreateInfo> 
                           pipelineCreateInfo {
                           {
                               .stageCount = 2,
                               .pStages = shaderStages,
                               .pVertexInputState = &vertexInputInfo,
                               .pInputAssemblyState = &inputAssemblyInfo,
                               .pViewportState = &viewportInfo,
                               .pRasterizationState = &rasterizationInfo,
                               .pMultisampleState = &multisampleInfo,
                               .pColorBlendState = &colorBlendInfo,
                               .pDynamicState = &dynamicStateInfo,
                               .layout = renderer->graphicsPipelineLayout,
                               .renderPass = nullptr}, 
                           {
                               .colorAttachmentCount = 1,
                               .pColorAttachmentFormats = &renderer->swapchainSurfaceFormat.format}
                           };

    renderer->graphicsPipeline = vk::raii::Pipeline(renderer->device, nullptr,
                                                    pipelineCreateInfo.get<vk::GraphicsPipelineCreateInfo>());
}











