//OpenGL Library
#include <glad\/glad.h>
#include <glfw3.h>

//Math library
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//ͼ���
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"

void framebuffer_size_callback(GLFWwindow * window, int width, int height) {	glViewport(0,0,width,height);	}
void ProcessInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void CoordinateSystem_Task1(GLFWwindow * window);
void CoordinateSystem_Task2(GLFWwindow * window);
void CoordinateSystem_Task3(GLFWwindow * window);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//CORE PROFILE

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shader1-8_Task", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}
	//Context and FrameBuffer
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	CoordinateSystem_Task3(window);

	//�������
	glfwTerminate();
	return 0;
}



void ProcessInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

float cubeVertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

/*
	Try experimenting with the FoV and aspect-ratio parameters of GLM's projection function
	�����޸�FOV�ͱ��ʣ��۲�ᷢ��ʲô��
		�޸�FOV�� FOV��ֵԽС��ͼƬ����Խ���ĸо���FOVԽ��ͼƬ������ԽԶ��
		�޸�Ratio��
				
				���Կ������ÿ�߱����ڱ�֤��һ����ǰ���£��Կ�Ƚ����޸ĵģ�����
			��ֵ��ʾ����������еĿ�߱ȡ�
				����߱ȵ��ڴ��ڿ�߱�ʱ������ͼ�εĿ�߱���������ͬ��
				��ֵԽ�ӽ��㣨��ʾ��ԶԶ���ڿ����ܽ��յĺ�����Ϣ��ǳ�С�������ֱ�ǿ�������˴��ڵĿ�������һ�ֺ�������ĸо�
				����ֵԽ���ʱ�򣬷����ܽ��յ��ĸ߶���Ϣ��ʼ�պ�ԭʼ����һ�µģ�����Ϊ��Զ���ڸߣ�����ԭ������Ϣ����ѹ�����˻�������롣
			��ˣ�
*/
void CoordinateSystem_Task1(GLFWwindow * window) {
	//shader
	Shader shader("shader1-8_Tasks.vertex","shader1-8_Tasks.fragment");

	//Texture
	int width, height, nrChannels;
	const char * photoPath1 = "container.jpg";
	const char * photoPath2 = "awesomeface.png";
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned int textures[2];
	glGenTextures(2, textures);

	//ͼƬ container.jpg
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char * data = stbi_load(photoPath1, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//ͼƬ awesomeface.png
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	 data = stbi_load(photoPath2, &width , &height , &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//
	float vertices[] = {
		// positions            // texture coords
		-0.5f,  0.5f, 0.0f,     0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,     1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,     0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,     1.0f, 0.0f   // bottom right	
	};
	
	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO ;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices , GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0));					
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 + 3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	shader.setInt("ourTexture1", 0); // or with shader class	��1��0
	shader.setInt("ourTexture2", 1); // or with shader class	��2��1
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	while (!glfwWindowShouldClose(window)) {
		//Process Input
		ProcessInput(window);


		//Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear( GL_COLOR_BUFFER_BIT );
		shader.use();

		glm::mat4 model; //��local �� model
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f) , glm::vec3(-1.0f, 0.0f, 0.0f));	//��������ϵ

		glm::mat4 view;	// �� model �� eye   ��ת����������ӽǣ�����������Ե�����������ǰ�ƶ���
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	//��������ϵ
		

		//�ǶȺͱ��ʣ��Ի����Ӱ�죡������������
		glm::mat4 projection;//͸���ӽ�
		//float angle = (90.0f * sin((float)glfwGetTime()))/2+ 90.0f;
		//std::cout << angle << std::endl;
		//float angle = (90.0f * sin((float)glfwGetTime())) / 2 + 90.0f;
		//std::cout << angle << std::endl;
		//float ratio = sin((float)glfwGetTime())* ((float)SCR_WIDTH / (float)SCR_HEIGHT);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//std::cout << ratio << std::endl;
		shader.setMartix("model", model);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		
		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES,0,6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT , 0);//���һ����������ʼλ��

		//Swap and polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return ;
}

/*Play with the view matrix by translating in several directions and see how the scene changes. Think of the view matrix as a camera object.
	�޸�view matrix ���Ų�ͬ�ķ���
	�ᷢ�֣�
		����������һ������ƶ����������Ϊ�ƶ�������������������Ե��ƶ��������
*/
void CoordinateSystem_Task2(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//����Ȳ���
	//shader
	Shader shader("shader1-8_Tasks.vertex", "shader1-8_Tasks.fragment");

	//Texture
	int width, height, nrChannels;
	const char * photoPath1 = "container.jpg";
	const char * photoPath2 = "awesomeface.png";
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned int textures[2];
	glGenTextures(2, textures);

	//ͼƬ container.jpg
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char * data = stbi_load(photoPath1, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//ͼƬ awesomeface.png
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load(photoPath2, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//
	float vertices[] = {
		// positions            // texture coords
		-0.5f,  0.5f, 0.0f,     0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,     1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,     0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,     1.0f, 0.0f   // bottom right	
	};

	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 + 3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	shader.setInt("ourTexture1", 0); // or with shader class	��1��0
	shader.setInt("ourTexture2", 1); // or with shader class	��2��1
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		ProcessInput(window);


		//Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		

		glm::mat4 view;	// �� model �� eye   ��ת����������ӽǣ�����������Ե�����������ǰ�ƶ���
		view = glm::translate(view, glm::vec3(1.0f*sin(glfwGetTime()), 1.0f*cos(glfwGetTime()), -3.0f));	//��������ϵ

		glm::mat4 projection;//͸���ӽ�
		//float angle = (90.0f * sin((float)glfwGetTime())) / 2 + 90.0f;
		//std::cout << angle << std::endl;
		//float ratio = sin((float)glfwGetTime())* ((float)SCR_WIDTH / (float)SCR_HEIGHT);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//std::cout << ratio << std::endl;
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);

		for (int i = 0; i < 10; ++i) {
			glm::mat4 model; //��local �� model
			model = glm::translate(model , cubePositions[i]);

			float angle = 20.0f * i;
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));	//��������ϵ
			shader.setMartix("model", model);
			glBindVertexArray(VAO);
			//glDrawArrays(GL_TRIANGLES,0,6);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//���һ����������ʼλ��
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

															//Swap and polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return;
}


void CoordinateSystem_Task3(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//����Ȳ���
							//shader
	Shader shader("shader1-8_Tasks.vertex", "shader1-8_Tasks.fragment");

	//Texture
	int width, height, nrChannels;
	const char * photoPath1 = "container.jpg";
	const char * photoPath2 = "awesomeface.png";
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned int textures[2];
	glGenTextures(2, textures);

	//ͼƬ container.jpg
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char * data = stbi_load(photoPath1, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//ͼƬ awesomeface.png
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load(photoPath2, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//
	float vertices[] = {
		// positions            // texture coords
		-0.5f,  0.5f, 0.0f,     0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,     1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,     0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,     1.0f, 0.0f   // bottom right	
	};

	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 + 3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	shader.setInt("ourTexture1", 0); // or with shader class	��1��0
	shader.setInt("ourTexture2", 1); // or with shader class	��2��1
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		ProcessInput(window);


		//Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();


		glm::mat4 view;	// �� model �� eye   ��ת����������ӽǣ�����������Ե�����������ǰ�ƶ���
		view = glm::translate(view, glm::vec3(0.0f/**sin(glfwGetTime())*/, 0.0f/**cos(glfwGetTime())*/, -3.0f));	//��������ϵ

		glm::mat4 projection;//͸���ӽ�
							 //float angle = (90.0f * sin((float)glfwGetTime())) / 2 + 90.0f;
							 //std::cout << angle << std::endl;
							 //float ratio = sin((float)glfwGetTime())* ((float)SCR_WIDTH / (float)SCR_HEIGHT);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//std::cout << ratio << std::endl;
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);

		for (int i = 0; i < 10; ++i) {
			glm::mat4 model; //��local �� model
			model = glm::translate(model, cubePositions[i]);

			float angle = 0.0f;
			if (i % 4 == 0) angle = 20.0f * i + 20.0f;
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));	//��������ϵ
			shader.setMartix("model", model);
			glBindVertexArray(VAO);
			//glDrawArrays(GL_TRIANGLES,0,6);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//���һ����������ʼλ��
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Swap and polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return;
}