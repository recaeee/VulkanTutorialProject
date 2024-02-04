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

#include <optional>
#include <set>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include <fstream>
//数学库，包含vector和matrix等，用于声明顶点数据
#include <glm/glm.hpp>
#include <array>

//使用常量而不是硬编码的width和height，因为我们会多次引用这些值
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
//多少帧应该同时工作
const int MAX_FRAMES_IN_FLIGHTS = 2;

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

//由于创建DebugUtilMessenger的函数是个扩展函数（不会自动加载到内存），所以我们要自己查找它的地址，并使用proxy function来执行。
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMeseengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

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
	//管理debug callback的handle
	VkDebugUtilsMessengerEXT debugMessenger;
	//Vulkan与实际平台Window system的交互，抽象的Surface
	VkSurfaceKHR surface;
	//启用的显卡
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//Logical device，用来与physical device交互
	VkDevice device;
	//存储Graphics queue的Handle
	VkQueue graphicsQueue;
	//存储Present queue的Handle
	VkQueue presentQueue;
	//存储Swap chain
	VkSwapchainKHR swapChain;
	//存储Swap chain中的VkImage们
	std::vector<VkImage> swapChainImages;
	//存储swap chain的format和extent，未来会用到
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	//存储swap chain images的image views
	std::vector<VkImageView> swapChainImageViews;
	//存储render pass
	VkRenderPass renderPass;
	//存储pipeline layout，用来指定创建管线时的uniform值
	VkPipelineLayout pipelineLayout;
	//存储pipeline
	VkPipeline graphicsPipeline;
	//存储framebuffers
	std::vector<VkFramebuffer> swapChainFramebuffers;
	//存储command pool
	VkCommandPool commandPool;
	//存储command buffer
	std::vector<VkCommandBuffer> commandBuffers;
	//存储从swapchain中取得image的semaphore
	std::vector<VkSemaphore> imageAvailableSemaphores;
	//存储image渲染完毕的semaphore
	std::vector<VkSemaphore> renderFinishedSemaphores;
	//存储保证一次只渲染一帧的fence
	std::vector<VkFence> inFlightFences;
	//当前帧索引
	uint32_t currentFrame = 0;
	//标识一个window resize发生的flag
	bool framebufferResized = false;

	//所有要启用的device extensions
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//始化GLFW并且创建一个window
	void initWindow()
	{
		//初始化GLFW库
		glfwInit();

		//由于GLFW最开始是被用于创建OpenGL context的，所以我们需要告诉它不要创建一个OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//disable resize功能
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//创建真正的window
		//前三个参数确定了window的宽、高和标题。第四个参数允许我们选择指定打开window的监视器，最后一个参数仅与OpenGL相关。
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		//将helloTriangleApplication的this指针存储到window中
		glfwSetWindowUserPointer(window, this);
		//注册resize回调
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	//**initVulkan**函数来用于实例化Vulkan objects私有成员
	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	//mainloop来开始渲染每一帧
	void mainLoop()
	{
		//为了确保应用直到发生错误或者关闭window才结束运行，我们需要增加一个事件循环在mainLoop中，如下
		while (!glfwWindowShouldClose(window))
		{
			//处理窗口事件并触发事件回调函数、如鼠标、键盘事件、窗口尺寸的调整、窗口关闭等
			glfwPollEvents();
			//绘制一帧
			drawFrame();
		}

		//等待logical device完成所有操作，再退出mainLoop
		vkDeviceWaitIdle(device);
	}

	//一旦window被关闭，我们将在**cleanup**函数中确保释放我们用到的所有资源
	void cleanup()
	{
		cleanupSwapChain();

		//销毁pipeline
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		//销毁pipeline layout
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		//销毁render pass
		vkDestroyRenderPass(device, renderPass, nullptr);

		//当程序结束，并且所有commands都完成了，并且没有更多的同步被需要时，semaphores和fence需要被销毁
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
		{
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		//销毁command pool
		vkDestroyCommandPool(device, commandPool, nullptr);
		
		//销毁VkDevice
		vkDestroyDevice(device, nullptr);
		//销毁DebugUtilsMessenger
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMeseengerEXT(instance, debugMessenger, nullptr);
		}
		//销毁Window surface
		vkDestroySurfaceKHR(instance, surface, nullptr);
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
		//额外创建一个DebugUtilsMessenger，让vkInstance的pNext拓展指向它，这样可以debug vkCreateInstance和vkDestroyInstance的信息。
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			polulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
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

	//自己管理Validation消息回调
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		}
		

		return VK_FALSE;
	}

	//初始化Debug信息的messenger(VkDebugUtilsMessengerEXT），这个messenger是vkInstance生命周期内使用的
	void setupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		//创建messenger同样需要CreateInfo的流程
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		polulateDebugMessengerCreateInfo(createInfo);

		//通过proxy function创建DebugUtilsMessenger
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//DebugUtilsMessenger的CreateInfo填充单独抽出来一个函数
	void polulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//回调
		createInfo.pfnUserCallback = debugCallback;
	}

#pragma region Physical devices and queue families
	//启用一张合适的显卡
	void pickPhysicalDevice()
	{
		//列出所有能用的显卡
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		//如果没有一张支持Vulkan的显卡可用，则抛出异常
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		//找到合适的显卡
		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}
		}
		
		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	//判断该显卡是否支持我们想要的功能
	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		//获取显卡的基本属性，比如名字、类型、支持的vulkan版本
		//VkPhysicalDeviceProperties deviceProperties;
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//获取显卡支持的特性，例如纹理压缩、64比特浮点数、多视口渲染
		//VkPhysicalDeviceFeatures deviceFeatures;
		//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		//在之后的章节我们还会查询显存、Queue families

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			//在该教程中，我们需要至少一个支持的image format和一个presentation mode。
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	//我们需要的所有Queue families
	struct QueueFamilyIndices
	{
		//optional允许uint32_t在实际分配值之前让其保持no value，并且可以通过has_value()来查询是否有值
		std::optional<uint32_t> graphicsFamily;
		//用于present image to surface的family
		std::optional<uint32_t> presentFamily;

		//快速判断当前PhysicalDevices是否支持所有我们需要的Queue Families
		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	//从PhysicalDevice得到我们需要的Queue family，并且可以用来判断physical device是否支持我们需要的所有queue famliy
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		//获取PhysicalDevice支持的所有Queue family
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		//我们需要支持Graphics Queue Family
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			//判断physical device是否支持graphics family
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
			//判断physical device是否支持present family
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				//注意present family和graphics family实际上很有可能指向同一个family，但我们依然将它们视为单独的两个
				indices.presentFamily = i;
			}

			//第一次全获取完就break吧
			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	//判断Physical device是否支持我们需要开启的device extensions
	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		//获取physical device支持的所有device extensions
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		//检查是否所有需要的device extensions都支持
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
#pragma endregion

#pragma region Logical device and queues
	void createLogicalDevice()
	{
		//先填充VkDeviceQueueCreateInfo，描述了我们想要的单个Queue family中的Queue数量。
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		// 目前，我们需要Graphics Queue和Present Queue，因此使用Vector存储queueCreateInfos。
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
			indices.presentFamily.value() };
		float queuePriority = 1.0f;
		//使用一个循环创建所有QueueCreateInfo
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			//Vulkan允许我们为Queues分配一个优先级来影响它们的Commandbuffer的执行调度，其值范围为0.0到1.0
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

	
		//填充需要的设备特性
		VkPhysicalDeviceFeatures deviceFeatures{};

		//填充VkDeviceCreateInfo
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		

		createInfo.pEnabledFeatures = &deviceFeatures;

		//填充针对于device的enabled extensions和（被废弃的）ValidationLayers
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
		
		//创建VkDevice
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		//Queue会在Logical device创建时自动创建，我们需要拿到它的句柄（它也会伴随VkDevice销毁而自动销毁）
		//目前两个Queue family相同，因此返回的两个handle也是相同值的。
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
#pragma endregion

#pragma region Presentation
	//创建window surface，它需要在创建完VkInstance和DebugMessenger后立即创建，因为它会影响Physical device的选择
	void createSurface()
	{
		//直接通过glfw提供的跨平台接口创建surface
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	//检查swap chain的三种属性
	struct SwapChainSupportDetails
	{
		//Basic surface capabilities（swap chain中images的最小最大数量，images的最小最大长宽）
		VkSurfaceCapabilitiesKHR capabilities;
		//Surface formats（像素格式，颜色空间）
		std::vector<VkSurfaceFormatKHR> formats;
		//Available presentation modes
		std::vector<VkPresentModeKHR> presentModes;
	};

	//检查physical device的swap chain支持的属性
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		//Basic surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		//Surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		//Available presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	//选择Swap chain最佳的surface format
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		//优先使用VK_FORMAT_B8R8G8A8_SRGB
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		//fallback使用第一个支持的格式
		return availableFormats[0];
	}

	//选择Swap chain最佳的Presentation mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		//优先使用VK_PRESENT_MODE_MAILBOX_KHR三重缓冲
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	//选择Swap chain最佳的Swap extent
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		//capabilities的currentExtent成员是Vulkan返回的宽高用于匹配window的分辨率
		//但如果其值为uint32_t的最大值，意味着其允许我们主动设置成将swap chain设置成其他分辨率
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			//意味着不允许我们主动改swap chain分辨率，一定要适配window
			return capabilities.currentExtent;
		}
		else
		{
			//Vulkan的分辨率以pixel为单位，而不是屏幕坐标
			int width, height;
			//获取window以pixel为单位的分辨率
			glfwGetFramebufferSize(window, &width, &height);
			
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			//在可用范围内尽可能贴近window分辨率
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	//创建Swap chain
	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//确定swap chain中的Images数量，至少多一张以确保并行度
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		//确保数量不要超过最大数量（maxImageCount为0意味着没上限。
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		//创建Swap chain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // 确定了每一个image组成的layer数量
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // 确定swap chain的使用目的

		//明确swap chain images在多queue families情况下的使用
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			//如果graphics和present为两个queue family，则使用CONCURRENT模式
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			//否则使用EXCLUSIVE模式，swap chain同一时间只属于一个queue family
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		//确定images被应用的变换（通常为旋转90°、水平翻转等），我们不需要任何变换
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		//确定和window system中其他window的透明度混合模式
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		//确定present mode
		createInfo.presentMode = presentMode;
		//确定被其他window遮挡的像素模式
		createInfo.clipped = VK_TRUE;
		//确定老的swap chain
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		//创建完swap chain后存储Images的handle
		//注意我们之前设置的imageCount只是确定swapchain中image的最小数量，实际可能会更多，因此需要重新取值
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		//存储format和extent
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	//为swap chain中每个VkImage创建ImageView
	void createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			//对应的Image
			createInfo.image = swapChainImages[i];
			//viewType和format字段指定如何解释图像数据
			//viewType包括1D、2D、3D纹理和贴图
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			//components字段允许我们混合颜色通道，可以实现不同通道映射，在这里我们使用默认映射
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//subsresourceRange描述Image的用途和可以访问image的哪些部分，我们在这里将其作为Color target
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			//创建ImageView
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}
#pragma endregion

#pragma region Graphics pipeline basics

	//创建pipeline
	void createGraphicsPipeline()
	{
		//加载shader资源
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		//封装到Shader modules
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		//将shader分配到特定管线阶段
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // 要分配的管线阶段
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main"; //entrypoint

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // 要分配的管线阶段
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main"; //entrypoint

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//顶点输入
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		//图元装配模式
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//视口
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//定义裁剪矩形
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		//确定动态状态
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//在创建管线时只需要确定viewportState的数量，在绘制时再确定实际使用的viewport和scissor
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		//配置光栅化器
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // 如果depthClampEnable为true，超出近、远平面的片元会被Clamp而不是丢弃它们。这在绘制深度图等一些特殊情况有用，使用它需要一个GPU feature。
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // 该值为true时，会在光栅化阶段丢弃所有片元。
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // 决定根据几何图形如何生成片元，可以是FILL、LINE、POINT，后两者需要GPU feature
		rasterizer.lineWidth = 1.0f; //超过1.0时，需要启用wideLines GPU feature
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //面剔除类型
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //确定正面方向
		rasterizer.depthBiasEnable = VK_FALSE; // 可以设置深度偏移
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		//配置多重采样
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//配置颜色混合，需要2个结构体
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		//创建pipeline layout，指定shader uniform值
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optinal

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		//创建pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//shader stages
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		//fixed-function
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		//pipeline layout
		pipelineInfo.layout = pipelineLayout;
		//render pass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;//subpass index
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		//创建完pipeline后立刻销毁shader modules
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	//读取文件，并以vector返回byte array
	static std::vector<char> readFile(const std::string& filename)
	{
		//ate表明从文件末尾开始读取
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	//将shader code包装到VkShaderModule中
	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		//注意需要传入uint32_t类型的指针
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	//创建Render pass
	void createRenderPass()
	{
		//创建attachment描述符
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat; //color attachment的format需要和swap chain images的format匹配
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		//loadOp和storeOp应用于color和depth
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//stencilLoadOp和stencilStoreOp应用于stencil
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//render pass前image期望的格式，结束后image将要转变的格式
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//subpass使用的attachment引用
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Vulkan未来会支持compute subpasses，所以我们需要显式告诉它是一个graphics subpass。
		//指定subpass的attachment引用
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		//配置subpass dependency
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 被依赖的subpass
		dependency.dstSubpass = 0; //依赖的subpass
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//创建render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

#pragma endregion

#pragma region Drawing
	//为swap chain的每个image创建framebuffers
	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass; // 要兼容的render pass
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments; // 对应的imageviews
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // 单个pool的指令只能提交到一个queue

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void createCommandBuffers()
	{
		//从command pool中分配command buffer
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHTS);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		//录制绘制指令
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//录制开始，先启动一个render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//render pass自身和要绑定的attachments
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		//渲染区域的尺寸
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		//在使用VK_ATTACHMENT_LOAD_OP_CLEAR时使用的Clear Values
		VkClearValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//绑定graphics pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		//配置dynamic states
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		//绘制指令
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		//结束render pass
		vkCmdEndRenderPass(commandBuffer);
		//结束command buffer录制
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void drawFrame()
	{
		//等待上一帧完成
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		
		//从swap chain中获取一张image
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		//swap chain过时的话重建
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			//swap chain已经不再和surface兼容，并且不能再用于渲染。通常发生在window resize。
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		//在确保我们会执行submit之后再重置fence到unsignaled
		vkResetFences(device, 1, &inFlightFences[currentFrame]);
		
		//重置command buffer，确保其可以被录制
		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
		//录制指令
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
		//提交command buffer到queue中，并且配置同步
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//配置同步
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		//配置要提交的command buffer
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
		//配置command buffer执行完毕后要signal的semaphores
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		//提交Command buffer
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//Presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//等待command buffer执行完毕
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		//呈现image的swap chains和每个swap chain的image index
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		//指定一个VkResult数组，来检查每个swap chain的presentation是否成功
		presentInfo.pResults = nullptr; // Optional
		//向swap chain提交一个present an image的请求
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
		{
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		//递进到下一帧
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHTS;
	}

	void createSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHTS);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHTS);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHTS);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//创建时置为signaled，避免第一帧block
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
				||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
				||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphores!");
			}
		}
	}
#pragma endregion

#pragma region Swap chain recreation
	//重建swap chain前清理旧资源
	void cleanupSwapChain()
	{
		//销毁framebuffers，需要在renderpass、imageviews销毁前、一帧绘制完后销毁
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		//销毁swap chain对应的image views，image view是我们手动创建的，因此也要手动销毁
		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
		//销毁Swap chain
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	//当window surface和swap chain不再兼容时重建swap chain
	void recreateSwapChain()
	{
		//当窗口最小化时暂停运行
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createFramebuffers();
	}

	static void framebufferResizeCallback(GLFWwindow* window, int wdth, int height)
	{
		//设置flag为true，发生resize
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}
#pragma endregion

#pragma region Vertex buffers
	//顶点着色器输入
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			//顶点绑定Vertex binding描述了在整个顶点中从内存加载数据的速率。它指定了data entries之间的字节数以及是否在每个顶点后或每个实例之后移动到下一个data entry
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = {
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};
#pragma endregion
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