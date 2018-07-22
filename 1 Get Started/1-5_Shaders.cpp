#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
#include "Shaders.h"
void framebuffer_size_callback(GLFWwindow * window, int width , int height);
void processInput(GLFWwindow * window);


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void ShaderMoreAttrib(GLFWwindow * window);
void Shader_AutoChangingColor(GLFWwindow * window);

void Shader_UseClass(GLFWwindow * window);

int main(void) 
{
	if (!glfwInit()) {
		std::cout<< "Failed to Initialization"<< std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1-5_Shaders_", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//初始化GLAD库
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	
	//Shader_AutoChangingColor(window);
	//ShaderMoreAttrib(window);
	
	Shader_UseClass(window);
	//Final
	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0,0,width, height);
}

void processInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}


/*	多属性顶点：
	每个点的颜色可控
*/
void ShaderMoreAttrib(GLFWwindow * window) {
	const  char * vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec4 ourColor;\n"
		"out vec4 vertexColor;\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0);\n"
		"	vertexColor = ourColor;\n"
		"}\n\0";
	const char * fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec4 vertexColor;\n"
		"void main(void)\n"
		"{\n"
		"	FragColor = vertexColor;\n"
		"}\n\0";

	//Shader program
	int successCompile{};
	char InfoLog[512];
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, (sizeof(InfoLog) / sizeof(InfoLog[0])), NULL, InfoLog);
		std::cout << "	ERROR::SHADER::VERTEX::COMPILE_FAILED:\n" << InfoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, (sizeof(InfoLog) / sizeof(InfoLog[0])), NULL, InfoLog);
		std::cout << "	ERROR::SHADER::FRAGMENT::COMPILE_FAILED:\n" << InfoLog << std::endl;
	}

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, (sizeof(InfoLog) / sizeof(InfoLog[0])), NULL, InfoLog);
		std::cout << "	ERROR::SHADER::PROGRAM::LINKING_FAILED:\n" << InfoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float vertices[] = {
		//Position				//Color
		-0.5f, -0.5f, 0.0f,		1.0f,0.0f,0.0f,
		0.5f, -0.5f, 0.0f,		0.0f,1.0f,0.0f,
		0.0f,  0.5f, 0.0f,		0.0f,0.0f,1.0f
	};

	//
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {

		//input process
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		glUseProgram(shaderProgram);
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}


/*	随时间变换颜色：
	使用Uniform全局属性
*/
void Shader_AutoChangingColor(GLFWwindow * window) {
	const  char * vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0);\n"
		"}\n\0";
	const char * fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"uniform vec4 OurColor;\n"
		"void main(void)\n"
		"{\n"
		"	FragColor = OurColor;\n"
		"}\n\0";

	//Shader program
	int successCompile{};
	char InfoLog[512];
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, (sizeof(InfoLog) / sizeof(InfoLog[0])), NULL, InfoLog);
		std::cout << "	ERROR::SHADER::VERTEX::COMPILE_FAILED:\n" << InfoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, (sizeof(InfoLog) / sizeof(InfoLog[0])), NULL, InfoLog);
		std::cout << "	ERROR::SHADER::FRAGMENT::COMPILE_FAILED:\n" << InfoLog << std::endl;
	}

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, (sizeof(InfoLog) / sizeof(InfoLog[0])), NULL, InfoLog);
		std::cout << "	ERROR::SHADER::PROGRAM::LINKING_FAILED:\n" << InfoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};

	//
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

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {

		//input process
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		glUseProgram(shaderProgram);
		float timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "OurColor");
		//std::cout << vertexColorLocation << std::endl;
		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}


void Shader_UseClass(GLFWwindow * window) {
	Shader shader("shader.vertex","shader.fragment");

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};

	//
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
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		float timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;
		shader.setFloat4f("OurColor", 0.0f, greenValue, 0.0f, 1.0f);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}