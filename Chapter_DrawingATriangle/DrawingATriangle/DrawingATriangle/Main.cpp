//GLFW����������Լ���һЩ���壬�����Զ�����Vulkanͷ�ļ�
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//stdexcept��iostreamͷ�ļ����ڱ�������errors
#include <iostream>
#include <stdexcept>
//cstdlibͷ�ļ��ṩ��EXIT_SUCCESS��EXIT_FAILURE��
#include <cstdlib>

#include <vector>

//ʹ�ó���������Ӳ�����width��height����Ϊ���ǻ���������Щֵ
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
		//VkInstanceֻӦ���ڳ����˳�ǰһ�����٣�����������Vulkan��Դ��Ӧ����instance����ǰ�ͷ�
		vkDestroyInstance(instance, nullptr);

		//����window
		glfwDestroyWindow(window);

		//����GLFW����
		glfwTerminate();
	}
	
	void createInstance()
	{
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
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		//���Extension֧��
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "available extensions\n";

		for (const auto& extension : extensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}

		checkRequiredExtensionsSupported(glfwExtensions, glfwExtensionCount, extensions);

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}

	void checkRequiredExtensionsSupported(const char** requiredExtensions, uint32_t requiredExtensionCount, std::vector<VkExtensionProperties> supportedExtensions)
	{
		for (int i = 0; i < requiredExtensionCount; i++)
		{
			bool found = false;
			for (const auto& extension : supportedExtensions)
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