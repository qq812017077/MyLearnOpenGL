//OpenGL Library
#include <glad\/glad.h>
#include <glfw3.h>

//Math library
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//图像库
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

	//初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	CoordinateSystem_Task3(window);

	//清除数据
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
	试着修改FOV和比率，观察会发生什么？
		修改FOV： FOV的值越小，图片离人越近的感觉。FOV越大，图片则离人越远。
		修改Ratio：
				
				可以看出，该宽高比是在保证高一定的前提下，对宽度进行修改的！！！
			该值表示的是摄像机中的宽高比。
				当宽高比等于窗口宽高比时，绘制图形的宽高比与数据相同。
				该值越接近零（表示高远远大于宽），能接收的横向信息会非常小，而其又被强行拉成了窗口的宽，所以有一种横向拉伸的感觉
				当数值越大的时候，发现能接收到的高度信息是始终和原始数据一致的，而因为宽远大于高，所以原来的信息，被压缩到了画面的中央。
			因此，
*/
void CoordinateSystem_Task1(GLFWwindow * window) {
	//shader
	Shader shader("shader1-8_Tasks.vertex","shader1-8_Tasks.fragment");

	//Texture
	int width, height, nrChannels;
	const char * photoPath1 = "container.jpg";
	const char * photoPath2 = "awesomeface.png";
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	unsigned int textures[2];
	glGenTextures(2, textures);

	//图片 container.jpg
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

	//图片 awesomeface.png
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
	shader.setInt("ourTexture1", 0); // or with shader class	把1给0
	shader.setInt("ourTexture2", 1); // or with shader class	把2给1
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

		glm::mat4 model; //从local 到 model
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f) , glm::vec3(-1.0f, 0.0f, 0.0f));	//右手坐标系

		glm::mat4 view;	// 从 model 到 eye   即转换到摄像机视角（摄像机向后，相对的整个世界向前移动）
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	//右手坐标系
		

		//角度和比率，对画面的影响！！！！！！！
		glm::mat4 projection;//透视视角
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
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT , 0);//最后一个参数是起始位置

		//Swap and polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return ;
}

/*Play with the view matrix by translating in several directions and see how the scene changes. Think of the view matrix as a camera object.
	修改view matrix 沿着不同的方向，
	会发现：
		整个场景在一起进行移动。可以理解为移动了整个场景，或者相对的移动了相机。
*/
void CoordinateSystem_Task2(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//打开深度测试
	//shader
	Shader shader("shader1-8_Tasks.vertex", "shader1-8_Tasks.fragment");

	//Texture
	int width, height, nrChannels;
	const char * photoPath1 = "container.jpg";
	const char * photoPath2 = "awesomeface.png";
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	unsigned int textures[2];
	glGenTextures(2, textures);

	//图片 container.jpg
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

	//图片 awesomeface.png
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
	shader.setInt("ourTexture1", 0); // or with shader class	把1给0
	shader.setInt("ourTexture2", 1); // or with shader class	把2给1
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
		

		glm::mat4 view;	// 从 model 到 eye   即转换到摄像机视角（摄像机向后，相对的整个世界向前移动）
		view = glm::translate(view, glm::vec3(1.0f*sin(glfwGetTime()), 1.0f*cos(glfwGetTime()), -3.0f));	//右手坐标系

		glm::mat4 projection;//透视视角
		//float angle = (90.0f * sin((float)glfwGetTime())) / 2 + 90.0f;
		//std::cout << angle << std::endl;
		//float ratio = sin((float)glfwGetTime())* ((float)SCR_WIDTH / (float)SCR_HEIGHT);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//std::cout << ratio << std::endl;
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);

		for (int i = 0; i < 10; ++i) {
			glm::mat4 model; //从local 到 model
			model = glm::translate(model , cubePositions[i]);

			float angle = 20.0f * i;
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));	//右手坐标系
			shader.setMartix("model", model);
			glBindVertexArray(VAO);
			//glDrawArrays(GL_TRIANGLES,0,6);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//最后一个参数是起始位置
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

															//Swap and polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return;
}


void CoordinateSystem_Task3(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//打开深度测试
							//shader
	Shader shader("shader1-8_Tasks.vertex", "shader1-8_Tasks.fragment");

	//Texture
	int width, height, nrChannels;
	const char * photoPath1 = "container.jpg";
	const char * photoPath2 = "awesomeface.png";
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	unsigned int textures[2];
	glGenTextures(2, textures);

	//图片 container.jpg
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

	//图片 awesomeface.png
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
	shader.setInt("ourTexture1", 0); // or with shader class	把1给0
	shader.setInt("ourTexture2", 1); // or with shader class	把2给1
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


		glm::mat4 view;	// 从 model 到 eye   即转换到摄像机视角（摄像机向后，相对的整个世界向前移动）
		view = glm::translate(view, glm::vec3(0.0f/**sin(glfwGetTime())*/, 0.0f/**cos(glfwGetTime())*/, -3.0f));	//右手坐标系

		glm::mat4 projection;//透视视角
							 //float angle = (90.0f * sin((float)glfwGetTime())) / 2 + 90.0f;
							 //std::cout << angle << std::endl;
							 //float ratio = sin((float)glfwGetTime())* ((float)SCR_WIDTH / (float)SCR_HEIGHT);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//std::cout << ratio << std::endl;
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);

		for (int i = 0; i < 10; ++i) {
			glm::mat4 model; //从local 到 model
			model = glm::translate(model, cubePositions[i]);

			float angle = 0.0f;
			if (i % 4 == 0) angle = 20.0f * i + 20.0f;
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));	//右手坐标系
			shader.setMartix("model", model);
			glBindVertexArray(VAO);
			//glDrawArrays(GL_TRIANGLES,0,6);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//最后一个参数是起始位置
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Swap and polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return;
}