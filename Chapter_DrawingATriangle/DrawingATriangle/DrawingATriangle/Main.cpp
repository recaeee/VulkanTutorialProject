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

//ʹ�ó���������Ӳ�����width��height����Ϊ���ǻ���������Щֵ
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
	//���õ��Կ�
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	//ʼ��GLFW���Ҵ���һ��window
	void initWindow()
	{
		//��ʼ��GLFW��
		glfwInit();

		//����GLFW�ʼ�Ǳ����ڴ���OpenGL context�ģ�����������Ҫ��������Ҫ����һ��OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//disable resize����
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//����������window
		//ǰ��������ȷ����window�Ŀ��ߺͱ��⡣���ĸ�������������ѡ��ָ����window�ļ����������һ����������OpenGL��ء�
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	//**initVulkan**����������ʵ����Vulkan objects˽�г�Ա
	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
	}

	//mainloop����ʼ��Ⱦÿһ֡
	void mainLoop()
	{
		//Ϊ��ȷ��Ӧ��ֱ������������߹ر�window�Ž������У�������Ҫ����һ���¼�ѭ����mainLoop�У�����
		while (!glfwWindowShouldClose(window))
		{
			//�������¼��������¼��ص�����������ꡢ�����¼������ڳߴ�ĵ��������ڹرյ�
			glfwPollEvents();
		}
	}

	//һ��window���رգ����ǽ���**cleanup**������ȷ���ͷ������õ���������Դ
	void cleanup()
	{
		//����DebugUtilsMessenger
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMeseengerEXT(instance, debugMessenger, nullptr);
		}

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

		return indices.isComplete();
	}

	//������Ҫ������Queue families
	struct QueueFamilyIndices
	{
		//optional����uint32_t��ʵ�ʷ���ֵ֮ǰ���䱣��no value�����ҿ���ͨ��has_value()����ѯ�Ƿ���ֵ
		std::optional<uint32_t> graphicsFamily;

		//�����жϵ�ǰPhysicalDevices�Ƿ�֧������������Ҫ��Queue Families
		bool isComplete()
		{
			return graphicsFamily.has_value();
		}
	};

	//�ж�PhysicalDevice�Ƿ�֧��������Ҫ��Queue family
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
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				break;
			}

			i++;
		}

		return indices;
	}
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