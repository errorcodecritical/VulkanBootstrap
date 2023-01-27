#pragma once

#include "info.h"

namespace api {
    VkPhysicalDevice selectPhysicalDevice(
        std::vector<VkPhysicalDevice> physicalDevices,
        VkSurfaceKHR surface
    );

    VkPresentModeKHR selectSurfacePresentMode(
        std::vector<VkPresentModeKHR> presentModes
    );

    VkSurfaceFormatKHR selectSurfaceFormat(
        std::vector<VkSurfaceFormatKHR> surfaceFormats
    );

    VkExtent2D selectSurfaceExtents(
        GLFWwindow* window,
        VkSurfaceCapabilitiesKHR& surfaceCapabilities
    );

    bool isDeviceAdequate(
        VkPhysicalDevice physicalDevice, 
        VkSurfaceKHR surface
    );

    VkInstance createInstance();

    VkDevice createLogicalDevice(
        VkPhysicalDevice physicalDevice, 
        VkSurfaceKHR surface
    );
    
    VkSwapchainKHR createSwapchain(
        VkPhysicalDevice physicalDevice, 
        VkDevice logicalDevice, 
        VkSurfaceKHR surface, 
        VkSurfaceFormatKHR surfaceFormat,
        VkPresentModeKHR presentMode,
        VkExtent2D extents
    );

    std::vector<VkImageView> createImageViews(
        VkDevice logicalDevice, 
        VkSwapchainKHR swapchain, 
        VkSurfaceFormatKHR surfaceFormat
    );

    VkShaderModule createShaderModule(
        VkDevice logicalDevice, 
        std::vector<uint32_t>& shaderBytecode
    );

    VkRenderPass createRenderPass(
        VkDevice logicalDevice, 
        VkSurfaceFormatKHR surfaceFormat
    );

    VkPipelineLayout createPipelineLayout(
        VkDevice logicalDevice
    );

    std::vector<VkFramebuffer> createFramebuffers(
        VkDevice logicalDevice, 
        VkExtent2D extents, 
        std::vector<VkImageView> imageViews, 
        VkRenderPass renderPass
    );

    VkCommandPool createCommandPool(
        VkDevice logicalDevice, 
        uint32_t graphicsFamily
    );

    VkCommandBuffer createCommandBuffer(
        VkDevice logicalDevice, 
        VkCommandPool commandPool
    );

    VkPipeline createPipeline(
        VkDevice logicalDevice, 
        VkSurfaceFormatKHR surfaceFormat, 
        VkExtent2D extents, 
        VkRenderPass renderPass, 
        VkShaderModule vertShaderModule, 
        VkShaderModule fragShaderModule
    );   

    VkSemaphore createSemaphore(
        VkDevice logicalDevice
    );

    VkFence createFence(
        VkDevice logicalDevice
    );

}

VkPhysicalDevice api::selectPhysicalDevice(std::vector<VkPhysicalDevice> physicalDevices, VkSurfaceKHR surface) {
    VkPhysicalDevice result = physicalDevices[0];

    for (const auto& physicalDevice : physicalDevices) {
        if (api::isDeviceAdequate(physicalDevice, surface)) {
            result = physicalDevice;
        }
    }

    return result;
}

VkPresentModeKHR api::selectSurfacePresentMode(std::vector<VkPresentModeKHR> presentModes) {
    VkPresentModeKHR result = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode : presentModes) {
        bool isPresentModeSufficient = (presentMode == VK_PRESENT_MODE_MAILBOX_KHR);

        if (isPresentModeSufficient) {
            result = presentMode;
            break;
        }
    }

    return result;
}

VkSurfaceFormatKHR api::selectSurfaceFormat(std::vector<VkSurfaceFormatKHR> surfaceFormats) {
    VkSurfaceFormatKHR result = surfaceFormats[0];

    for (const auto& surfaceFormat : surfaceFormats) {
        bool isFormatSufficient = (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB);
        bool isColorSpaceSufficient = (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

        if (isFormatSufficient && isColorSpaceSufficient) {
            result = surfaceFormat;
            break;
        }
    }

    return result;
}

VkExtent2D api::selectSurfaceExtents(GLFWwindow* window, VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    VkExtent2D result = surfaceCapabilities.currentExtent;

    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        result.width = std::clamp(
            static_cast<uint32_t>(width),
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width
        );

        result.height = std::clamp(
            static_cast<uint32_t>(height),
            surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.maxImageExtent.height
        );
    }

    return result;
}

bool api::isDeviceAdequate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    bool supported = api::queryFullDeviceExtensionSupport(physicalDevice, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
    
    std::vector<VkSurfaceFormatKHR> formats = api::enumerateSurfaceFormats(physicalDevice, surface);
    std::vector<VkPresentModeKHR> modes = api::enumerateSurfacePresentModes(physicalDevice, surface);

    bool result = (supported && !formats.empty() && !modes.empty());

    return result;
}

VkInstance api::createInstance() {
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&instanceCreateInfo.enabledExtensionCount);
    instanceCreateInfo.enabledLayerCount = 0;

    VkInstance result;

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    return result;
}

VkDevice api::createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t graphicsFamily, presentFamily;
    api::queryQueueFamilyIndices(physicalDevice, surface, &graphicsFamily, nullptr, nullptr, &presentFamily);

    float queuePriority = 1.0f;
    const char* enabledExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceQueueCreateInfo queueCreateInfo[2] = {};
    queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[0].queueFamilyIndex = graphicsFamily;
    queueCreateInfo[0].queueCount = 1;
    queueCreateInfo[0].pQueuePriorities = &queuePriority;

    queueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[1].queueFamilyIndex = presentFamily;
    queueCreateInfo[1].queueCount = 1;
    queueCreateInfo[1].pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 2;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions;
    deviceCreateInfo.enabledExtensionCount = 1;

    VkDevice result;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    return result;
}

VkSwapchainKHR api::createSwapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extents) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities = api::querySurfaceCapabilities(physicalDevice, surface);

    uint32_t queueFamilyIndices[2] = {};
    api::queryQueueFamilyIndices(physicalDevice, surface, queueFamilyIndices, nullptr, nullptr, queueFamilyIndices+1);

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extents;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
        swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    VkSwapchainKHR result = 0;

    if (vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swapchain!");
    }

    return result;
}

std::vector<VkImageView> api::createImageViews(VkDevice logicalDevice, VkSwapchainKHR swapchain, VkSurfaceFormatKHR surfaceFormat) {
    std::vector<VkImage> swapchainImages = api::enumerateSwapchainImages(logicalDevice, swapchain);
    std::vector<VkImageView> result(swapchainImages.size());

    uint32_t index = 0;
    for (const auto& swapchainImage : swapchainImages) {
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = swapchainImage;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = surfaceFormat.format;
        viewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY};
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logicalDevice, &viewCreateInfo, nullptr, &result[index++]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }
    }

    return result;    
}

VkShaderModule api::createShaderModule(VkDevice logicalDevice, std::vector<uint32_t>& shaderBytecode) {
    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = shaderBytecode.size() * sizeof(uint32_t);
    shaderCreateInfo.pCode = shaderBytecode.data();

    VkShaderModule result;

    if (vkCreateShaderModule(logicalDevice, &shaderCreateInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return result;
}

VkRenderPass api::createRenderPass(VkDevice logicalDevice, VkSurfaceFormatKHR surfaceFormat) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass result;

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return result;
}

VkPipelineLayout api::createPipelineLayout(VkDevice logicalDevice) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VkPipelineLayout result;

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    return result;
}

std::vector<VkFramebuffer> api::createFramebuffers(VkDevice logicalDevice, VkExtent2D extents, std::vector<VkImageView> imageViews, VkRenderPass renderPass) {
    std::vector<VkFramebuffer> result(imageViews.size());

    uint32_t index = 0;
    for (const auto& imageView : imageViews) {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imageView;
        framebufferInfo.width = extents.width;
        framebufferInfo.height = extents.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &result[index++]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    return result;
}

VkCommandPool api::createCommandPool(VkDevice logicalDevice, uint32_t graphicsFamily) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamily;

    VkCommandPool result;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    return result;
}

VkCommandBuffer api::createCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer result;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }    

    return result;
}

VkPipeline api::createPipeline(VkDevice logicalDevice, VkSurfaceFormatKHR surfaceFormat, VkExtent2D extents, VkRenderPass renderPass, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule) {   
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineLayout pipelineLayout = createPipelineLayout(logicalDevice);

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extents.width;
    viewport.height = (float) extents.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extents;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional  

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    VkPipeline result;
    
    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return result;
}

VkSemaphore api::createSemaphore(VkDevice logicalDevice) {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore result;

    if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    return result;
}

VkFence api::createFence(VkDevice logicalDevice) {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence result;

    if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

    return result;
}