#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
#include "Shaders.h"

void framebuffer_size_callback(GLFWwindow * window , int  width , int height);
void ProcessInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


void Task_1(GLFWwindow * window);
void Task_2(GLFWwindow * window);
void Task_3(GLFWwindow * window);

int main(void) {
	//
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1-5Shader_Tasks", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//GLAD库
	//glfwGetProcAddress:该函数基于当前操作系统
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//Task_1(window);
	Task_3(window);
	glfwTerminate();
}
void framebuffer_size_callback(GLFWwindow * window, int  width, int height) {
	glViewport(0, 0, width, height);
}
void ProcessInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

/*
Answer to the question: Do you know why the bottom-left side is black?(Task3)
-- --------------------------------------------------------------------
Think about this for a second: the output of our fragment's color is equal to the (interpolated) coordinate of
the triangle. What is the coordinate of the bottom-left point of our triangle? This is (-0.5f, -0.5f, 0.0f). Since the
xy values are negative they are clamped to a value of 0.0f. This happens all the way to the center sides of the
triangle since from that point on the values will be interpolated positively again. Values of 0.0f are of course black
and that explains the black side of the triangle.
*/
void Task_3(GLFWwindow * window) {
	Shader shader("shader1-5_Task3.vertex", "shader1-5_Task3.fragment");

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Task_2(GLFWwindow * window) {
	Shader shader("shader1-5_Task2.vertex","shader.fragment");

	float vertices[] = {
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.0f,  -0.5f, 0.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer( GL_ARRAY_BUFFER, VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		float time = glfwGetTime();
		float offset = (sin(time) / 2.0f);
		shader.setFloat1f("myOffset", offset);
		shader.setFloat4f("OurColor", 1.0f, 0.0f, 0.0f, 0.0f);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}


void Task_1(GLFWwindow * window) {
	Shader shader("shader.vertex", "shader.fragment");

	float vertices[] = {
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.0f,  -0.5f, 0.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		shader.setFloat4f("OurColor", 1.0f, 0.0f, 0.0f, 0.0f);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}
