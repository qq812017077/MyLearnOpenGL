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
#include "Camera.h"
#include "materialLibrary.h"


/*Diffuse maps
很显然，一个物体的不同部位通常有着不同的反射颜色
So what we want is some way to set the diffuse color of an object for each indifidual fragment.
我们要基于物体的像素位置来获取对应的颜色！！

这即是：
漫反射贴图！
*/


/*Specular maps
一个specular高光的亮度可以通过图片中每个纹理的亮度来获得。
specular贴图的每个像素可以显示为一个颜色向量，比如：在那里黑色代表颜色向量vec3(0.0f)，
灰色是vec3(0.5f)。在片段着色器中，我们采样相应的颜色值，把它乘以光的specular亮度。
像素越“白”，乘积的结果越大，物体的specualr部分越亮。
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);
unsigned int loadTexture(char const * path);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void LightingMaps_Task12(GLFWwindow * window);
void LightingMaps_Task3(GLFWwindow * window);
void LightingMaps_Task4(GLFWwindow * window);
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f;
Camera camera(cameraPos, cameraUp, yaw, pitch);

bool openSpecular = true;
int lastStatus;

int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-3_materials_tasks", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	LightingMaps_Task4(window);
	glfwTerminate();
}


/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow * window, double xPos, double yPos) {
	if (firstMouse) // this bool variable is initially set to true
	{
		//第一次调用时的xPos,yPos和lastX,lastY相差巨大，需要进行一次调整
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//敏感度
	float xOffset = xPos - lastX;	//向左移动为负，向右移动为正。
	float yOffset = lastY - yPos;	//向上移动为正，向下移动为负。

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	std::cout << camera.Zoom << std::endl;
	camera.ProcessMouseScroll(yOffset);
}
void ProcessInput(GLFWwindow * window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastStatus == GLFW_PRESS) {
		//使用空格来开关镜面反射效果
		if (openSpecular) openSpecular = false;
		else openSpecular = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	//按键的处理
	Camera_Movement curMovement = Camera_Movement::NONE;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		curMovement = Camera_Movement::FORWARD;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		curMovement = Camera_Movement::LEFT;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		curMovement = Camera_Movement::BACKWARD;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		curMovement = Camera_Movement::RIGHT;
	}

	camera.ProcessKeyboard(curMovement, deltaTime, false);
}
/*包括了每个顶点及其法向量,以及漫反射贴图
*/
float cubeVertices[] = {
	// positions          // normals           // texture coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

/*	1.尝试调整light的ambient，diffuse 以及 specular
			观察现象

	2.尝试反转镜面向量。
		即： 高变低，低变高
			通过 vec3(1.0f)-vec3(texture(material.specular, TexCoords)) 可以达到要求
*/
void LightingMaps_Task12(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-4.vertex", "Shader2-4.fragment");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//Texture
	unsigned int diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap);	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char * data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//VAO,VBO
	unsigned objectVAO, lightVAO, VBO;
	glGenVertexArrays(1, &objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	//光的颜色
	glm::vec3 ambient(0.4f, 0.4f, 0.4f);
	glm::vec3 diffuse(0.7f, 0.7f, 0.7f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	objectShader.setVec3("light.position", lightPos);
	objectShader.setVec3("light.ambient", ambient);
	objectShader.setVec3("light.diffuse", diffuse);
	objectShader.setVec3("light.specular", specular);

	float curTime{}, lastTime{};

	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMartix("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model, (float)glm::radians(10.0f*glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

/*	
	Try creating a specular map from the diffuse texture
	即使用漫反射贴图原有的颜色进行镜面映射:
		则高光部分会反射出相应的颜色：
			即：若镜面反射图显示蓝色，则高光呈现蓝色

*/
void LightingMaps_Task3(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-4.vertex", "Shader2-4.fragment");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//Texture
	unsigned int diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap);	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char * data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("lighting_maps_specular_color_blue.png", &width, &height, &nrChannels, 0);
	//data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//VAO,VBO
	unsigned objectVAO, lightVAO, VBO;
	glGenVertexArrays(1, &objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	//光的颜色
	glm::vec3 ambient(0.4f, 0.4f, 0.4f);
	glm::vec3 diffuse(0.7f, 0.7f, 0.7f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	objectShader.setVec3("light.position", lightPos);
	objectShader.setVec3("light.ambient", ambient);
	objectShader.setVec3("light.diffuse", diffuse);
	objectShader.setVec3("light.specular", specular);

	float curTime{}, lastTime{};

	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMartix("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model, (float)glm::radians(10.0f*glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

/*
	emission map
		放射光贴图：保存了每一个像素的发光值。
	发光值即如果自身包含光源的时候，发布部分的发光颜色
		用于 机器人的眼睛，或者物体上的小灯。

		在我们的箱子上添加一个发光贴图

	结果：
		在 ambient、diffuse 和 specular 的基础上在添加 emission。
		
		！！！
			有点怀疑，为什么可以在基础上加？物体本身颜色值对齐不会干扰吗？
			也许，是实际运用中物体贴图对应的部分本身即是黑色，即0.0，所以才被允许加起来？
*/
void LightingMaps_Task4(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-4.vertex", "Shader2-4.fragment_Task4");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//Texture
	unsigned int diffuseMap, specularMap , emissionMap;
	glGenTextures(1, &diffuseMap);	glGenTextures(1, &specularMap); glGenTextures(1, &emissionMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char * data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	//data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, emissionMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("matrix.jpg", &width, &height, &nrChannels, 0);
	//data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//VAO,VBO
	unsigned objectVAO, lightVAO, VBO;
	glGenVertexArrays(1, &objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	objectShader.setInt("material.emission", 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, emissionMap);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	//光的颜色
	glm::vec3 ambient(0.4f, 0.4f, 0.4f);
	glm::vec3 diffuse(0.7f, 0.7f, 0.7f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	objectShader.setVec3("light.position", lightPos);
	objectShader.setVec3("light.ambient", ambient);
	objectShader.setVec3("light.diffuse", diffuse);
	objectShader.setVec3("light.specular", specular);

	float curTime{}, lastTime{};

	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMartix("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model, (float)glm::radians(10.0f*glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}