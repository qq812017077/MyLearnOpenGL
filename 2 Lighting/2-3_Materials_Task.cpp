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

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0,0,width,height); }
void mouse_callback(GLFWwindow * window, double xPos , double yPos);
void scroll_callback(GLFWwindow * window , double xOffset , double yOffset);
void ProcessInput(GLFWwindow * window ,float deltaTime);
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void Material_Tasks(GLFWwindow * window);

glm::vec3 cameraPos(0.0f,0.0f,3.0f);
glm::vec3 cameraUp(0.0f,1.0f,0.0f);
float yaw = -90.0f, pitch = 0.0f;
Camera camera(cameraPos, cameraUp,yaw,pitch);

bool openSpecular = true;
int lastStatus;

int main(void) {
	if (!glfwInit()) {
		std::cout<< "Initializing Failed"<< std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"2-3_materials_tasks",NULL,NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window,mouse_callback);
	glfwSetScrollCallback(window,scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Material_Tasks(window);
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
void ProcessInput(GLFWwindow * window , float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window,true);
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
	}else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		curMovement = Camera_Movement::LEFT;
	}else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		curMovement = Camera_Movement::BACKWARD;
	}else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		curMovement = Camera_Movement::RIGHT;
	}

	camera.ProcessKeyboard(curMovement, deltaTime,false);
}
/*包括了每个顶点及其法向量
*/
float cubeVertices[] = {
	//每个面6个顶点，
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

void Material_Tasks(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-3.vertex","Shader2-3.fragment");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//VAO,VBO
	unsigned objectVAO, lightVAO , VBO;
	glGenVertexArrays(1,&objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1,&VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setVec3("material.ambient", silver.ambient);
	objectShader.setVec3("material.diffuse", silver.diffuse);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	////光的颜色
	//glm::vec3 ambient(0.5f, 0.5f, 0.5f);
	//glm::vec3 diffuse(0.2f, 0.5f, 0.2f);
	//glm::vec3 specular(1.0f, 1.0f, 1.0f);
	//光的颜色
	glm::vec3 ambient(1.0f, 1.0f, 1.0f);
	glm::vec3 diffuse(1.0f, 1.0f, 1.0f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	objectShader.setVec3("light.position", lightPos);
	objectShader.setVec3("light.ambient", ambient);
	objectShader.setVec3("light.diffuse", diffuse);
	objectShader.setVec3("light.specular", specular);

	float curTime{}, lastTime{};
	while (!glfwWindowShouldClose(window)) {
		
		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window,curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH/(float)SCR_HEIGHT,0.1f,100.0f);
		
		lightShader.use();
		lightShader.setMartix("view",view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model,glm::vec3(0.2f));
		lightShader.setMartix("model",model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model,(float)glm::radians(10.0f*glfwGetTime()),glm::vec3(0.0f,1.0f,0.0f));
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}