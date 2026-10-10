#include "headers/base.h"
#include "headers/pipeline.h"
#include "headers/vertex.h"
#include "headers/buffer.h"

VkShaderModule readSpv(Renderer *renderer, const char *fileName) {
    FILE *f = fopen(fileName, "rb");
    if (!f) printf("shader file not opened properly");

    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    rewind(f);

    Arena *scratch = renderer->scratch;
    u64 scratchPos = scratch->pos;

    u32 *buffer = (u32 *)pushArena(scratch, size);

    fread(buffer, size, 1, f);
    fclose(f);

    VkShaderModuleCreateInfo shaderModuleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = buffer
    };

    VkShaderModule output;
    VK_CHECK(vkCreateShaderModule(renderer->device, &shaderModuleInfo, NULL, &output));

    arenaPopTo(scratch, scratchPos);

    return output;
}

void createGraphicsPipeline(Renderer *renderer) {
    //Shaders
    auto shaderModule = readSpv(renderer, "shaders/shader.spv");

    VkPipelineShaderStageCreateInfo vertexShaderInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = shaderModule,
        .pName = "vertMain"
    };

    VkPipelineShaderStageCreateInfo fragmentShaderInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = shaderModule,
        .pName = "fragMain"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderInfo, fragmentShaderInfo};

    //Dynamic State
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
        .pDynamicStates = dynamicStates
    };

    //Vertex Input
    VkVertexInputBindingDescription bindingDescription = getVertexBindingDescription();
    VkVertexInputAttributeDescription attributeDescription[VertexAttributeCount];
    getVertexAttributeDescriptions(attributeDescription);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = VertexAttributeCount,
        .pVertexAttributeDescriptions = attributeDescription
    };

    //Input Assembler
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    //Viewport
    VkPipelineViewportStateCreateInfo viewportInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    //Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f
    };

    //Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE
    };

    //Depth & Stencil
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
    };

    //Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    //Pipeline Layout
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(glm::mat4)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = renderer->cameraBufferDescriptorSetLayouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    VK_CHECK(vkCreatePipelineLayout(renderer->device, &pipelineLayoutInfo, NULL, &renderer->graphicsPipelineLayout));

    //Pipeline Creation & Dynamic Rendering
    // Chain: pipelineCreateInfo -> renderingInfo
    VkPipelineRenderingCreateInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &renderer->swapchainSurfaceFormat.format
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingInfo,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterizationInfo,
        .pMultisampleState = &multisampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = renderer->graphicsPipelineLayout,
        .renderPass = VK_NULL_HANDLE
    };

    VK_CHECK(vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
                                       NULL, &renderer->graphicsPipeline));

    //cleanup
    vkDestroyShaderModule(renderer->device, shaderModule, NULL);
}
