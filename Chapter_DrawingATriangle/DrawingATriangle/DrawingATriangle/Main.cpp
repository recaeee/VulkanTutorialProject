//GLFW����������Լ���һЩ���壬�����Զ�����Vulkanͷ�ļ�
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//stdexcept��iostreamͷ�ļ����ڱ�������errors
#include <iostream>
#include <stdexcept>
//cstdlibͷ�ļ��ṩ��EXIT_SUCCESS��EXIT_FAILURE��
#include <cstdlib>

#include <vector>
#include <cstring>

#include <optional>
#include <set>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include <fstream>
//��ѧ�⣬����vector��matrix�ȣ�����������������
#include <glm/glm.hpp>
#include <array>

//ʹ�ó���������Ӳ�����width��height����Ϊ���ǻ���������Щֵ
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
//����֡Ӧ��ͬʱ����
const int MAX_FRAMES_IN_FLIGHTS = 2;

//Ҫʹ�õ�validation layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//debugģʽ�¿���Validation layers��release������
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

//���ڴ���DebugUtilMessenger�ĺ����Ǹ���չ�����������Զ����ص��ڴ棩����������Ҫ�Լ��������ĵ�ַ����ʹ��proxy function��ִ�С�
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
	//����debug callback��handle
	VkDebugUtilsMessengerEXT debugMessenger;
	//Vulkan��ʵ��ƽ̨Window system�Ľ����������Surface
	VkSurfaceKHR surface;
	//���õ��Կ�
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//Logical device��������physical device����
	VkDevice device;
	//�洢Graphics queue��Handle
	VkQueue graphicsQueue;
	//�洢Present queue��Handle
	VkQueue presentQueue;
	//�洢Swap chain
	VkSwapchainKHR swapChain;
	//�洢Swap chain�е�VkImage��
	std::vector<VkImage> swapChainImages;
	//�洢swap chain��format��extent��δ�����õ�
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	//�洢swap chain images��image views
	std::vector<VkImageView> swapChainImageViews;
	//�洢render pass
	VkRenderPass renderPass;
	//�洢pipeline layout������ָ����������ʱ��uniformֵ
	VkPipelineLayout pipelineLayout;
	//�洢pipeline
	VkPipeline graphicsPipeline;
	//�洢framebuffers
	std::vector<VkFramebuffer> swapChainFramebuffers;
	//�洢command pool
	VkCommandPool commandPool;
	//�洢command buffer
	std::vector<VkCommandBuffer> commandBuffers;
	//�洢��swapchain��ȡ��image��semaphore
	std::vector<VkSemaphore> imageAvailableSemaphores;
	//�洢image��Ⱦ��ϵ�semaphore
	std::vector<VkSemaphore> renderFinishedSemaphores;
	//�洢��֤һ��ֻ��Ⱦһ֡��fence
	std::vector<VkFence> inFlightFences;
	//��ǰ֡����
	uint32_t currentFrame = 0;
	//��ʶһ��window resize������flag
	bool framebufferResized = false;

	//����Ҫ���õ�device extensions
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//ʼ��GLFW���Ҵ���һ��window
	void initWindow()
	{
		//��ʼ��GLFW��
		glfwInit();

		//����GLFW�ʼ�Ǳ����ڴ���OpenGL context�ģ�����������Ҫ��������Ҫ����һ��OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//disable resize����
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//����������window
		//ǰ��������ȷ����window�Ŀ��ߺͱ��⡣���ĸ�������������ѡ��ָ����window�ļ����������һ����������OpenGL��ء�
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		//��helloTriangleApplication��thisָ��洢��window��
		glfwSetWindowUserPointer(window, this);
		//ע��resize�ص�
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	//**initVulkan**����������ʵ����Vulkan objects˽�г�Ա
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

	//mainloop����ʼ��Ⱦÿһ֡
	void mainLoop()
	{
		//Ϊ��ȷ��Ӧ��ֱ������������߹ر�window�Ž������У�������Ҫ����һ���¼�ѭ����mainLoop�У�����
		while (!glfwWindowShouldClose(window))
		{
			//�������¼��������¼��ص�����������ꡢ�����¼������ڳߴ�ĵ��������ڹرյ�
			glfwPollEvents();
			//����һ֡
			drawFrame();
		}

		//�ȴ�logical device������в��������˳�mainLoop
		vkDeviceWaitIdle(device);
	}

	//һ��window���رգ����ǽ���**cleanup**������ȷ���ͷ������õ���������Դ
	void cleanup()
	{
		cleanupSwapChain();

		//����pipeline
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		//����pipeline layout
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		//����render pass
		vkDestroyRenderPass(device, renderPass, nullptr);

		//�������������������commands������ˣ�����û�и����ͬ������Ҫʱ��semaphores��fence��Ҫ������
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
		{
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		//����command pool
		vkDestroyCommandPool(device, commandPool, nullptr);
		
		//����VkDevice
		vkDestroyDevice(device, nullptr);
		//����DebugUtilsMessenger
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMeseengerEXT(instance, debugMessenger, nullptr);
		}
		//����Window surface
		vkDestroySurfaceKHR(instance, surface, nullptr);
		//VkInstanceֻӦ���ڳ����˳�ǰһ�����٣�����������Vulkan��Դ��Ӧ����instance����ǰ�ͷ�
		vkDestroyInstance(instance, nullptr);
		//����window
		glfwDestroyWindow(window);
		//����GLFW����
		glfwTerminate();
	}
	
	void createInstance()
	{
		//���Validation layers֧��
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
		//Instance�����ǵ�Ӧ����Vulkan libaray֮�����ϵ����������Ҫ������ָ�������ǵ�Ӧ����ص�һЩ��ϸ��Ϣ
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
		//������Ҫʹ����Щȫ��extensions��validation layers
		auto extensions = getRequiredExtension();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		//���ⴴ��һ��DebugUtilsMessenger����vkInstance��pNext��չָ��������������debug vkCreateInstance��vkDestroyInstance����Ϣ��
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

		//���Extension֧��
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
		std::cout << "available extensions\n";

		for (const auto& extension : availableExtensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}
		//����Ҫ���Validation layers��extension�Ƿ�֧��
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

	//���ʹ�õ�validation layers�Ƿ�֧��
	bool checkValidationLayerSupport()
	{
		//��extension����һ���Ĳ���
		//�ȵõ�����֧�ֵ�layers
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//���
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

	//�õ���Ҫʹ�õ�extensions
	std::vector<const char*> getRequiredExtension()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		//ѡ���Կ���Validation layers
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	//�Լ�����Validation��Ϣ�ص�
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

	//��ʼ��Debug��Ϣ��messenger(VkDebugUtilsMessengerEXT�������messenger��vkInstance����������ʹ�õ�
	void setupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		//����messengerͬ����ҪCreateInfo������
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		polulateDebugMessengerCreateInfo(createInfo);

		//ͨ��proxy function����DebugUtilsMessenger
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//DebugUtilsMessenger��CreateInfo��䵥�������һ������
	void polulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//�ص�
		createInfo.pfnUserCallback = debugCallback;
	}

#pragma region Physical devices and queue families
	//����һ�ź��ʵ��Կ�
	void pickPhysicalDevice()
	{
		//�г��������õ��Կ�
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		//���û��һ��֧��Vulkan���Կ����ã����׳��쳣
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		//�ҵ����ʵ��Կ�
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

	//�жϸ��Կ��Ƿ�֧��������Ҫ�Ĺ���
	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		//��ȡ�Կ��Ļ������ԣ��������֡����͡�֧�ֵ�vulkan�汾
		//VkPhysicalDeviceProperties deviceProperties;
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//��ȡ�Կ�֧�ֵ����ԣ���������ѹ����64���ظ����������ӿ���Ⱦ
		//VkPhysicalDeviceFeatures deviceFeatures;
		//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		//��֮����½����ǻ����ѯ�Դ桢Queue families

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			//�ڸý̳��У�������Ҫ����һ��֧�ֵ�image format��һ��presentation mode��
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	//������Ҫ������Queue families
	struct QueueFamilyIndices
	{
		//optional����uint32_t��ʵ�ʷ���ֵ֮ǰ���䱣��no value�����ҿ���ͨ��has_value()����ѯ�Ƿ���ֵ
		std::optional<uint32_t> graphicsFamily;
		//����present image to surface��family
		std::optional<uint32_t> presentFamily;

		//�����жϵ�ǰPhysicalDevices�Ƿ�֧������������Ҫ��Queue Families
		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	//��PhysicalDevice�õ�������Ҫ��Queue family�����ҿ��������ж�physical device�Ƿ�֧��������Ҫ������queue famliy
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		//��ȡPhysicalDevice֧�ֵ�����Queue family
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		//������Ҫ֧��Graphics Queue Family
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			//�ж�physical device�Ƿ�֧��graphics family
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
			//�ж�physical device�Ƿ�֧��present family
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				//ע��present family��graphics familyʵ���Ϻ��п���ָ��ͬһ��family����������Ȼ��������Ϊ����������
				indices.presentFamily = i;
			}

			//��һ��ȫ��ȡ���break��
			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	//�ж�Physical device�Ƿ�֧��������Ҫ������device extensions
	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		//��ȡphysical device֧�ֵ�����device extensions
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		//����Ƿ�������Ҫ��device extensions��֧��
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
		//�����VkDeviceQueueCreateInfo��������������Ҫ�ĵ���Queue family�е�Queue������
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		// Ŀǰ��������ҪGraphics Queue��Present Queue�����ʹ��Vector�洢queueCreateInfos��
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
			indices.presentFamily.value() };
		float queuePriority = 1.0f;
		//ʹ��һ��ѭ����������QueueCreateInfo
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			//Vulkan��������ΪQueues����һ�����ȼ���Ӱ�����ǵ�Commandbuffer��ִ�е��ȣ���ֵ��ΧΪ0.0��1.0
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

	
		//�����Ҫ���豸����
		VkPhysicalDeviceFeatures deviceFeatures{};

		//���VkDeviceCreateInfo
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		

		createInfo.pEnabledFeatures = &deviceFeatures;

		//��������device��enabled extensions�ͣ��������ģ�ValidationLayers
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
		
		//����VkDevice
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		//Queue����Logical device����ʱ�Զ�������������Ҫ�õ����ľ������Ҳ�����VkDevice���ٶ��Զ����٣�
		//Ŀǰ����Queue family��ͬ����˷��ص�����handleҲ����ֵͬ�ġ�
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
#pragma endregion

#pragma region Presentation
	//����window surface������Ҫ�ڴ�����VkInstance��DebugMessenger��������������Ϊ����Ӱ��Physical device��ѡ��
	void createSurface()
	{
		//ֱ��ͨ��glfw�ṩ�Ŀ�ƽ̨�ӿڴ���surface
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	//���swap chain����������
	struct SwapChainSupportDetails
	{
		//Basic surface capabilities��swap chain��images����С���������images����С��󳤿�
		VkSurfaceCapabilitiesKHR capabilities;
		//Surface formats�����ظ�ʽ����ɫ�ռ䣩
		std::vector<VkSurfaceFormatKHR> formats;
		//Available presentation modes
		std::vector<VkPresentModeKHR> presentModes;
	};

	//���physical device��swap chain֧�ֵ�����
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

	//ѡ��Swap chain��ѵ�surface format
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		//����ʹ��VK_FORMAT_B8R8G8A8_SRGB
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		//fallbackʹ�õ�һ��֧�ֵĸ�ʽ
		return availableFormats[0];
	}

	//ѡ��Swap chain��ѵ�Presentation mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		//����ʹ��VK_PRESENT_MODE_MAILBOX_KHR���ػ���
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	//ѡ��Swap chain��ѵ�Swap extent
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		//capabilities��currentExtent��Ա��Vulkan���صĿ������ƥ��window�ķֱ���
		//�������ֵΪuint32_t�����ֵ����ζ�������������������óɽ�swap chain���ó������ֱ���
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			//��ζ�Ų���������������swap chain�ֱ��ʣ�һ��Ҫ����window
			return capabilities.currentExtent;
		}
		else
		{
			//Vulkan�ķֱ�����pixelΪ��λ����������Ļ����
			int width, height;
			//��ȡwindow��pixelΪ��λ�ķֱ���
			glfwGetFramebufferSize(window, &width, &height);
			
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			//�ڿ��÷�Χ�ھ���������window�ֱ���
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	//����Swap chain
	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//ȷ��swap chain�е�Images���������ٶ�һ����ȷ�����ж�
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		//ȷ��������Ҫ�������������maxImageCountΪ0��ζ��û���ޡ�
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		//����Swap chain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // ȷ����ÿһ��image��ɵ�layer����
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // ȷ��swap chain��ʹ��Ŀ��

		//��ȷswap chain images�ڶ�queue families����µ�ʹ��
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			//���graphics��presentΪ����queue family����ʹ��CONCURRENTģʽ
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			//����ʹ��EXCLUSIVEģʽ��swap chainͬһʱ��ֻ����һ��queue family
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		//ȷ��images��Ӧ�õı任��ͨ��Ϊ��ת90�㡢ˮƽ��ת�ȣ������ǲ���Ҫ�κα任
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		//ȷ����window system������window��͸���Ȼ��ģʽ
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		//ȷ��present mode
		createInfo.presentMode = presentMode;
		//ȷ��������window�ڵ�������ģʽ
		createInfo.clipped = VK_TRUE;
		//ȷ���ϵ�swap chain
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		//������swap chain��洢Images��handle
		//ע������֮ǰ���õ�imageCountֻ��ȷ��swapchain��image����С������ʵ�ʿ��ܻ���࣬�����Ҫ����ȡֵ
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		//�洢format��extent
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	//Ϊswap chain��ÿ��VkImage����ImageView
	void createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			//��Ӧ��Image
			createInfo.image = swapChainImages[i];
			//viewType��format�ֶ�ָ����ν���ͼ������
			//viewType����1D��2D��3D�������ͼ
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			//components�ֶ��������ǻ����ɫͨ��������ʵ�ֲ�ͬͨ��ӳ�䣬����������ʹ��Ĭ��ӳ��
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//subsresourceRange����Image����;�Ϳ��Է���image����Щ���֣����������ｫ����ΪColor target
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			//����ImageView
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}
#pragma endregion

#pragma region Graphics pipeline basics

	//����pipeline
	void createGraphicsPipeline()
	{
		//����shader��Դ
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		//��װ��Shader modules
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		//��shader���䵽�ض����߽׶�
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // Ҫ����Ĺ��߽׶�
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main"; //entrypoint

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // Ҫ����Ĺ��߽׶�
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main"; //entrypoint

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//��������
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		//ͼԪװ��ģʽ
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//�ӿ�
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//����ü�����
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		//ȷ����̬״̬
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//�ڴ�������ʱֻ��Ҫȷ��viewportState���������ڻ���ʱ��ȷ��ʵ��ʹ�õ�viewport��scissor
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		//���ù�դ����
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // ���depthClampEnableΪtrue����������Զƽ���ƬԪ�ᱻClamp�����Ƕ������ǡ����ڻ������ͼ��һЩ����������ã�ʹ������Ҫһ��GPU feature��
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // ��ֵΪtrueʱ�����ڹ�դ���׶ζ�������ƬԪ��
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // �������ݼ���ͼ���������ƬԪ��������FILL��LINE��POINT����������ҪGPU feature
		rasterizer.lineWidth = 1.0f; //����1.0ʱ����Ҫ����wideLines GPU feature
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //���޳�����
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //ȷ�����淽��
		rasterizer.depthBiasEnable = VK_FALSE; // �����������ƫ��
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		//���ö��ز���
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//������ɫ��ϣ���Ҫ2���ṹ��
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

		//����pipeline layout��ָ��shader uniformֵ
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

		//����pipeline
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

		//������pipeline����������shader modules
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	//��ȡ�ļ�������vector����byte array
	static std::vector<char> readFile(const std::string& filename)
	{
		//ate�������ļ�ĩβ��ʼ��ȡ
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

	//��shader code��װ��VkShaderModule��
	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		//ע����Ҫ����uint32_t���͵�ָ��
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	//����Render pass
	void createRenderPass()
	{
		//����attachment������
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat; //color attachment��format��Ҫ��swap chain images��formatƥ��
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		//loadOp��storeOpӦ����color��depth
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//stencilLoadOp��stencilStoreOpӦ����stencil
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//render passǰimage�����ĸ�ʽ��������image��Ҫת��ĸ�ʽ
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//subpassʹ�õ�attachment����
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Vulkanδ����֧��compute subpasses������������Ҫ��ʽ��������һ��graphics subpass��
		//ָ��subpass��attachment����
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		//����subpass dependency
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // ��������subpass
		dependency.dstSubpass = 0; //������subpass
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//����render pass
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
	//Ϊswap chain��ÿ��image����framebuffers
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
			framebufferInfo.renderPass = renderPass; // Ҫ���ݵ�render pass
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments; // ��Ӧ��imageviews
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
	
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // ����pool��ָ��ֻ���ύ��һ��queue

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void createCommandBuffers()
	{
		//��command pool�з���command buffer
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
		//¼�ƻ���ָ��
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//¼�ƿ�ʼ��������һ��render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//render pass�����Ҫ�󶨵�attachments
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		//��Ⱦ����ĳߴ�
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		//��ʹ��VK_ATTACHMENT_LOAD_OP_CLEARʱʹ�õ�Clear Values
		VkClearValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//��graphics pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		//����dynamic states
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
		//����ָ��
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		//����render pass
		vkCmdEndRenderPass(commandBuffer);
		//����command buffer¼��
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void drawFrame()
	{
		//�ȴ���һ֡���
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		
		//��swap chain�л�ȡһ��image
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		//swap chain��ʱ�Ļ��ؽ�
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			//swap chain�Ѿ����ٺ�surface���ݣ����Ҳ�����������Ⱦ��ͨ��������window resize��
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		//��ȷ�����ǻ�ִ��submit֮��������fence��unsignaled
		vkResetFences(device, 1, &inFlightFences[currentFrame]);
		
		//����command buffer��ȷ������Ա�¼��
		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
		//¼��ָ��
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
		//�ύcommand buffer��queue�У���������ͬ��
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//����ͬ��
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		//����Ҫ�ύ��command buffer
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
		//����command bufferִ����Ϻ�Ҫsignal��semaphores
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		//�ύCommand buffer
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//Presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//�ȴ�command bufferִ�����
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		//����image��swap chains��ÿ��swap chain��image index
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		//ָ��һ��VkResult���飬�����ÿ��swap chain��presentation�Ƿ�ɹ�
		presentInfo.pResults = nullptr; // Optional
		//��swap chain�ύһ��present an image������
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

		//�ݽ�����һ֡
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
		//����ʱ��Ϊsignaled�������һ֡block
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
	//�ؽ�swap chainǰ�������Դ
	void cleanupSwapChain()
	{
		//����framebuffers����Ҫ��renderpass��imageviews����ǰ��һ֡�����������
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		//����swap chain��Ӧ��image views��image view�������ֶ������ģ����ҲҪ�ֶ�����
		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
		//����Swap chain
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	//��window surface��swap chain���ټ���ʱ�ؽ�swap chain
	void recreateSwapChain()
	{
		//��������С��ʱ��ͣ����
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
		//����flagΪtrue������resize
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}
#pragma endregion

#pragma region Vertex buffers
	//������ɫ������
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			//�����Vertex binding�����������������д��ڴ�������ݵ����ʡ���ָ����data entries֮����ֽ����Լ��Ƿ���ÿ��������ÿ��ʵ��֮���ƶ�����һ��data entry
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