#include "config.h"

using namespace api;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // ------------------------------------------- //

    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    std::vector<VkPhysicalDevice> physicalDevices;

    uint32_t graphicsFamily, presentFamily;

    window = glfwCreateWindow(600, 400, "Application", nullptr, nullptr);
    instance = createInstance();
    
    glfwCreateWindowSurface(instance, window, nullptr, &surface);

    VkPhysicalDevice physicalDevice = selectPhysicalDevice(enumeratePhysicalDevices(instance), surface);
    VkDevice logicalDevice = createLogicalDevice(physicalDevice, surface);

    api::queryQueueFamilyIndices(physicalDevice, surface, &graphicsFamily, nullptr, nullptr, &presentFamily);

    VkQueue graphicsQueue, presentQueue;
    vkGetDeviceQueue(logicalDevice, graphicsFamily, 0, &graphicsQueue); 
    vkGetDeviceQueue(logicalDevice, presentFamily, 0, &presentQueue); 

    VkSurfaceCapabilitiesKHR surfaceCapabilities = querySurfaceCapabilities(physicalDevice, surface);
    std::vector<VkSurfaceFormatKHR> surfaceFormats = enumerateSurfaceFormats(physicalDevice, surface);
    std::vector<VkPresentModeKHR> presentModes = enumerateSurfacePresentModes(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(surfaceFormats);
    VkPresentModeKHR presentMode = selectSurfacePresentMode(presentModes);
    VkExtent2D extents = selectSurfaceExtents(window, surfaceCapabilities);

    VkSwapchainKHR swapchain = createSwapchain(physicalDevice, logicalDevice, surface, surfaceFormat, presentMode, extents);

    std::vector<VkImageView> imageViews = createImageViews(logicalDevice, swapchain, surfaceFormat);

    // ------------------------------------------- //

    std::vector<uint32_t> vertShaderCode = loadBytecodeSPV("./build/shader.vert.spv");
    std::vector<uint32_t> fragShaderCode = loadBytecodeSPV("./build/shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(logicalDevice, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(logicalDevice, fragShaderCode);

    VkRenderPass renderPass = createRenderPass(logicalDevice, surfaceFormat);
    std::vector<VkFramebuffer> framebuffers = createFramebuffers(logicalDevice, extents, imageViews, renderPass);
    
    VkPipeline pipeline = createPipeline(logicalDevice, surfaceFormat, extents, renderPass, vertShaderModule, fragShaderModule);

    VkCommandPool commandPool = createCommandPool(logicalDevice, graphicsFamily);
    VkCommandBuffer commandBuffer = createCommandBuffer(logicalDevice, commandPool);

    // ------------------------------------------- //

    VkSemaphore imageAvailableSemaphore = createSemaphore(logicalDevice);
    VkSemaphore renderFinishedSemaphore = createSemaphore(logicalDevice);
    VkFence inFlightFence = createFence(logicalDevice);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extents.width);
    viewport.height = static_cast<float>(extents.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extents;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    uint32_t imageIndex;

    // ------------------------------------------- //

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(logicalDevice, 1, &inFlightFence);
        vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        vkResetCommandBuffer(commandBuffer, 0);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extents;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
        
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    for (const auto& imageView : imageViews) {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}