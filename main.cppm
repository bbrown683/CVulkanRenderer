import window;
import instance;
import device;
import queue;
import swapchain;
import cmd;
import buffer;

auto main() -> int {
    auto window = CSDLWindow();
    auto sdlWindow = window.GetSDLWindow();

    auto instance = CVulkanInstance(sdlWindow);
    auto vkInstance = instance.GetVkInstance();
    auto vkPhysicalDevices = instance.GetVkPhysicalDevices();
    auto vkPhysicalDevice = vkPhysicalDevices.front();

    auto device = CVulkanDevice(vkPhysicalDevice);
    auto vkDevice = device.GetVkDevice();
    auto graphicsQueueIndex = device.GetGraphicsQueueIndex();

    auto queue = CVulkanQueue(vkDevice, graphicsQueueIndex);
    auto vkQueue = queue.GetVkQueue();

    auto swapchain = CVulkanSwapchain(vkInstance, vkPhysicalDevice, vkDevice, vkQueue, sdlWindow, 2);

    auto bufferAllocator = CVulkanBufferAllocator(vkInstance, vkPhysicalDevice, vkDevice);

    auto commandPool = CVulkanCommandPool(vkDevice, graphicsQueueIndex);
    auto vkCommandPool = commandPool.GetVkCommandPool();

    auto commandBuffer = CVulkanCommandBuffer(vkDevice, vkCommandPool);

    window.PollEvents();
    return 0;
}