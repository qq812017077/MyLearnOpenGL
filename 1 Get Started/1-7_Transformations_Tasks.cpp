//OpenGL初始化库
#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//图像库
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"

/*
stb_image.h 是一个很好用的头文件，可以用来加载大部分的文件格式。
并且很容易集成到项目中。

定义STB_IMAGE_IMPLEMENTATION
使得预处理器将处理该头文件以至于其仅包含相关定义的源代码
*/



void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);
void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


void Transformation_Task1(GLFWwindow * window);
void Transformation_Task2(GLFWwindow * window);

int main(void)
{

	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
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

	Transformation_Task2(window);


	glfwTerminate();
	//system("pause");
	return 0;
}

const char * Photo_Path = "container.jpg";
/*Task1
	 try switching the order around by first rotating and then translating. See what happens and try to reason why this happens
	 即：先旋转后平移。会发生什么？
		
		修改之后：
			代码上是先旋转后平移，实际上则应理解为先平移后旋转。
			旋转应该是围绕着中心点进行的。故展现出来的是：围绕着中心进行
	
	下面是官方的解释：
	 Why does our container now spin around our screen?:
	== ===================================================
	Remember that matrix multiplication is applied in reverse. This time a translation is thus
	applied first to the container positioning it in the bottom-right corner of the screen.
	After the translation the rotation is applied to the translated container.

	A rotation transformation is also known as a change-of-basis transformation
	for when we dig a bit deeper into linear algebra. Since we're changing the
	basis of the container, the next resulting translations will translate the container
	based on the new basis vectors. Once the vector is slightly rotated, the vertical
	translations would also be slightly translated for example.

	If we would first apply rotations then they'd resolve around the rotation origin (0,0,0), but
	since the container is first translated, its rotation origin is no longer (0,0,0) making it
	looks as if its circling around the origin of the scene.

	If you had trouble visualizing this or figuring it out, don't worry. If you
	experiment with transformations you'll soon get the grasp of it; all it takes
	is practice and experience.

*/
void Transformation_Task1(GLFWwindow * window) {
	Shader shader("shader1-7.vertex", "shader1-7.fragment");

	//贴图
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//加载图片
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	int width, height, nrChannels;
	unsigned char * data = stbi_load(Photo_Path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);//为目标生成多级纹理
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	//生成了纹理和相应的多级渐远纹理后，释放图像的内存并解绑纹理对象是一个很好的习惯。
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);//解绑

	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f   // bottom right	
	};

	unsigned int indices[] =
	{
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		//process input 
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);//使用的时候注意绑定
		shader.use();
		glm::mat4 trans;

		trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
		
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

/*
	Try to use transformations only to draw a second container with another call to glDrawElements .
	this second container is placed at the top-left of the window and instead of rotating, scale it over time(by using sin)
	尝试使用glDrawElements绘制第二个矩形。放在左上角且进行缩放
*/
void Transformation_Task2(GLFWwindow * window) {
	Shader shader("shader1-7.vertex", "shader1-7.fragment");

	//贴图
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//加载图片
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	int width, height, nrChannels;
	unsigned char * data = stbi_load(Photo_Path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);//为目标生成多级纹理
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	//生成了纹理和相应的多级渐远纹理后，释放图像的内存并解绑纹理对象是一个很好的习惯。
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);//解绑

	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f   // bottom right	
	};

	unsigned int indices[] =
	{
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		//process input 
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);//使用的时候注意绑定
		shader.use();
		glBindVertexArray(VAO);
		glm::mat4 trans1,trans2;

		//右下旋转
		trans1 = glm::translate(trans1, glm::vec3(0.5f, -0.5f, 0.0f));
		trans1 = glm::rotate(trans1, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans1));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//左上缩放
		trans2 = glm::translate(trans2, glm::vec3(-0.5f, 0.5f, 0.0f));
		trans2 = glm::scale(trans2, sin((float)glfwGetTime())*(glm::vec3(1.0f, 1.0f, 1.0f)));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans2));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
}
//检测按键输入
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}

