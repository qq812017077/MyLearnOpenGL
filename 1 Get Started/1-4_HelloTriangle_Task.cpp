#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
#define BUFZISE(x)	(sizeof(x)/sizeof(x[0]))

//回调和事件管理
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

//任务
void Task_1(GLFWwindow *window);
void Task_2(GLFWwindow *window);
void Task_3(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const  char * vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

const char *fragmentShaderSource_Yellow = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
"}\n\0";


int main(void) {
	//初始化库
	if (!glfwInit()) { std::cout << "初始化失败" << std::endl; }
	//配置参数
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//创建窗口
	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL_Task", NULL, NULL);
	if (NULL == window) {
		std::cout << "窗口创建失败" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//注意同一时间开启一个任务
	//Task_1(window);
	//Task_2(window);
	Task_3(window);

	//最后一件事，释放资源
	glfwTerminate();
}

//帧缓冲大小缓存
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0,0,width,height);
}
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window,true);//关闭
	}
}

//Try to draw 2 triangles next to each other using glDrawArrays by adding more vertices to your data
/*	即：不使用EBO，仅通过VAO 和 VBO 绘制两个三角形，这样的后果就是 ：	
		每个图形的点都需要单独说明，即使重合
*/
void Task_1(GLFWwindow * window) {

	//shader着色器程序构建
	int successCompile;
	char InfoLog[512];
	//顶点
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILE_FAILED\n" << InfoLog << std::endl;
	}
	//片元
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n" << InfoLog << std::endl;
	}
	//链接
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
	}
	//删除
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//定点数据
	float vertices[] = {
		-0.5f,  0.5f, 0.0f,   // top left 
		0.5f,  0.5f, 0.0f,  // top right
		-0.5f, -0.5f, 0.0f,  // bottom left

		0.5f,  0.5f, 0.0f,  // top right
		-0.5f, -0.5f, 0.0f,  // bottom left
		0.5f, -0.5f, 0.0f  // bottom right
	};

	/*
	VBO		vertex buffer object
	VAO		vertex array object
	*/
	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);//在使用VBO之后，解除绑定
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//loop
	while (!glfwWindowShouldClose(window)) {

		//input事件处理
		processInput(window);


		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//绘制
		glUseProgram(shaderProgram);

		glBindVertexArray(VAO);			//
		glDrawArrays(GL_TRIANGLES, 0, 6);//第二个参数指定顶点数组索引，第三个指定顶点数
										 //glDrawArrays(GL_TRIANGLES, 3, 3);//第二个参数指定顶点数组索引，第三个指定顶点数
										 //
		glfwSwapBuffers(window);//交换缓冲区数据
		glfwPollEvents();//检测是否所有事件都被触发，并更新窗口状态，调用对应的回调函数
	}
	return ;
}


//Now create the same 2 triangles using two different VAOs and VBOs for their data:
/*	
	即：创建两个相同的三角形，但是使用不同的VAO和VBO
*/
void Task_2(GLFWwindow * window) {
	int successCompile;
	char InfoLog[512];

	//Shader程序
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);//第二个参数指定了数组中的元素个数
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, sizeof(InfoLog)/ sizeof(InfoLog[0]), NULL, InfoLog);	//第三个参数保存InfoLog字符串长度
		std::cout << "ERROR::SHADER::VERTEX::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);//第二个参数指定了数组中的元素个数
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);	//第三个参数保存InfoLog字符串长度
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);	//第三个参数保存InfoLog字符串长度
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
	}
	//删除
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float vertices[] = {
		-0.5f,  0.5f, 0.0f,   // top left 
		0.5f,  0.5f, 0.0f,  // top right
		-0.5f, -0.5f, 0.0f,  // bottom left

		0.5f,  0.5f, 0.0f,  // top right
		-0.5f, -0.5f, 0.0f,  // bottom left
		0.5f, -0.5f, 0.0f  // bottom right
	};

	unsigned int VAOs[2], VBOs[2] ;
	glGenVertexArrays(2 , VAOs);
	glGenBuffers(2, VBOs);

	//配置第一组VAO 和 VBO
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)/2, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//以顶点属性位置值作为参数，启用顶点属性

	glBindBuffer(GL_ARRAY_BUFFER,0);	//解绑
	glBindVertexArray(0);

	//配置第二组VAO 和 VBO
	glBindVertexArray(VAOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)/2, vertices + (sizeof(vertices) / sizeof(vertices[0]))/ 2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//以顶点属性位置值作为参数，启用顶点属性

	glBindBuffer(GL_ARRAY_BUFFER, 0);	//解绑
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {
		//input process
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(VAOs[1]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return;
}

/*	Create two shader programs where the second program uses a different fragment shader that outputs the color yellow; 
	draw both triangles again where one outputs the color yellow
*/
void Task_3(GLFWwindow * window) {
	//shader
	int successCompile;
	char InfoLog[512];

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, BUFZISE(InfoLog), NULL, InfoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, BUFZISE(InfoLog), NULL, InfoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int fragmentShader_Yellow = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader_Yellow, 1, &fragmentShaderSource_Yellow, NULL);
	glCompileShader(fragmentShader_Yellow);
	glGetShaderiv(fragmentShader_Yellow, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader_Yellow, BUFZISE(InfoLog), NULL, InfoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);	
	glAttachShader(shaderProgram, fragmentShader);	
	glLinkProgram(shaderProgram);					
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, BUFZISE(InfoLog), NULL, InfoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
	}

	int shaderProgram_Yellow = glCreateProgram();
	glAttachShader(shaderProgram_Yellow, vertexShader);
	glAttachShader(shaderProgram_Yellow, fragmentShader_Yellow);
	glLinkProgram(shaderProgram_Yellow);
	glGetShaderiv(shaderProgram_Yellow, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram_Yellow, BUFZISE(InfoLog), NULL, InfoLog);
		std::cout << "ERROR::SHADER::PROGRAM_YELLOW::LINKING_FAILED\n" << InfoLog << std::endl;
	}

	//定点数据
	float vertices[] = {
		-0.5f,  0.5f, 0.0f,   // top left 
		0.5f,  0.5f, 0.0f,  // top right
		-0.5f, -0.5f, 0.0f,  // bottom left

		0.5f,  0.5f, 0.0f,  // top right
		-0.5f, -0.5f, 0.0f,  // bottom left
		0.5f, -0.5f, 0.0f  // bottom right
	};

	/*
	VBO		vertex buffer object
	VAO		vertex array object
	*/
	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);//在使用VBO之后，解除绑定
	glBindVertexArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {
		//input process
		processInput(window);


		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清空所用的颜色
		glClear(GL_COLOR_BUFFER_BIT);//使用glClearColor设置的颜色进行清空
		glBindVertexArray(VAO);

		glUseProgram(shaderProgram);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glUseProgram(shaderProgram_Yellow);
		glDrawArrays(GL_TRIANGLES, 3, 6);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return;
}