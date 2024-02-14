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
//��ѧ�⣬����vector��matrix�ȣ����������������ݡ�3D MVP�任��
#define GLM_FORCE_RADIANS
//ʹGLM�����������ڴ����
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//ʹ��Vulkan����ȷ�Χ0.0��1.0
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <array>

//ͼ����ؿ�
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
	//�洢descriptor set layout
	VkDescriptorSetLayout descriptorSetLayout;
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
	//Vertex buffer
	VkBuffer vertexBuffer;
	//ʵ�ʷ����vertex buffer���ڴ棨device local��������Ϊ���Դ棩
	VkDeviceMemory vertexBufferMemory;
	//Index buffer
	VkBuffer indexBuffer;
	//ʵ�ʷ����index buffer���ڴ�
	VkDeviceMemory indexBufferMemory;
	//uniform buffers��������ͬʱ���е�frame����һ��
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	//�洢descriptor pool�����ڷ���descriptor sets
	VkDescriptorPool descriptorPool;
	//ÿ������֡��descriptor set
	std::vector<VkDescriptorSet> descriptorSets;
	//�洢����image�����Ӧdevice memory
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	//Texture iamge��Ӧ��image view
	VkImageView textureImageView;
	//Texture image��Ӧ��sampler
	VkSampler textureSampler;
	//Depth buffer���
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

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

	//initVulkan����������ʵ����Vulkan objects˽�г�Ա
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
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createDepthResources();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
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

		//����sampler
		vkDestroySampler(device, textureSampler, nullptr);

		//����texture image view
		vkDestroyImageView(device, textureImageView, nullptr);

		//����texture image�Ͷ�Ӧmemory
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		//����Uniform buffer�Ͷ�ӦMemory
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		//����descriptor pool
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		//����descriptor set layout
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		//����index buffer
		vkDestroyBuffer(device, indexBuffer, nullptr);
		//������index buffer�󶨵��ڴ�
		vkFreeMemory(device, indexBufferMemory, nullptr);

		//����vertex buffer
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		//������vertex buffer�󶨵��ڴ�
		vkFreeMemory(device, vertexBufferMemory, nullptr);

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

		//����Ƿ�֧�ָ������Թ���
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;//�����������Թ���

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
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //ȷ�����淽��
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
		pipelineLayoutInfo.setLayoutCount = 1; // Optional
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
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

		//��vertex buffer
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		//��index buffer
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		//Ϊÿһ֡����ȷ��descriptor setʵ�ʰ󶨵���ɫ���е�descriptor�����ƣ�ʵ�ʾ���Ϊÿһ֡����ȷ��descriptor set
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

		//����ָ�ʹ��index buffer
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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

		//����uniform����
		updateUniformBuffer(currentFrame);
		
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
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			//�����Vertex binding�����������������д��ڴ�������ݵ����ʡ���ָ����data entries֮����ֽ����Լ��Ƿ���ÿ��������ÿ��ʵ��֮���ƶ�����һ��data entry
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
			//��������Attribute description�ṹ����������δ�Դ�԰�����Binding descrription�Ķ������ݿ�����ȡ��������
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		//�����������
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0,
		//�����������
		4, 5, 6, 6, 7, 4
	};

	void createVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		//����staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		//���������ݿ�����staging buffer��
		//���Ƚ��ڴ�ӳ�䵽CPU���Է��ʵ��ڴ���
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		//���ڴ��еĶ������ݿ�����staging buffer���ڴ���
		memcpy(data, vertices.data(), (size_t)bufferSize);
		//���ӳ��
		vkUnmapMemory(device, stagingBufferMemory);
		
		//����ʵ�ʵ�vertex buffer��ע����device local��
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		//���������ݴ�staging buffer������device local��vertex buffer
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		//����staging buffer����VkDeviceMemory
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	//ΪҪ�����buffer�ҵ���ȷ�ĺ��ʵ��Դ�����
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		//��ѯ���п��õ��ڴ�����
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		//�ҵ�buffer�ʺϵ��ڴ�����
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	//����buffer
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;//sizeָ����buffer���ֽ�Ϊ��λ�Ĵ�С
		bufferInfo.usage = usage;//usage������buffer�е����ݵ���;������ʹ�ð�λ��������ָ�������;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//bufferҲ���Ա�һ��ָ����queue familyռ�ã�����ͬʱ�����queue family����

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}

		//buffer�Ѿ��������ˣ���ʵ���ϲ�δ������κ��ڴ�
		//ʹ��ǡ��������vkGetBufferMemoryRequirements������ѯ���ڴ�����
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		//ʵ�ʷ����Դ�
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		//ʹ��vkBindBufferMemory�����ڴ���buffer��������
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	//��һ��buffer�����ݿ�������һ��buffer
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		//����һ����ʱ��command buffer�����ڴ洫�����
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		//����
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;// Optional
		copyRegion.dstOffset = 0;// Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	//����index buffer
	void createIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		//����staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		//���������ݿ�����staging buffer��
		//���Ƚ��ڴ�ӳ�䵽CPU���Է��ʵ��ڴ���
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		//���ڴ��еĶ������ݿ�����staging buffer���ڴ���
		memcpy(data, indices.data(), (size_t)bufferSize);
		//���ӳ��
		vkUnmapMemory(device, stagingBufferMemory);

		//����ʵ�ʵ�index buffer��ע����device local��
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		//���������ݴ�staging buffer������device local��index buffer
		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		//����staging buffer�Ͷ�ӦVkDeviceMemory
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
#pragma endregion

#pragma region Uniform buffers
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	void createDescriptorSetLayout()
	{
		//��������bindings
		//Uniform buffer��binding
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;

		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;//ָ�����ĸ�shader�׶θ�discriptor�ᱻ����
		uboLayoutBinding.pImmutableSamplers = nullptr;// Optional

		//combined image sampler��binding
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;//ubo��bindingΪ0��combined image sampler��bindingΪ1
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;// Optional
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;//ƬԪ��ɫ����ʹ��

		//����binding����
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		//����descriptor set layout
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHTS);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHTS);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHTS);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
		{
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}

	void updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		//��uniform buffer object�ж���MVP�任
		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		//�����ݿ�����uniform buffer��
		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		//uniform buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//��Ҫ������Щdescriptor����
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHTS);//����
		//combined image sampler
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHTS);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHTS);//���ܱ������descriptor sets���������

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHTS, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHTS);
		allocInfo.pSetLayouts = layouts.data();
		//Ϊ�����е�ÿһ֡����һ��descriptor set
		descriptorSets.resize(MAX_FRAMES_IN_FLIGHTS);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
		{
			//Ϊÿ��descriptor set�������е�descriptor��ÿ��descriptor����һ��uniform buffer
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			//��descriptor set�а�ʵ�ʵ�image��sampler��Դ��descriptors
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			//����descriptor set
			//uniform buffer
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;   
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional
			//combined image sampler
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
#pragma endregion

#pragma region Texture mapping
	void createTextureImage()
	{
		//����ͼ���ļ�
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize  imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		//����staging buffer����Ӧ��host visible memory
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		//�����ص�ͼ���ļ��������ݿ�����staging buffer
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		//����ԭʼ����������
		stbi_image_free(pixels);
		//����textureImage�����ڴ�
		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage, textureImageMemory);

		//��texture imageת����VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		//ִ��buffer��image��copy����
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		//��copy֮�����һ��transition��׼����shader����
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
		//����staging buffer�Ͷ�Ӧmemory
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		//����image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//GPU�޷�ʹ�ã����ǵ�һ��transition������texels
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		//��ȡimage��Ӧ���ڴ�Ҫ���ҵ���Ӧ���ڴ����ͣ�
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);
		//�����ڴ�
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}
		//��image��Ϊ������ʵ���ڴ�
		vkBindImageMemory(device, image, imageMemory, 0);
	}

	//�����Single time command buffer��¼�������
	VkCommandBuffer beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}


	void endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	//����layoutת��
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;//������ǲ�����image�����е����ݣ�����oldLayout��ʹ��VK_IMAGE_LAYOUT_UNDEFINEDʱ����
		barrier.newLayout = newLayout;

		//�����ת��queue family����Ȩ����ֵһ��Ҫ���ó�VK_QUEUE_FAMILY_IGNORED������Ĭ��ֵ������
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		//ʹ����ȷ��subresource aspect
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format))
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;//write����Ҫ�ȴ��κ�����
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;//transfer write���뷢����pipeline transfer stage

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//����Ŀ��ܵ�pipeline stage
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;//image����ͬһpipeline stageд�룬�����ƬԪ��ɫ����ȡ

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}



		vkCmdPipelineBarrier(commandBuffer, 
			sourceStage, destinationStage, 
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		//ָ��Ҫ������buffer��Χ�Ϳ�������image��Χ
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		//����buffer��image
		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(commandBuffer);
	}

	void createTextureImageView()
	{
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	//����image view�ĳ���
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//ָ������Ӧ��Ӧ�õ�����filters��transformations
		//��ζԷŴ����С��texels���в�ֵ
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		//������������ʱ��Ѱַģʽ
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//�������Թ���
		samplerInfo.anisotropyEnable = VK_TRUE;
		//��physical device����ȡ��properties
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		//��properties�л�ȡ�������Թ������ֵ
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		//��ʹ��clamp to borderѰַģʽ�²�������imageʱ�Ĺ̶���ɫֵ
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//����imageѰַtexel��ʱ��ʹ����������ϵͳ
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		//�Ƿ����ñȽϺ�����������PCF����ʵ������Ӱ
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		//mipmapping����ֶ�
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		//����sampler
		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
#pragma endregion

#pragma region Depth buffering
	void createDepthResources()
	{
		//�ҵ��ʺϵ�depth format
		VkFormat depthFormat = findDepthFormat();

		createImage(swapChainExtent.width, swapChainExtent.height, depthFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		//��ʽת��image layout
		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			//��ѯformat�Ƿ�֧��
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		//���û��֧�ֵ�format�����׳��쳣
		throw std::runtime_error("failed to find supported format");
	}

	VkFormat findDepthFormat()
	{
		return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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