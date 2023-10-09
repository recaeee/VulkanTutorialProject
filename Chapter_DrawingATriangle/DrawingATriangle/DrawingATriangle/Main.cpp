//GLFW将会包含其自己的一些定义，并且自动加载Vulkan头文件
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//stdexcept和iostream头文件用于报告和输出errors
#include <iostream>
#include <stdexcept>
//cstdlib头文件提供了EXIT_SUCCESS和EXIT_FAILURE宏
#include <cstdlib>

#include <vector>
#include <cstring>

//使用常量而不是硬编码的width和height，因为我们会多次引用这些值
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

//要使用的validation layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//debug模式下开启Validation layers，release则不启用
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;

	//始化GLFW并且创建一个window
	void initWindow()
	{
		//初始化GLFW库
		glfwInit();

		//由于GLFW最开始是被用于创建OpenGL context的，所以我们需要告诉它不要创建一个OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//disable resize功能
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//创建真正的window
		//前三个参数确定了window的宽、高和标题。第四个参数允许我们选择指定打开window的监视器，最后一个参数仅与OpenGL相关。
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	//**initVulkan**函数来用于实例化Vulkan objects私有成员
	void initVulkan()
	{
		createInstance();
	}

	//mainloop来开始渲染每一帧
	void mainLoop()
	{
		//为了确保应用直到发生错误或者关闭window才结束运行，我们需要增加一个事件循环在mainLoop中，如下
		while (!glfwWindowShouldClose(window))
		{
			//处理窗口事件并触发事件回调函数、如鼠标、键盘事件、窗口尺寸的调整、窗口关闭等
			glfwPollEvents();
		}
	}

	//一旦window被关闭，我们将在**cleanup**函数中确保释放我们用到的所有资源
	void cleanup()
	{
		//VkInstance只应该在程序退出前一刻销毁，所有其他的Vulkan资源都应该在instance销毁前释放
		vkDestroyInstance(instance, nullptr);

		//销毁window
		glfwDestroyWindow(window);

		//结束GLFW本身
		glfwTerminate();
	}
	
	void createInstance()
	{
		//检查Validation layers支持
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
		//Instance是我们的应用与Vulkan libaray之间的联系，创建它需要向驱动指定和我们的应用相关的一些详细信息
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		//我们想要使用哪些全局extensions和validation layers
		auto extensions = getRequiredExtension();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		//检查Extension支持
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
		std::cout << "available extensions\n";

		for (const auto& extension : availableExtensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}
		//不需要检查Validation layers的extension是否支持
		checkRequiredExtensionsSupported(glfwExtensions, glfwExtensionCount, availableExtensions);

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}

	void checkRequiredExtensionsSupported(const char** requiredExtensions, uint32_t requiredExtensionCount, std::vector<VkExtensionProperties> availableExtensions)
	{
		for (int i = 0; i < requiredExtensionCount; i++)
		{
			bool found = false;
			for (const auto& extension : availableExtensions)
			{
				if (strcmp(requiredExtensions[i], extension.extensionName))
				{
					found = true;
				}
			}
			if (!found)
			{
				throw std::runtime_error("Required extension not supported.");
			}
		}
		std::cout << "All required extensions are supported." << std::endl;
	}

	//检查使用的validation layers是否都支持
	bool checkValidationLayerSupport()
	{
		//和extension几乎一样的操作
		//先得到所有支持的layers
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//检查
		for (const char* layerName : validationLayers) 
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	//得到需要使用的extensions
	std::vector<const char*> getRequiredExtension()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		//选择性开启Validation layers
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
};

int main()
{
	HelloTriangleApplication app;

	try 
	{
		app.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}