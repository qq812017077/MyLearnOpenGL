#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
//确保在GLFW之前包含GLAD，

//回调函数，当用户修改窗口大小时，viewport也应进行相应修改
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//input
void processInput(GLFWwindow *window);

int main(void) {
	
	//1.//初始化库,成功返回GL_TRUE
	//*********************************************************************************************
	if (!glfwInit()) {
		std::cout << "初始化失败";
	}	

	//2.配置参数
	//*********************************************************************************************
	//set the major and minor version both to 3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//use the core-profile
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint的第一个参数说明想要配置的option。
	//第二个参数设置option 的值
	//option是一系列以GLFW为前缀的参数

	//3.创建窗口及对应环境
	//*********************************************************************************************
	//第三个参数为窗口名
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL){
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//4.initialize GLAD：OpenGL函数的指针是由其管理的
	//*********************************************************************************************
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		//glfwGetProcAddress 基于当前的操作系统定义正确的函数。
		//pass GLAD the function to load the address of the OpenGL function pointers.
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	
	//5.说明OpenGL渲染窗口的大小，这样OpenGL才能知道我们希望如何设置窗口的大小和位置。
	//*********************************************************************************************
	glViewport(0, 0, 800, 600);//包括左上角坐标，以及宽高
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//6.搭建引擎
	//*********************************************************************************************
	while (!glfwWindowShouldClose(window))	//当GLFW被要求关闭的时候，返回true
	{
		//input
		processInput(window);

		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		//check and call events and swap the buffers
		//NOTE:: color buffer (a large buffer that contains color values for each pixel in GLFW's window) 
		glfwSwapBuffers(window);	//交换指定窗口的双缓存（前后缓存），颜色缓存
		glfwPollEvents();//检测是否所有事件都被触发，并更新窗口状态，调用对应的回调函数
	}

	//最后一件事，释放资源
	glfwTerminate();
	//system("pause");
	return 0;
}

//
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//第一个参数即：GLFWwindow
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}