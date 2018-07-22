#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
#define BUFZISE(x)	(sizeof(x)/sizeof(x[0]))

//�ص����¼�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

//����
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
	//��ʼ����
	if (!glfwInit()) { std::cout << "��ʼ��ʧ��" << std::endl; }
	//���ò���
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//��������
	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL_Task", NULL, NULL);
	if (NULL == window) {
		std::cout << "���ڴ���ʧ��" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//ע��ͬһʱ�俪��һ������
	//Task_1(window);
	//Task_2(window);
	Task_3(window);

	//���һ���£��ͷ���Դ
	glfwTerminate();
}

//֡�����С����
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0,0,width,height);
}
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window,true);//�ر�
	}
}

//Try to draw 2 triangles next to each other using glDrawArrays by adding more vertices to your data
/*	������ʹ��EBO����ͨ��VAO �� VBO �������������Σ������ĺ������ ��	
		ÿ��ͼ�εĵ㶼��Ҫ����˵������ʹ�غ�
*/
void Task_1(GLFWwindow * window) {

	//shader��ɫ�����򹹽�
	int successCompile;
	char InfoLog[512];
	//����
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILE_FAILED\n" << InfoLog << std::endl;
	}
	//ƬԪ
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n" << InfoLog << std::endl;
	}
	//����
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
	}
	//ɾ��
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//��������
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);//��ʹ��VBO֮�󣬽����
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//loop
	while (!glfwWindowShouldClose(window)) {

		//input�¼�����
		processInput(window);


		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//����
		glUseProgram(shaderProgram);

		glBindVertexArray(VAO);			//
		glDrawArrays(GL_TRIANGLES, 0, 6);//�ڶ�������ָ����������������������ָ��������
										 //glDrawArrays(GL_TRIANGLES, 3, 3);//�ڶ�������ָ����������������������ָ��������
										 //
		glfwSwapBuffers(window);//��������������
		glfwPollEvents();//����Ƿ������¼����������������´���״̬�����ö�Ӧ�Ļص�����
	}
	return ;
}


//Now create the same 2 triangles using two different VAOs and VBOs for their data:
/*	
	��������������ͬ�������Σ�����ʹ�ò�ͬ��VAO��VBO
*/
void Task_2(GLFWwindow * window) {
	int successCompile;
	char InfoLog[512];

	//Shader����
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);//�ڶ�������ָ���������е�Ԫ�ظ���
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, sizeof(InfoLog)/ sizeof(InfoLog[0]), NULL, InfoLog);	//��������������InfoLog�ַ�������
		std::cout << "ERROR::SHADER::VERTEX::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);//�ڶ�������ָ���������е�Ԫ�ظ���
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);	//��������������InfoLog�ַ�������
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n" << InfoLog << std::endl;
	}

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(shaderProgram, sizeof(InfoLog) / sizeof(InfoLog[0]), NULL, InfoLog);	//��������������InfoLog�ַ�������
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
	}
	//ɾ��
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

	//���õ�һ��VAO �� VBO
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)/2, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//�Զ�������λ��ֵ��Ϊ���������ö�������

	glBindBuffer(GL_ARRAY_BUFFER,0);	//���
	glBindVertexArray(0);

	//���õڶ���VAO �� VBO
	glBindVertexArray(VAOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)/2, vertices + (sizeof(vertices) / sizeof(vertices[0]))/ 2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//�Զ�������λ��ֵ��Ϊ���������ö�������

	glBindBuffer(GL_ARRAY_BUFFER, 0);	//���
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

	//��������
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);//��ʹ��VBO֮�󣬽����
	glBindVertexArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {
		//input process
		processInput(window);


		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//����������õ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);//ʹ��glClearColor���õ���ɫ�������
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