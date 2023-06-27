module;
#include "platform/vulkan.hpp"
export module debug;

export struct DebugCallback {
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugUtilsCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverityEnum = vk::DebugUtilsMessageSeverityFlagBitsEXT(messageSeverity);
        const char* message = pCallbackData->pMessage;
        switch (messageSeverityEnum) {
            using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
            case eVerbose: printf("VERBOSE: %s\n", message); break;
            case eInfo: printf("INFO: %s\n", message); break;
            case eWarning: printf("WARNING: %s\n", message); break;
            case eError: printf("ERROR: %s\n", message); break;
        }
        return VK_FALSE;
    }
};