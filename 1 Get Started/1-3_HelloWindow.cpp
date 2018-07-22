#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
//ȷ����GLFW֮ǰ����GLAD��

//�ص����������û��޸Ĵ��ڴ�Сʱ��viewportҲӦ������Ӧ�޸�
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//input
void processInput(GLFWwindow *window);

int main(void) {
	
	//1.//��ʼ����,�ɹ�����GL_TRUE
	//*********************************************************************************************
	if (!glfwInit()) {
		std::cout << "��ʼ��ʧ��";
	}	

	//2.���ò���
	//*********************************************************************************************
	//set the major and minor version both to 3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//use the core-profile
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint�ĵ�һ������˵����Ҫ���õ�option��
	//�ڶ�����������option ��ֵ
	//option��һϵ����GLFWΪǰ׺�Ĳ���

	//3.�������ڼ���Ӧ����
	//*********************************************************************************************
	//����������Ϊ������
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL){
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//4.initialize GLAD��OpenGL������ָ������������
	//*********************************************************************************************
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		//glfwGetProcAddress ���ڵ�ǰ�Ĳ���ϵͳ������ȷ�ĺ�����
		//pass GLAD the function to load the address of the OpenGL function pointers.
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	
	//5.˵��OpenGL��Ⱦ���ڵĴ�С������OpenGL����֪������ϣ��������ô��ڵĴ�С��λ�á�
	//*********************************************************************************************
	glViewport(0, 0, 800, 600);//�������Ͻ����꣬�Լ����
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//6.�����
	//*********************************************************************************************
	while (!glfwWindowShouldClose(window))	//��GLFW��Ҫ��رյ�ʱ�򣬷���true
	{
		//input
		processInput(window);

		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		//check and call events and swap the buffers
		//NOTE:: color buffer (a large buffer that contains color values for each pixel in GLFW's window) 
		glfwSwapBuffers(window);	//����ָ�����ڵ�˫���棨ǰ�󻺴棩����ɫ����
		glfwPollEvents();//����Ƿ������¼����������������´���״̬�����ö�Ӧ�Ļص�����
	}

	//���һ���£��ͷ���Դ
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
//��һ����������GLFWwindow
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}