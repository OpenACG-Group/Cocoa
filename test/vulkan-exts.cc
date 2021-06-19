#include <iostream>
#include <vulkan/vulkan.h>

int main(int argc, char const **argv)
{
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::cout << count << std::endl;
	return 0;
}
