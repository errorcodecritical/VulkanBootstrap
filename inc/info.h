#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace api {
    std::vector<VkExtensionProperties> enumerateInstanceExtensions();
    std::vector<VkQueueFamilyProperties> enumerateQueueFamilies(VkPhysicalDevice physicalDevice);
    std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance);
    std::vector<VkExtensionProperties> enumerateDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
    std::vector<VkSurfaceFormatKHR> enumerateSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    std::vector<VkPresentModeKHR> enumerateSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    std::vector<VkImage> enumerateSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain);

    bool queryFullDeviceExtensionSupport(
        VkPhysicalDevice physicalDevice, 
        std::vector<std::string> requiredExtensions
    );

    VkSurfaceCapabilitiesKHR querySurfaceCapabilities(
        VkPhysicalDevice physicalDevice, 
        VkSurfaceKHR surface
    );   
    
    void queryQueueFamilyIndices(
        VkPhysicalDevice physicalDevice, 
        VkSurfaceKHR surface, 
        uint32_t* graphicsFamily, 
        uint32_t* computeFamily, 
        uint32_t* transferFamily, 
        uint32_t* presentFamily
    );

    std::vector<uint32_t> loadBytecodeSPV(
        const std::string& filename
    );
}

std::vector<VkExtensionProperties> api::enumerateInstanceExtensions() {
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> result(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, result.data());

    return result;
}

std::vector<VkQueueFamilyProperties> api::enumerateQueueFamilies(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> result(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, result.data());

    return result;
}

std::vector<VkPhysicalDevice> api::enumeratePhysicalDevices(VkInstance instance) {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    std::vector<VkPhysicalDevice> result(count);
    vkEnumeratePhysicalDevices(instance, &count, result.data());

    return result;
}

std::vector<VkSurfaceFormatKHR> api::enumerateSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    
    std::vector<VkSurfaceFormatKHR> result(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, result.data());

    return result;
}

std::vector<VkPresentModeKHR> api::enumerateSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    
    std::vector<VkPresentModeKHR> result(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, result.data());

    return result;
}

std::vector<VkExtensionProperties> api::enumerateDeviceExtensionSupport(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> result(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, result.data());

    return result;
}

std::vector<VkImage> api::enumerateSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain) {
    uint32_t count;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &count, nullptr);
    
    std::vector<VkImage> result(count);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &count, result.data());

    return result;
}

bool api::queryFullDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<std::string> requiredExtensions) {
    std::vector<VkExtensionProperties> supportedExtensions = api::enumerateDeviceExtensionSupport(physicalDevice);

    uint32_t count = requiredExtensions.size();

    if (count == 0) { 
        return true;
    }

    for (const auto& required : requiredExtensions) {
        for (const auto& supported : supportedExtensions) {
            if (required == supported.extensionName) {
                count--;
                break;
            }
        }
    }

    return (count == 0);
}

VkSurfaceCapabilitiesKHR api::querySurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR result;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &result);

    return result;
}

void api::queryQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* graphicsFamily, uint32_t* computeFamily, uint32_t* transferFamily, uint32_t* presentFamily) {
    std::vector<VkQueueFamilyProperties> queueFamilies = api::enumerateQueueFamilies(physicalDevice);

    uint32_t index = 0;
    for (const auto& queueFamily : queueFamilies) {        
        VkBool32 graphicsSupported = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);
        VkBool32 computeSupported = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
        VkBool32 transferSupported = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);
        VkBool32 presentSupported = VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentSupported);
        
        if (graphicsSupported && graphicsFamily) {
            *graphicsFamily = index;
        }

        if (computeSupported && computeFamily) {
            *computeFamily = index;
        }

        if (transferSupported && transferFamily) {
            *transferFamily = index;
        }

        if (presentSupported && presentFamily) {
            *presentFamily = index;
        }

        index++;
    }
}

std::vector<uint32_t> api::loadBytecodeSPV(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t length = (size_t) file.tellg();
    std::vector<uint32_t> buffer(length / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), length);
    file.close();

    return buffer;
}