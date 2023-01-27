/*
Copyright (c) 2023 William M.H.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
 
#include <vulkan/vulkan.h>

// Collection of utilities to retrieve Vulkan resources.
namespace platform {

    // Container for queue family indices. Return type of function [queryQueueFamilyIndices].
    typedef struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t computeFamily;
        uint32_t transferFamily;
        uint32_t presentFamily;
    } QueueFamilyIndices;

    /**
     * @brief Lists all available Vulkan and implicitly enabled instance extensions.
     * @return std::vector<VkExtensionProperties> 
     */
    std::vector<VkExtensionProperties> enumerateInstanceExtensions();

    /**
     * @brief Lists all available physical devices on the current platform.
     * @param instance 
     * @return std::vector<VkPhysicalDevice> 
     */
    std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance);

    /**
     * @brief Lists all supported extensions for a given physical device.
     * @param physicalDevice 
     * @return std::vector<VkExtensionProperties> 
     */
    std::vector<VkExtensionProperties> enumerateDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

    /**
     * @brief Lists all available queue families for a given physical device.
     * @param physicalDevice 
     * @return std::vector<VkQueueFamilyProperties> 
     */
    std::vector<VkQueueFamilyProperties> enumerateQueueFamilies(VkPhysicalDevice physicalDevice);

    /**
     * @brief Lists all supported color formats for a given surface.
     * @param physicalDevice 
     * @param surface 
     * @return std::vector<VkSurfaceFormatKHR> 
     */
    std::vector<VkSurfaceFormatKHR> enumerateSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    /**
     * @brief Lists all supported presentation modes for a given surface.
     * @param physicalDevice 
     * @param surface 
     * @return std::vector<VkPresentModeKHR> 
     */
    std::vector<VkPresentModeKHR> enumerateSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    /**
     * @brief Lists all presentable images associated with a given swapchain.
     * @param logicalDevice 
     * @param swapchain 
     * @return std::vector<VkImage> 
     */
    std::vector<VkImage> enumerateSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain);

    /**
     * @brief Check whether all required extensions are supported for a given physical device.
     * @param physicalDevice 
     * @param requiredExtensions 
     * @return true 
     * @return false 
     */
    bool queryFullDeviceExtensionSupport(
        VkPhysicalDevice physicalDevice, 
        std::vector<std::string> requiredExtensions
    );

    /** Query basic capabilities of a given surface.
     * @brief 
     * @param physicalDevice 
     * @param surface 
     * @return VkSurfaceCapabilitiesKHR 
     */
    VkSurfaceCapabilitiesKHR querySurfaceCapabilities(
        VkPhysicalDevice physicalDevice, 
        VkSurfaceKHR surface
    );   
    
    /**
     * @brief Query graphics, compute, transfer and presentation family indices for a given surface.
     * If a family is unsupported, the value of its index will be 0xFFFFFFFF.
     * @param physicalDevice 
     * @param surface 
     * @return QueueFamilyIndices 
     */
    QueueFamilyIndices queryQueueFamilyIndices(
        VkPhysicalDevice physicalDevice, 
        VkSurfaceKHR surface
    );

    /**
     * @brief Loads the 32-bit bytecode from a given .spv file.
     * @param filename 
     * @return std::vector<uint32_t> 
     */
    std::vector<uint32_t> loadBytecodeSPV(const std::string& filename);
}

// IMPLEMENTATION

std::vector<VkExtensionProperties> platform::enumerateInstanceExtensions() {
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> result(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, result.data());

    return result;
}

std::vector<VkQueueFamilyProperties> platform::enumerateQueueFamilies(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> result(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, result.data());

    return result;
}

std::vector<VkPhysicalDevice> platform::enumeratePhysicalDevices(VkInstance instance) {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    std::vector<VkPhysicalDevice> result(count);
    vkEnumeratePhysicalDevices(instance, &count, result.data());

    return result;
}

std::vector<VkSurfaceFormatKHR> platform::enumerateSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    
    std::vector<VkSurfaceFormatKHR> result(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, result.data());

    return result;
}

std::vector<VkPresentModeKHR> platform::enumerateSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    
    std::vector<VkPresentModeKHR> result(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, result.data());

    return result;
}

std::vector<VkExtensionProperties> platform::enumerateDeviceExtensionSupport(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> result(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, result.data());

    return result;
}

std::vector<VkImage> platform::enumerateSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain) {
    uint32_t count;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &count, nullptr);
    
    std::vector<VkImage> result(count);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &count, result.data());

    return result;
}

bool platform::queryFullDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<std::string> requiredExtensions) {
    std::vector<VkExtensionProperties> supportedExtensions = platform::enumerateDeviceExtensionSupport(physicalDevice);

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

VkSurfaceCapabilitiesKHR platform::querySurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR result;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &result);

    return result;
}

platform::QueueFamilyIndices platform::queryQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    std::vector<VkQueueFamilyProperties> queueFamilies = platform::enumerateQueueFamilies(physicalDevice);

    QueueFamilyIndices result = {0xFFFFFFFF};

    uint32_t index = 0;
    for (const auto& queueFamily : queueFamilies) {        
        VkBool32 graphicsSupported = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);
        VkBool32 computeSupported = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
        VkBool32 transferSupported = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);
        VkBool32 presentSupported = VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentSupported);
        
        if (graphicsSupported) {
            result.graphicsFamily = index;
        }

        if (computeSupported) {
            result.computeFamily = index;
        }

        if (transferSupported) {
            result.transferFamily = index;
        }

        if (presentSupported) {
            result.presentFamily = index;
        }

        index++;
    }

    return result;
}

std::vector<uint32_t> platform::loadBytecodeSPV(const std::string& filename) {
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