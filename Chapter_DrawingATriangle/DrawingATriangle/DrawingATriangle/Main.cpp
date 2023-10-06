//GLFW将会包含其自己的一些定义，并且自动加载Vulkan头文件
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//stdexcept和iostream头文件用于报告和输出errors
#include <iostream>
#include <stdexcept>
//cstdlib头文件提供了EXIT_SUCCESS和EXIT_FAILURE宏
#include <cstdlib>

//使用常量而不是硬编码的width和height，因为我们会多次引用这些值
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
		//销毁window
		glfwDestroyWindow(window);

		//结束GLFW本身
		glfwTerminate();
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