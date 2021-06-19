#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

int main(int argc, char const *argv[])
{
    ::glfwInit();
    ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::cout << "Count = " << count << std::endl;

    uint32_t exts = 0;
    char const **extensions = ::glfwGetRequiredInstanceExtensions(&exts);
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = exts;
    createInfo.ppEnabledExtensionNames = extensions;
    
    ::GLFWwindow *window = ::glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    ::VkInstance instance;
    ::vkCreateInstance(&createInfo, nullptr, &instance);

    VkSurfaceKHR surface;
    ::glfwCreateWindowSurface(instance, window, nullptr, &surface);

    while (!::glfwWindowShouldClose(window))
    {
        ::glfwWaitEvents();
        ::glfwPollEvents();
    }

    ::vkDestroySurfaceKHR(instance, surface, nullptr);
    ::vkDestroyInstance(instance, nullptr);

    ::glfwDestroyWindow(window);
    ::glfwTerminate();
    return 0;
}
